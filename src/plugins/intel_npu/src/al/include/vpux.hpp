//
// Copyright (C) 2024 Intel Corporation.
// SPDX-License-Identifier: Apache 2.0
//

#pragma once

#include <cstdint>

#include "openvino/runtime/properties.hpp"
#include "sync_infer_request.hpp"
#include "vpux/al/config/config.hpp"
#include "vpux/al/icompiled_model.hpp"
#include "vpux/al/icompiler.hpp"

namespace vpux {

using intel_npu::OptionsDesc;

//------------------------------------------------------------------------------
class IDevice;

class IEngineBackend : public std::enable_shared_from_this<IEngineBackend> {
public:
    /** @brief Get device, which can be used for inference. Backend responsible for selection. */
    virtual const std::shared_ptr<IDevice> getDevice() const;
    /** @brief Search for a specific device by name */
    virtual const std::shared_ptr<IDevice> getDevice(const std::string& specificDeviceName) const;
    /** @brief Get device, which is configured/suitable for provided params */
    virtual const std::shared_ptr<IDevice> getDevice(const ov::AnyMap& paramMap) const;
    /** @brief Provide a list of names of all devices, with which user can work directly */
    virtual const std::vector<std::string> getDeviceNames() const;
    /** @brief Get name of backend */
    virtual const std::string getName() const = 0;
    /** @brief Register backend-specific options */
    virtual void registerOptions(OptionsDesc& options) const;

protected:
    virtual ~IEngineBackend() = default;
};

//------------------------------------------------------------------------------

class IExecutor {
public:
    virtual ~IExecutor() = default;
};

//------------------------------------------------------------------------------

class IDevice : public std::enable_shared_from_this<IDevice> {
public:
    using Uuid = ov::device::UUID;

    virtual std::shared_ptr<IExecutor> createExecutor(
        const std::shared_ptr<const NetworkDescription>& networkDescription,
        const Config& config) = 0;

    virtual std::string getName() const = 0;
    virtual std::string getFullDeviceName() const = 0;
    virtual Uuid getUuid() const;
    virtual uint32_t getSubDevId() const;
    virtual uint32_t getMaxNumSlices() const;
    virtual uint64_t getAllocMemSize() const;
    virtual uint64_t getTotalMemSize() const;
    virtual uint32_t getDriverVersion() const;

    virtual std::shared_ptr<SyncInferRequest> createInferRequest(
        const std::shared_ptr<const vpux::ICompiledModel>& compiledModel,
        const std::shared_ptr<IExecutor>& executor,
        const Config& config) = 0;

protected:
    virtual ~IDevice() = default;
};

}  // namespace vpux
