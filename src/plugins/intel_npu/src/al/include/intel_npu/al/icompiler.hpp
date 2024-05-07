// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

// Compiler Interface

#pragma once

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "intel_npu/al/config/config.hpp"
#include "openvino/core/partial_shape.hpp"
#include "openvino/core/type/element_type.hpp"
#include "openvino/runtime/common.hpp"
#include "openvino/runtime/profiling_info.hpp"

namespace intel_npu {

/**
 * @brief A helper structure used for storing the metadata found within the I/O nodes.
 * @details The "legacyName" attribute holds the name most commonly used as map key for multiple structures.
 * This value also corresponds to the identifier used by the OpenVINO 1.0 API.
 *
 * "originalShape" corresponds to the shape registered in the graph, while "transposedShape" holds the shape obtained
 * upon applying a transposition corresponding to the legacy layout value. Use the "transposedShape" one if not sure
 * which one you need.
 */
struct IODescriptor {
    std::string nameFromCompiler;
    ov::element::Type_t precision;
    ov::PartialShape shapeFromCompiler;
    bool isStateInput = false;
    bool isStateOutput = false;
    bool isShapeTensor = false;
    std::optional<size_t> relatedDescriptorIndex;

    std::string nodeFriendlyName;
    std::unordered_set<std::string> outputTensorNames;
    ov::PartialShape shapeFromIRModel;
};

struct NetworkMetadata final {
    std::string name;

    std::vector<IODescriptor> inputs;
    std::vector<IODescriptor> outputs;
    std::vector<IODescriptor> profilingOutputs;

    int numStreams = 1;

    std::optional<size_t> findByName(const std::vector<IODescriptor>& descriptors, const std::string_view targetName) {
        for (size_t descriptorIndex = 0; descriptorIndex < descriptors.size(); ++descriptorIndex) {
            if (descriptors.at(descriptorIndex).nameFromCompiler == targetName) {
                return std::optional(descriptorIndex);
            }
        }

        return std::nullopt;
    }

    void bindRelatedDescriptors() {
        size_t ioIndex = 0;

        for (IODescriptor& input : inputs) {
            if (input.relatedDescriptorIndex.has_value()) {
                ++ioIndex;
                continue;
            }

            if (input.isStateInput) {
                const std::optional<size_t> relatedDescriptorIndex = findByName(outputs, input.nameFromCompiler);

                if (relatedDescriptorIndex.has_value()) {
                    input.relatedDescriptorIndex = *relatedDescriptorIndex;
                    outputs.at(*relatedDescriptorIndex).relatedDescriptorIndex = ioIndex;
                }
            } else if (input.isShapeTensor) {
                const std::optional<size_t> relatedDescriptorIndex = findByName(inputs, input.nameFromCompiler);

                if (relatedDescriptorIndex.has_value() && *relatedDescriptorIndex != ioIndex) {
                    input.relatedDescriptorIndex = *relatedDescriptorIndex;
                    inputs.at(*relatedDescriptorIndex).relatedDescriptorIndex = ioIndex;
                }
            }

            ++ioIndex;
        }

        ioIndex = 0;

        for (IODescriptor& output : outputs) {
            if (output.relatedDescriptorIndex.has_value()) {
                ++ioIndex;
                continue;
            }

            if (output.isShapeTensor) {
                const std::optional<size_t> relatedDescriptorIndex = findByName(outputs, output.nameFromCompiler);

                if (relatedDescriptorIndex.has_value() && *relatedDescriptorIndex != ioIndex) {
                    output.relatedDescriptorIndex = *relatedDescriptorIndex;
                    outputs.at(*relatedDescriptorIndex).relatedDescriptorIndex = ioIndex;
                }
            }

            ++ioIndex;
        }
    }
};  // namespace intel_npu

/**
 * @struct NetworkDescription
 * @brief The object returned by the compiler
 * to provide such information about a network as description of inputs and outputs,
 * name and compiled network in a format executable by device
 */
struct NetworkDescription final {
    NetworkDescription(std::vector<uint8_t>&& compiledNetwork, NetworkMetadata&& metadata)
        : compiledNetwork(std::move(compiledNetwork)),
          metadata(std::move(metadata)) {}
    // Force move semantics to prevent blob copies
    NetworkDescription(const NetworkDescription&) = delete;
    NetworkDescription(NetworkDescription&&) = default;
    NetworkDescription& operator=(const NetworkDescription&) = delete;
    NetworkDescription& operator=(NetworkDescription&&) = default;
    ~NetworkDescription() = default;

    std::vector<uint8_t> compiledNetwork;

    NetworkMetadata metadata;
};

/**
 * @interface ICompiler
 * @brief An interface to be implemented by a concrete compiler to provide
 * methods for preparing a network for execution on a NPU device
 */
class ICompiler : public std::enable_shared_from_this<ICompiler> {
public:
    /**
     * @brief Returns the maximum OpenVino opset version supported by the compiler
     * @return opset version e.g. 11 for opset11
     */
    virtual uint32_t getSupportedOpsetVersion() const = 0;

    /**
     * @brief Transforms a network from the OpenVINO model representation to a format executable
     * by a NPU device
     * @param model a shared pointer to the OpenVINO model to be compiled
     * @param config a reference to NPUConfig containing plugin config options
     *        including config options related to compilation
     * @return a shared pointer on an object implementing NetworkDescription interface
     */
    virtual NetworkDescription compile(const std::shared_ptr<const ov::Model>& model, const Config& config) const = 0;

    /**
     * @brief Returns information about supported layers of the network passed
     * @param model The model to be queried
     * @param config A reference to NPUConfig containing plugin config options
     *        including config options related to compilation
     * @returns SupportedOpsMap structure with information about supported layers
     */
    virtual ov::SupportedOpsMap query(const std::shared_ptr<const ov::Model>& model, const Config& config) const = 0;

    /**
     * @brief Parses already compiled network to extract meta information:
     *        inputs and outputs descriptions
     * @param network compiled network represented as a vector of char
     * @param config a reference to NPUConfig containing plugin config options
     *        Note: compilation options will be ignored,
     *        since the network is already compiled
     * @param netName a reference to the string describing network name
     *        to be used for creating network description
     * @return a shared pointer on an object implementing NetworkDescription interface
     */
    virtual NetworkMetadata parse(const std::vector<uint8_t>& network, const Config& config) const = 0;

    virtual std::vector<ov::ProfilingInfo> process_profiling_output(const std::vector<uint8_t>& profData,
                                                                    const std::vector<uint8_t>& network,
                                                                    const Config& config) const = 0;

protected:
    virtual ~ICompiler() = default;
};

}  // namespace intel_npu
