#
# Copyright (c) 2025-present Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
function(compile_options TARGET_NAME )
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
                /nologo
                /W2
                /WX
                /std:c++latest
                /experimental:module
                /fp:fast
                /DUNICODE
                /D_UNICODE
        )
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${TARGET_NAME} PRIVATE
                    /Od
                    /GS
                    /Ob0
                    /RTC1
                    /MDd
                    /Wv:18
            )
            add_definitions(-D_DEBUG)
        else()
            target_compile_options(${TARGET_NAME} PRIVATE
                    /O2
                    /GS-
                    /Gy
                    /Oi
                    /Ot
                    /MD
                    /Wv:18
            )
        endif()
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
                -Wno-deprecated-declarations
                -Wno-nullability-completeness
                -Werror
        )
        target_link_libraries(${TARGET_NAME} -static)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_definitions(_DEBUG)
            target_compile_options(${TARGET_NAME} PRIVATE
                    -O0
            )
        else()
            target_compile_options(${TARGET_NAME} PRIVATE
                    -O3
                    -DNDEBUG
            )
        endif()
    endif()
endfunction()