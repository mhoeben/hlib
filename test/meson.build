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

# Define sources
sources = files(
    'src/buffer.cpp',
    'src/container.cpp',
    'src/endian.cpp',
    'src/error.cpp',
    'src/event_bus.cpp',
    'src/event_loop.cpp',
    'src/event_queue.cpp',
    'src/math.cpp',
    'src/result.cpp',
    'src/sock_addr.cpp',
    'src/socket.cpp',
    'src/string.cpp',
    'src/subprocess.cpp',
    'src/test.cpp',
    'src/test.hpp',
    'src/time.cpp',
    'src/type_traits.cpp',
    'src/uri.cpp'
)

# Clang compiler options
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

# Define the executable
hlib_tests = executable(
    'hlib_tests',
    sources,
    cpp_args: cpp_args,
    include_directories: [
        '../include'
    ],
    dependencies: [
        hlib_dep,
        catch2.get_variable('catch2_with_main_dep')
    ]
)

# Set default build type
default_options = ['buildtype=release']

# Run unit tests
test('HLib unit tests', hlib_tests)
