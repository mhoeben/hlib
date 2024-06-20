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
#include "hlib/emitter.hpp"
#include <cstring>

using namespace hlib;

//
// Public
//
std::size_t Emitter::available() const noexcept
{
    assert(m_progress <= size());
    return size() - m_progress;
}

bool Emitter::empty() const noexcept
{
    assert(m_progress <= size());
    return m_progress == size();
}

void const* Emitter::provide() noexcept
{
    assert(m_progress <= this->size());

    return static_cast<std::uint8_t const*>(this->data()) + m_progress;
}

void const* Emitter::provide(std::size_t size) noexcept
{
    assert(m_progress + size <= this->size());

    void const* data = provide();
    m_progress += size;
    return data;
}

