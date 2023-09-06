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

project("codegen" VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(${PROJECT_NAME}
    src/config.hpp
    src/generator.cpp
    src/generator.hpp
    src/generator_c99.cpp
    src/generator_c99.hpp
    src/generator_cpp17.cpp
    src/generator_cpp17.hpp
    src/generator_typescript.cpp
    src/generator_typescript.hpp
    src/main.cpp
    src/types.cpp
    src/types.hpp
    src/utility.cpp
    src/utility.hpp
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
    -Wstrict-overflow=5
    -Wswitch-default
    -Wundef
    -Wno-variadic-macros
    -Wformat-nonliteral

    $<$<COMPILE_LANGUAGE:CXX>:
        -Wctor-dtor-privacy
        -Wsign-promo
    >

    $<$<CONFIG:Debug>:>

    $<$<CONFIG:Release>:
        -flto
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

target_compile_definitions(${PROJECT_NAME} PRIVATE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ../../include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    hlib
)

