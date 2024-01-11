//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include "hlib/base.hpp"
#include "hlib/result.hpp"

extern "C"
{
struct hlib_ipc_s;
} // extern "C"

namespace hlib
{

class IPC final
{
    HLIB_NOT_COPYABLE(IPC);
    HLIB_NOT_MOVABLE(IPC);

public:
    enum Type
    {
        NamedPipe,
        SharedMemory
    };

    struct Config
    {
        Type type;
        std::string filepath;
        mode_t mode{ 0600 };
        int flags{ 0 };
        std::size_t capacity{ 0 };

        Config(Type a_type, std::string a_filepath, mode_t a_mode = 0600, int a_flags = 0);
        Config(Type a_type, std::size_t a_capacity, std::string a_filepath, mode_t a_mode = 0600);
    };

public:
    IPC(Config const& config, bool producer);
    ~IPC();

    int fd() const noexcept;

    Result<std::size_t> produce(void const* data, size_t size);
    Result<std::size_t> consume(void* data, size_t size);

protected:
    IPC(hlib_ipc_s* ipc);

private:
    hlib_ipc_s* m_ipc;
};

} // namespace hlib

