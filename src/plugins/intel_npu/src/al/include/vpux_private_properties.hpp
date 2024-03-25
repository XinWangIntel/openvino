//
// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: Apache 2.0
//

#pragma once

#include "openvino/runtime/properties.hpp"

namespace ov {
namespace intel_npu {

namespace Platform {

constexpr std::string_view AUTO_DETECT = "AUTO_DETECT";  // Auto detection
constexpr std::string_view NPU3700 = "3700";             // VPU30XX
constexpr std::string_view NPU3720 = "3720";             // VPU37XX

/**
 * @brief Converts the given platform value to the standard one.
 * @details The same platform value can be defined in multiple ways (e.g. "3720" vs "VPU3720" vs "NPU3720"). The current
 * function converts the prefixed variants to the non-prefixed ones in order to enable the comparison between platform
 * values.
 *
 * The values already found in the standard form are returned as they are.
 *
 * @param platform The value to be converted.
 * @return The same platform value given as parameter but converted to the standard form.
 */
inline std::string standardize(const std::string_view platform) {
    constexpr std::string_view VPUPrefix = "VPU";
    constexpr std::string_view NPUPrefix = "NPU";

    if (!platform.compare(0, VPUPrefix.length(), VPUPrefix) || !platform.compare(0, NPUPrefix.length(), NPUPrefix)) {
        return std::string(platform).substr(NPUPrefix.length());
    }

    return std::string(platform);
}

}  // namespace Platform

/**
 * @enum ColorFormat
 * @brief Extra information about input color format for preprocessing
 * @note Configuration API v 2.0
 */
enum ColorFormat : uint32_t {
    RAW = 0u,  ///< Plain blob (default), no extra color processing required
    RGB,       ///< RGB color format
    BGR,       ///< BGR color format, default in DLDT
    RGBX,      ///< RGBX color format with X ignored during inference
    BGRX,      ///< BGRX color format with X ignored during inference
};

/**
 * @brief Prints a string representation of ov::intel_npu::ColorFormat to a stream
 * @param out An output stream to send to
 * @param fmt A color format value to print to a stream
 * @return A reference to the `out` stream
 * @note Configuration API v 2.0
 */
inline std::ostream& operator<<(std::ostream& out, const ColorFormat& fmt) {
    switch (fmt) {
    case ColorFormat::RAW: {
        out << "RAW";
    } break;
    case ColorFormat::RGB: {
        out << "RGB";
    } break;
    case ColorFormat::BGR: {
        out << "BGR";
    } break;
    case ColorFormat::RGBX: {
        out << "RGBX";
    } break;
    case ColorFormat::BGRX: {
        out << "BGRX";
    } break;
    default:
        out << static_cast<uint32_t>(fmt);
        break;
    }
    return out;
}

/**
 * @brief [Only for VPUX Plugin]
 * Type: string, default is MLIR.
 * Type of VPU compiler to be used for compilation of a network
 * @note Configuration API v 2.0
 */
enum class CompilerType { MLIR, DRIVER };

/**
 * @brief Prints a string representation of ov::intel_npu::CompilerType to a stream
 * @param out An output stream to send to
 * @param fmt A compiler type value to print to a stream
 * @return A reference to the `out` stream
 * @note Configuration API v 2.0
 */
inline std::ostream& operator<<(std::ostream& out, const CompilerType& fmt) {
    switch (fmt) {
    case CompilerType::MLIR: {
        out << "MLIR";
    } break;
    case CompilerType::DRIVER: {
        out << "DRIVER";
    } break;
    default:
        out << static_cast<uint32_t>(fmt);
        break;
    }
    return out;
}

/**
 * @brief [Only for VPUX Plugin]
 * Type: String. Default is "AUTO".
 * This option is added for enabling ELF backend.
 * Possible values: "AUTO", "YES", "NO".
 */

enum class ElfCompilerBackend {
    AUTO = 0,
    NO = 1,
    YES = 2,
};

/**
 * @brief Prints a string representation of ov::intel_npu::ElfCompilerBackend to a stream
 * @param out An output stream to send to
 * @param fmt A elf compiler backend value to print to a stream
 * @return A reference to the `out` stream
 * @note Configuration API v 2.0
 */
inline std::ostream& operator<<(std::ostream& out, const ElfCompilerBackend& fmt) {
    switch (fmt) {
    case ElfCompilerBackend::AUTO: {
        out << "AUTO";
    } break;
    case ElfCompilerBackend::NO: {
        out << "NO";
    } break;
    case ElfCompilerBackend::YES: {
        out << "YES";
    } break;
    default:
        out << static_cast<uint32_t>(fmt);
        break;
    }
    return out;
}

/**
 * @brief [Only for VPUX Plugin]
 * Type: string, default is MODEL.
 * Type of profiling to execute. Can be Model (default) or INFER (based on npu timestamps)
 * @note Configuration API v 2.0
 */
enum class ProfilingType { MODEL, INFER };

/**
 * @brief Prints a string representation of ov::intel_npu::ProfilingType to a stream
 * @param out An output stream to send to
 * @param fmt A profiling type value to print to a stream
 * @return A reference to the `out` stream
 * @note Configuration API v 2.0
 */
inline std::ostream& operator<<(std::ostream& out, const ProfilingType& fmt) {
    switch (fmt) {
    case ProfilingType::MODEL: {
        out << "MODEL";
    } break;
    case ProfilingType::INFER: {
        out << "INFER";
    } break;
    default:
        out << static_cast<uint32_t>(fmt);
        break;
    }
    return out;
}

/**
 * @brief [Only for VPUX Plugin]
 * Type: Arbitrary string.
 * This option allows to specify device.
 * The plugin accepts any value given through this option. If the device is not available, either the driver or the
 * compiler will throw an exception depending on the flow running at the time.
 */
static constexpr ov::Property<std::string> platform{"NPU_PLATFORM"};

/**
 * @brief
 * Type: integer, default is -1
 * Subdevice ID or stepping number of the device
 */
static constexpr ov::Property<int64_t> stepping{"NPU_STEPPING"};

/**
 * @brief [Only for VPUX Plugin]
 * Type: string, default is MLIR for DEVELOPER_BUILD, DRIVER otherwise.
 * Type of VPU compiler to be used for compilation of a network
 */
static constexpr ov::Property<CompilerType> compiler_type{"NPU_COMPILER_TYPE"};

static constexpr ov::Property<std::string> compilation_mode{"NPU_COMPILATION_MODE"};

/**
 * @brief [Only for VPUX compiler]
 * Type: std::string, default is empty.
 * Config for HW-mode's pipeline
 * Available values: low-precision=true/low-precision=false
 */
static constexpr ov::Property<std::string> compilation_mode_params{"NPU_COMPILATION_MODE_PARAMS"};

/**
 * @brief [Only for VPUX Plugin]
 * Type: integer, default is None
 * Number of DPU groups
 */
static constexpr ov::Property<int64_t> dpu_groups{"NPU_DPU_GROUPS"};

/**
 * @brief
 * Type: integer, default is -1
 * Number of max tiles supported by device
 */
static constexpr ov::Property<int64_t> max_tiles{"NPU_MAX_TILES"};

/**
 * @brief [Only for VPUX Plugin]
 * Type: integer, default is None
 * Number of DMA engines
 */
static constexpr ov::Property<int64_t> dma_engines{"NPU_DMA_ENGINES"};

/**
 * @brief
 * Type: Boolean. Default is "NO".
 * Determines which branch we use for dynamic shapes.
 * If set to 'YES', we immediately apply the bounds so that we have a static shape for further work.
 * If not, we store the related information in TensorAttr and the IE representation looks
 * like this: tensor<1x?x3xf32, {bounds = [1, 18, 3], ..}>.
 * Possible values: "YES", "NO".
 */
static constexpr ov::Property<std::string> dynamic_shape_to_static{"NPU_DYNAMIC_SHAPE_TO_STATIC"};

/**
 * @brief [Only for VPUX Plugin]
 * Type: string, default is empty.
 * MODEL - model layer profiling is done
 * INFER - vpu inference performance numbers are measured
 * Model layers profiling are used if this string is empty
 */
static constexpr ov::Property<ProfilingType> profiling_type{"NPU_PROFILING_TYPE"};

/**
 * @brief
 * Type: String. Default is "AUTO".
 * This option is added for enabling ELF backend.
 * Possible values: "AUTO", "YES", "NO".
 */
static constexpr ov::Property<ElfCompilerBackend> use_elf_compiler_backend{"NPU_USE_ELF_COMPILER_BACKEND"};

/**
 * @brief [Only for VPUX Plugin]
 * Type: integer, default is 1
 * This option allows to omit creating an executor and therefore to omit running an inference when its value is 0
 */
static constexpr ov::Property<int64_t> create_executor{"NPU_CREATE_EXECUTOR"};

/**
 * @brief [Only for VPUX compiler]
 * Type: std::string, default is empty.
 * Config for Backend pipeline

 * Available values: enable-memory-side-cache=true/false
 * Available values: enable-partial-workload-management=true/false
 */
static constexpr ov::Property<std::string> backend_compilation_params{"NPU_BACKEND_COMPILATION_PARAMS"};

}  // namespace intel_npu
}  // namespace ov
