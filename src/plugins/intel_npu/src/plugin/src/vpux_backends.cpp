// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "vpux_backends.hpp"

#include <fstream>
#include <memory>

#include "device_helpers.hpp"
#include "vpux/al/config/common.hpp"

#if defined(ENABLE_ZEROAPI_BACKEND)
#    include <zero_backend.h>
#endif

#if !defined(OPENVINO_STATIC_LIBRARY) && defined(ENABLE_IMD_BACKEND)
#    include <openvino/util/file_util.hpp>
#    include <openvino/util/shared_object.hpp>
#endif

using namespace vpux;

namespace {

std::string backendToString(const AvailableBackends& backend) {
    switch (backend) {
    case AvailableBackends::LEVEL_ZERO:
        return "npu_level_zero_backend";
    case AvailableBackends::IMD:
        return "npu_imd_backend";
    default:
        return "unsupported backend";
    }
}

#if !defined(OPENVINO_STATIC_LIBRARY) && defined(ENABLE_IMD_BACKEND)
std::shared_ptr<void> loadBackendLibrary(const std::string& libpath) {
#    if defined(OPENVINO_ENABLE_UNICODE_PATH_SUPPORT) && defined(_WIN32)
    return ov::util::load_shared_object(ov::util::string_to_wstring(libpath).c_str());
#    else
    return ov::util::load_shared_object(libpath.c_str());
#    endif
}

std::shared_ptr<IEngineBackend> getBackend(std::shared_ptr<void> so, const vpux::Config& config) {
    static constexpr auto CreateFuncName = "CreateVPUXEngineBackend";
    auto symbol = ov::util::get_symbol(so, CreateFuncName);

    using CreateFuncT = void (*)(std::shared_ptr<IEngineBackend>&, const Config&);
    const auto createFunc = reinterpret_cast<CreateFuncT>(symbol);

    std::shared_ptr<IEngineBackend> backendPtr;
    createFunc(backendPtr, config);
    return backendPtr;
}

ov::SoPtr<IEngineBackend> loadBackend(const std::string& libpath, const vpux::Config& config) {
    auto backendSO = loadBackendLibrary(libpath);
    auto backend = getBackend(backendSO, config);

    return ov::SoPtr<IEngineBackend>(backend, backendSO);
}
#endif

}  // namespace

namespace vpux {

// TODO Config will be useless here, since only default values will be used
VPUXBackends::VPUXBackends(const std::vector<AvailableBackends>& backendRegistry, [[maybe_unused]] const Config& config)
    : _logger("NPUBackends", intel_npu::Logger::global().level()) {
    std::vector<ov::SoPtr<IEngineBackend>> registeredBackends;
    [[maybe_unused]] const auto registerBackend = [&](ov::SoPtr<IEngineBackend> backend, const std::string& name) {
        const auto backendDevices = backend->getDeviceNames();
        if (!backendDevices.empty()) {
            std::stringstream deviceNames;
            for (const auto& device : backendDevices) {
                deviceNames << device << " ";
            }
            _logger.debug("Register '%s' with devices '%s'", name.c_str(), deviceNames.str().c_str());
            registeredBackends.emplace_back(backend);
        }
    };

    for (const auto& name : backendRegistry) {
        std::string backendName = backendToString(name);
        _logger.debug("Try '%s' backend", backendName.c_str());

        try {
#if !defined(OPENVINO_STATIC_LIBRARY) && defined(ENABLE_IMD_BACKEND)
            if (name == vpux::AvailableBackends::IMD) {
                const auto path =
                    ov::util::make_plugin_library_name(ov::util::get_ov_lib_path(), backendName + OV_BUILD_POSTFIX);
                const auto exists = std::ifstream(path).good();
                if (!exists) {
                    _logger.debug("Backend '%s' at '%s' doesn't exist", backendName.c_str(), path.c_str());
                    continue;
                }
                const auto backend = loadBackend(path, config);
                registerBackend(backend, backendName);
            }
#endif

#if defined(ENABLE_ZEROAPI_BACKEND)
            if (name == vpux::AvailableBackends::LEVEL_ZERO) {
                const auto backend = ov::SoPtr<IEngineBackend>(std::make_shared<ZeroEngineBackend>(config));
                registerBackend(backend, backendName);
            }
#endif
        } catch (const std::exception& ex) {
            _logger.error("Got an error during backend '%s' loading : %s", backendName.c_str(), ex.what());
        } catch (...) {
            _logger.error("Got an unknown error during backend '%s' loading", backendName.c_str());
        }
    }

    if (registeredBackends.empty()) {
        registeredBackends.emplace_back(nullptr);
    }

    // TODO: implementation of getDevice methods needs to be updated to go over all
    // registered backends to search a device.
    // A single backend is chosen for now to keep existing behavior
    _backend = *registeredBackends.begin();

    if (_backend != nullptr) {
        _logger.info("Use '%s' backend for inference", _backend->getName().c_str());
    } else {
        _logger.error("Cannot find backend for inference. Make sure the device is available.");
    }
}

std::string VPUXBackends::getBackendName() const {
    if (_backend != nullptr) {
        return _backend->getName();
    }

    return "";
}

std::shared_ptr<IDevice> VPUXBackends::getDevice(const std::string& specificName) const {
    _logger.debug("Searching for device %s to use started...", specificName.c_str());
    // TODO iterate over all available backends
    std::shared_ptr<IDevice> deviceToUse;

    if (_backend != nullptr) {
        if (specificName.empty()) {
            deviceToUse = _backend->getDevice();
        } else {
            deviceToUse = _backend->getDevice(specificName);
        }
    }

    if (deviceToUse == nullptr) {
        _logger.warning("Device not found!");
    } else {
        _logger.debug("Device found: %s", deviceToUse->getName().c_str());
    }
    return deviceToUse;
}

std::shared_ptr<IDevice> VPUXBackends::getDevice(const ov::AnyMap& paramMap) const {
    return _backend->getDevice(paramMap);
}

std::vector<std::string> VPUXBackends::getAvailableDevicesNames() const {
    return _backend == nullptr ? std::vector<std::string>() : _backend->getDeviceNames();
}

void VPUXBackends::registerOptions(OptionsDesc& options) const {
    if (_backend != nullptr) {
        _backend->registerOptions(options);
    }
}

// TODO config should be also specified to backends, to allow use logging in devices and all levels below
void VPUXBackends::setup(const Config& config) {
    _logger.setLevel(config.get<LOG_LEVEL>());
}

std::string VPUXBackends::getCompilationPlatform(const std::string_view platform, const std::string& deviceId) const {
    // Platform parameter has a higher priority than deviceID
    if (platform != ov::intel_npu::Platform::AUTO_DETECT) {
        return std::string(platform);
    }

    // Get compilation platform from deviceID
    if (!deviceId.empty()) {
        return utils::getPlatformByDeviceName(deviceId);
    }

    // Automatic detection of compilation platform
    const auto devNames = getAvailableDevicesNames();
    if (devNames.empty()) {
        OPENVINO_THROW("No devices found - platform must be explicitly specified for compilation. Example: -d NPU.3700 "
                       "instead of -d NPU.");
    }

    return utils::getPlatformByDeviceName(devNames.at(0));
}

}  // namespace vpux
