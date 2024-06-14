#
# MIT License
#
# Copyright (c) 2024 Maarten Hoeben
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
project('hlib',
    'cpp',
    version: '0.1',
    default_options: ['cpp_std=c++17'],
    license: 'MIT'
)

# Define sources
sources = files(
    'include/hlib/base.hpp',
    'include/hlib/buffer.hpp',
    'include/hlib/debug.hpp',
    'include/hlib/endian.hpp',
    'include/hlib/error.hpp',
    'include/hlib/event_bus.hpp',
    'include/hlib/event_loop.hpp',
    'include/hlib/event_queue.hpp',
    'include/hlib/file.hpp',
    'include/hlib/lock.hpp',
    'include/hlib/macro.hpp',
    'include/hlib/math.hpp',
    'include/hlib/memory.hpp',
    'include/hlib/result.hpp',
    'include/hlib/scope_guard.hpp',
    'include/hlib/signal.hpp',
    'include/hlib/sink.hpp',
    'include/hlib/sock_addr.hpp',
    'include/hlib/socket.hpp',
    'include/hlib/source.hpp',
    'include/hlib/string.hpp',
    'include/hlib/subprocess.hpp',
    'include/hlib/test.hpp',
    'include/hlib/time.hpp',
    'include/hlib/timer.hpp',
    'include/hlib/type_traits.hpp',
    'include/hlib/uri.hpp',
    'include/hlib/utility.hpp',
    'include/hlib/uuid.hpp',

    'src/hlib_buffer.cpp',
    'src/hlib_error.cpp',
    'src/hlib_event_bus.cpp',
    'src/hlib_event_loop.cpp',
    'src/hlib_event_queue.cpp',
    'src/hlib_file.cpp',
    'src/hlib_format.cpp',
    'src/hlib_math.cpp',
    'src/hlib_scope_guard.cpp',
    'src/hlib_signal.cpp',
    'src/hlib_sink.cpp',
    'src/hlib_sock_addr.cpp',
    'src/hlib_socket.cpp',
    'src/hlib_source.cpp',
    'src/hlib_string.cpp',
    'src/hlib_subprocess.cpp',
    'src/hlib_test.cpp',
    'src/hlib_time.cpp',
    'src/hlib_timer.cpp',
    'src/hlib_uri.cpp',
    'src/hlib_utility.cpp',
    'src/hlib_uuid.cpp'
)

# Compiler options
cpp_args = [
    '-Werror',
    '-Wall',
    '-Wextra',
    '-Wstrict-aliasing',
    '-pedantic',
    '-Wunreachable-code',
    '-Wcast-align',
    '-Wcast-qual',
    '-Winit-self',
    '-Wmissing-include-dirs',
    '-Wredundant-decls',
    '-Wshadow',
    '-Wstrict-overflow=2',
    '-Wswitch-default',
    '-Wundef',
    '-Wno-variadic-macros',
    '-Wformat-nonliteral',
    '-Wctor-dtor-privacy',
    '-Wsign-promo',
    '-Wstring-conversion'
]

# Define the library
hlib = library(
    'hlib',
    sources,
    cpp_args: cpp_args,
    include_directories: [
        'include'
    ],
    dependencies: [
        dependency('uuid')
    ],
    pic:true,
    install: true
)

# Set default build type
default_options = ['buildtype=release']

# Declare a dependency for use in subprojects.
hlib_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: hlib
)

# if not meson.is_subproject()
#     install_headers('include/hlib/', subdir: 'hlib')
# 
#     pkgc = import('pkgconfig')
#     pkgc.generate(name: 'mhoeben_hlib',
#         version: meson.project_version(),
#         description: 'Commonly used C++ classes'
#     )
# endif
