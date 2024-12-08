# Copyright (C) 2018-2025 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

file(GLOB SOURCES "src/*.cpp" "include/*.hpp")
file(GLOB_RECURSE NPUW_SOURCES "npuw/*.cpp" "npuw/*.hpp")

set(NPU_PLUGIN_TARGET "openvino_intel_npu_plugin")
set(NPU_PLUGIN_ENGINE_SOURCE_FILE "src/plugin.cpp")
set(TARGET_NAME ${NPU_PLUGIN_TARGET})

#
# Plugin library
#

ov_add_plugin(NAME ${NPU_PLUGIN_TARGET}
    DEVICE_NAME ${NPU_DEVICE_NAME}
    SOURCES ${SOURCES} ${NPUW_SOURCES}
    VERSION_DEFINES_FOR ${NPU_PLUGIN_ENGINE_SOURCE_FILE}
)

set_target_properties(${TARGET_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ${ENABLE_LTO})
ov_add_clang_format_target(${TARGET_NAME}_clang FOR_TARGETS ${TARGET_NAME})

list(APPEND CMAKE_PREFIX_PATH $ENV{VPUX_HOME}/build-x86_64/RelWithDebInfo/lib/cmake/mlir)
find_package(MLIR REQUIRED CONFIG)
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
    PRIVATE
        openvino_npu_driver_compiler_adapter
        openvino_npu_level_zero_backend
    PRIVATE
  MLIRAnalysis
  MLIRBuiltinToLLVMIRTranslation
  MLIRExecutionEngine
  MLIRIR
  MLIRJitRunner
  MLIRLLVMDialect
  MLIRLLVMToLLVMIRTranslation
  MLIRToLLVMIRTranslationRegistration
  MLIRParser
  MLIRTargetLLVMIRExport
  MLIRSupport
  LLVMSupport
  LLVMCore
  LLVMX86CodeGen
  LLVMX86AsmParser
)
message(STATUS "MLIR_LIBS: ${MLIR_LIBS}")
get_target_property(MLIRExecutionEngine_INCLUDE_DIRS MLIRExecutionEngine INCLUDE_DIRECTORIES)
get_target_property(MLIRJitRunner_INCLUDE_DIRS MLIRJitRunner INCLUDE_DIRECTORIES)
message(STATUS "MLIRExecutionEngine includes ${MLIRExecutionEngine_INCLUDE_DIRS}")
message(STATUS "MLIRJitRunner includes ${MLIRJitRunner_INCLUDE_DIRS}")
if (TARGET MLIRExecutionEngine)
    message(STATUS "MLIRExecutionEngine exists")
#else()
    #message(STATUS "MLIRExecutionEngine does not exist")
endif()

if(ENABLE_IMD_BACKEND)
    target_compile_definitions(${TARGET_NAME} PRIVATE ENABLE_IMD_BACKEND)
endif()

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_NAME} PRIVATE OPENVINO_STATIC_LIBRARY)
endif()

target_link_libraries(${TARGET_NAME}
    PRIVATE
        openvino::npu_al
        openvino::npu_common
)

target_include_directories(${TARGET_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

cross_compiled_file(${TARGET_NAME}
        ARCH AVX2 ANY
                    npuw/util_xarch.cpp
        API         npuw/util_xarch.hpp
        NAME        unpack_i4i8 unpack_u4i8 unpack_i4f16 unpack_i4f16_scale unpack_i4f16_z unpack_u4f16 unpack_u4f16_scale_zp unpack_u4f16_asymm_zp unpack_u4f16_z unpack_u4f32 unpack_i8f16 unpack_i8f16_scale unpack_u8f16 to_f16 copy_row_as_column
        NAMESPACE   ov::npuw::util::XARCH
)

ov_add_api_validator_post_build_step(TARGET ${NPU_PLUGIN_TARGET})
