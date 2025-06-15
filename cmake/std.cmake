#
# Copyright (c) 2025-present Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
# compile the std lib as a module (for LLVM & MSVC only)

# extract the compiler directory
get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
if(MSVC)
    set(STD_BASE_DIR ${COMPILER_DIR}/../../../modules)
    set(STD_EXTENSION ixx)
else()
    set(STD_BASE_DIR ${COMPILER_DIR}/../share/libc++/v1)
    set(STD_EXTENSION cppm)
endif()

# compile the std library
add_library(std-cxx-modules STATIC)
target_sources(std-cxx-modules
        PUBLIC
        FILE_SET moduleStd
        TYPE CXX_MODULES
        BASE_DIRS ${STD_BASE_DIR}
        FILES
        ${STD_BASE_DIR}/std.${STD_EXTENSION}
        ${STD_BASE_DIR}/std.compat.${STD_EXTENSION})

if(MSVC)
    target_compile_options(std-cxx-modules PRIVATE
            /std:c++latest
            /experimental:module
            /EHsc
            /nologo
            /W4
            /c
            /fp:fast
    )
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(std-cxx-modules PRIVATE
                /Od
                /D_DEBUG
                /MTd
        )
    else()
        target_compile_options(std-cxx-modules PRIVATE
                /O2
                /MT
        )
    endif()
else()
    target_compile_options(std-cxx-modules
            PRIVATE
            -Wno-reserved-module-identifier
            -Wno-reserved-user-defined-literal
    )
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(std-cxx-modules PRIVATE
                -O0
        )
    else()
        target_compile_options(std-cxx-modules PRIVATE
                -O3
        )
    endif()
endif()