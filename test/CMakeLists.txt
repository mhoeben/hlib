#
# MIT License
#
# Copyright (c) 2023 Maarten Hoeben
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
cmake_minimum_required(VERSION 3.0)

project("hlib_tests")

add_subdirectory(../subprojects/catch2 catch2)

add_executable(${PROJECT_NAME}
    src/buffer.cpp
    src/container.cpp
    src/cpu.cpp
    src/error.cpp
    src/event_bus.cpp
    src/event_loop.cpp
    src/event_queue.cpp
    src/fsm.cpp
    src/math.cpp
    src/memory.cpp
    src/result.cpp
    src/serial.cpp
    src/sock_addr.cpp
    src/socket.cpp
    src/string.cpp
    src/subprocess.cpp
    src/test.cpp
    src/test.hpp
    src/time.cpp
    src/type_traits.cpp
    src/uri.cpp
    src/usage.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Werror
    -Wall
    -Wextra
    -Wstrict-aliasing
    -pedantic
    -Wunreachable-code
    -Wcast-align
    -Wcast-qual
    -Winit-self
    -Wmissing-include-dirs
    -Wredundant-decls
    -Wshadow
    -Wstrict-overflow=2
    -Wswitch-default
    -Wundef
    -Wno-variadic-macros
    -Wformat-nonliteral

    $<$<COMPILE_LANGUAGE:CXX>:
        # -Wctor-dtor-privacy Incompatible with catch2.
        # -Wsign-promo        Incompatible with catch2.
    >

    $<$<CONFIG:Debug>:>

    $<$<CONFIG:Release>:
    #    -flto=auto
    >

    $<$<COMPILE_LANG_AND_ID:C,GNU>:
        -Wlogical-op
    >
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:
        -Wnoexcept
        -Wstrict-null-sentinel
    >

    $<$<COMPILE_LANG_AND_ID:C,Clang>:
        -Wstring-conversion
    >
    $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
        -Wstring-conversion
    >
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ../include
    ${CMAKE_CURRENT_BINARY_DIR}
    ../subprojects/catch2/src
    ${CMAKE_CURRENT_BINARY_DIR}/catch2/generated-includes
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Catch2::Catch2WithMain
    hlib
)

add_custom_target(run-${PROJECT_NAME}
    ALL
    COMMENT "Running ${PROJECT_NAME} tests"
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
)

