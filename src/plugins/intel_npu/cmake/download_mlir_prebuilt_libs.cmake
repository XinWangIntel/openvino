# Copyright (C) 2018-2025 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

if(ENABLE_MLIR_COMPILER)
    if(ENABLE_SYSTEM_NPU_MLIR_COMPILER)
        message(STATUS "Using system NPU MLIR compiler libraries, skip download")
    else()
        message(STATUS "Downloading prebuilt NPU MLIR compiler libraries")
        set(MLIR_COMPILER_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/temp/mlir_compiler_libs")
        file(MAKE_DIRECTORY "${MLIR_COMPILER_LIBS_DIR}")

        set(MLIR_COMPILER_LIBS_URL "https://af01p-ir.devtools.intel.com/artifactory/movidius_vpu_ip2_git_lfs_store-ir-local/xin-test/prebuilt_mlir_libs_25ww20.zip")
        set(MLIR_COMPILER_LIBS_ZIP "${MLIR_COMPILER_LIBS_DIR}/prebuilt_mlir_libs_25ww20.zip")
        set(MLIR_COMPILER_LIBS_DIR_UNZIPPED "${MLIR_COMPILER_LIBS_DIR}/prebuilt_mlir_libs_25ww20")

        # Check if the prebuilt MLIR compiler libraries not exist
        if(NOT EXISTS "${MLIR_COMPILER_LIBS_DIR_UNZIPPED}")
            # Download the prebuilt MLIR compiler libraries, if failure, show error message
            # and exit
            message(STATUS "Downloading prebuilt MLIR compiler libraries from ${MLIR_COMPILER_LIBS_URL}")
            file(DOWNLOAD "${MLIR_COMPILER_LIBS_URL}" "${MLIR_COMPILER_LIBS_ZIP}"
                TIMEOUT 3600
                LOG log_output
                STATUS download_status
                SHOW_PROGRESS)
            list(GET download_status 0 download_result)
            if(NOT download_result EQUAL 0)
                message(FATAL_ERROR "Download failed!\nStatus: ${download_status}\nLog: ${log_output}")
            else()
                message(STATUS "Download completed: ${MLIR_COMPILER_LIBS_ZIP}")
            endif()

            message(STATUS "Unzipping prebuilt MLIR compiler libraries to ${MLIR_COMPILER_LIBS_DIR_UNZIPPED}")
            file(ARCHIVE_EXTRACT INPUT "${MLIR_COMPILER_LIBS_ZIP}" DESTINATION "${MLIR_COMPILER_LIBS_DIR}")
            file(REMOVE "${MLIR_COMPILER_LIBS_ZIP}")
        else()
            message(STATUS "Prebuilt MLIR compiler libraries already exist, skip download")
        endif()

        if(WIN32)
            file(COPY ${MLIR_COMPILER_LIBS_DIR_UNZIPPED}/Windows10/md/npu_mlir_compiler.dll
                DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            )
            message(STATUS "Copying prebuilt MLIR compiler libraries npu_mlir_compiler.dll to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
        else()
            file(COPY ${MLIR_COMPILER_LIBS_DIR_UNZIPPED}/ubuntu22/libnpu_mlir_compiler.so
                DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            )
            message(STATUS "Copying prebuilt MLIR compiler libraries libnpu_mlir_compiler.so to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
        endif()
    endif()
endif()
