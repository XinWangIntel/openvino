# Copyright (C) 2018-2025 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#
# zeroApi
#

if(ENABLE_NPU_PLUGIN_ENGINE)
    add_library(level-zero-ext INTERFACE)
    set_property(TARGET level-zero-ext APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/level-zero-ext/>)
    add_library(LevelZero::NPUExt ALIAS level-zero-ext)
endif()

#
# yaml-cpp
#

if(ENABLE_INTEL_NPU_PROTOPIPE)
    set(YAML_BUILD_SHARED_LIBS OFF)
    add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)
    # NB: Suppress warnings in yaml-cpp
    if(SUGGEST_OVERRIDE_SUPPORTED)
        target_compile_options(yaml-cpp PRIVATE -Wno-suggest-override)
    endif()
endif()

if(ENABLE_LLVM_BACKEND)
    set(TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jit-tools)
    set(TEST_DEST_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    add_custom_command(
        OUTPUT ${TEST_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_SOURCE_DIR} ${TEST_DEST_DIR}
        COMMENT "Copying jit files to bin directory"
    )

    add_custom_target(copy_test_files ALL DEPENDS ${TEST_DEST_DIR})
endif()