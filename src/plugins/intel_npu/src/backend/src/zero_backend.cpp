//
// Copyright (C) 2024 Intel Corporation.
// SPDX-License-Identifier: Apache 2.0
//

#include "zero_backend.h"

#include <vector>

#include "vpux/al/config/common.hpp"
#include "zero_device.h"

namespace vpux {

ZeroEngineBackend::ZeroEngineBackend(const Config& config) {
    Logger::global().setLevel(config.get<LOG_LEVEL>());

    _instance = std::make_shared<ZeroInitStructsHolder>();

    auto device = std::make_shared<ZeroDevice>(_instance);
    _devices.emplace(std::make_pair(device->getName(), device));
}

ZeroEngineBackend::~ZeroEngineBackend() = default;

const std::shared_ptr<IDevice> ZeroEngineBackend::getDevice() const {
    if (_devices.empty()) {
        return {};
    } else {
        return _devices.begin()->second;
    }
}

const std::shared_ptr<IDevice> ZeroEngineBackend::getDevice(const std::string& /*name*/) const {
    // TODO Add the search of the device by platform & slice
    return getDevice();
}

const std::vector<std::string> ZeroEngineBackend::getDeviceNames() const {
    std::vector<std::string> devicesNames;
    std::for_each(_devices.cbegin(), _devices.cend(), [&devicesNames](const auto& device) {
        devicesNames.push_back(device.first);
    });
    return devicesNames;
}

}  // namespace vpux
