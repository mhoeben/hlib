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
#include "hlib/ipc.hpp"
#include "hlib/c/ipc.h"
#include "hlib/error.hpp"

using namespace hlib;

//
// Public (IPC::Config)
//
IPC::Config::Config(Type a_type, std::string a_filepath, mode_t a_mode, int a_flags)
    : type{ a_type }
    , filepath(a_filepath)
    , mode{ a_mode }
    , flags{ a_flags }
    , capacity{ 0 }
{
}

IPC::Config::Config(Type a_type, std::size_t a_capacity, std::string a_filepath, mode_t a_mode)
    : type{ a_type }
    , filepath(a_filepath)
    , mode{ a_mode }
    , flags{ 0 }
    , capacity{ a_capacity }
{
}

//
// Public (IPC)
//
IPC::IPC(Config const& config, bool producer)
{
    hlib_ipc_config_t cfg;
    cfg.type = static_cast<hlib_ipc_type_t>(config.type);
    cfg.filepath = config.filepath.c_str();
    cfg.mode = config.mode;
    cfg.flags = config.flags;
    cfg.capacity = config.capacity;

    m_ipc = hlib_ipc_create(&cfg, producer);
    if (nullptr == m_ipc) {
        throw make_system_error(errno, "hlib_ipc_create() failed");
    }
}

IPC::~IPC()
{
    hlib_ipc_destroy(m_ipc);
}

int IPC::fd() const noexcept
{
    return hlib_ipc_get_fd(m_ipc);
}

Result<size_t> IPC::produce(void const* data, size_t size)
{
    ssize_t r = hlib_ipc_produce(m_ipc, data, size);
    if (-1 == r) {
        return make_system_error(errno, "hlib_ipc_produce() failed");
    }

    return r;
}

Result<size_t> IPC::consume(void* data, size_t size)
{
    ssize_t r = hlib_ipc_consume(m_ipc, data, size);
    if (-1 == r) {
        return make_system_error(errno, "hlib_ipc_consume() failed");
    }

    return r;
}

