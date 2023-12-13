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

project("hlib" VERSION 0.1 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type." FORCE)
endif()

if (NOT BUILD_SHARED_LIBS)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

find_package(OpenSSL REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(third_party/fmt fmt)
add_subdirectory(tools/codegen codegen)

add_library(${PROJECT_NAME} # STATIC, use BUILD_SHARED_LIBS for SHARED.
    include/hlib/base.hpp
    include/hlib/buffer.hpp
    include/hlib/codec.hpp
    include/hlib/config.hpp
    include/hlib/endian.hpp
    include/hlib/error.hpp
    include/hlib/event_loop.hpp
    include/hlib/event_queue.hpp
    include/hlib/file.hpp
    include/hlib/file_server.hpp
    include/hlib/http_server.hpp
    include/hlib/format.hpp
    include/hlib/json.hpp
    include/hlib/lock.hpp
    include/hlib/log.hpp
    include/hlib/math.hpp
    include/hlib/memory.hpp
    include/hlib/result.hpp
    include/hlib/scope_guard.hpp
    include/hlib/signal.hpp
    include/hlib/sock_addr.hpp
    include/hlib/socket.hpp
    include/hlib/string.hpp
    include/hlib/subprocess.hpp
    include/hlib/time.hpp
    include/hlib/timer.hpp
    include/hlib/type_traits.hpp
    include/hlib/uri.hpp
    include/hlib/utility.hpp
    include/hlib/uuid.hpp
    include/hlib/web_socket_server.hpp

    include/hlib/c/base.h
    include/hlib/c/buffer.h
    include/hlib/c/codec.h
    include/hlib/c/codec_binary.h
    include/hlib/c/endian.h
    include/hlib/c/error.h
    include/hlib/c/vector.h

    src/hlib_base.cpp
    src/hlib_buffer.cpp
    src/hlib_c_impl.cpp
    src/hlib_codec.cpp
    src/hlib_codec_binary.cpp
    src/hlib_codec_binary.hpp
    src/hlib_codec_json.cpp
    src/hlib_codec_json.hpp
    src/hlib_endian.cpp
    src/hlib_error.cpp
    src/hlib_event_loop.cpp
    src/hlib_event_queue.cpp
    src/hlib_file.cpp
    src/hlib_file_server.cpp
    src/hlib_format.cpp
    src/hlib_http_server.cpp
    src/hlib_json.cpp
    src/hlib_log.cpp
    src/hlib_math.cpp
    src/hlib_scope_guard.cpp
    src/hlib_signal.cpp
    src/hlib_sock_addr.cpp
    src/hlib_socket.cpp
    src/hlib_string.cpp
    src/hlib_subprocess.cpp
    src/hlib_time.cpp
    src/hlib_timer.cpp
    src/hlib_uri.cpp
    src/hlib_utility.cpp
    src/hlib_uuid.cpp
    src/hlib_web_socket_server.cpp
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
        -Wctor-dtor-privacy
        -Wsign-promo
    >

    $<$<CONFIG:Debug>:>

    $<$<CONFIG:Release>:
        -flto=auto
    >

    $<$<COMPILE_LANG_AND_ID:C,GNU>:
        -Wlogical-op
        -Wmissing-prototypes
    >
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:
        -Wnoexcept
        -Wstrict-null-sentinel
    >

    $<$<COMPILE_LANG_AND_ID:C,Clang>:
        -Wstring-conversion
        -Wmissing-prototypes
    >
    $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
        -Wstring-conversion
    >
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    third_party/fmt/include
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    fmt
    uuid
    ${OPENSSL_LIBRARIES}
)

if (HLIB_ENABLE_TESTS)
    add_subdirectory(test)
endif()

