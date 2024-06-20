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
#include "hlib/receiver.hpp"

using namespace hlib;

//
// Implementation
//
Receiver::Receiver(std::size_t maximum)
    : m_maximum{ maximum }
{
}

//
// Public
//
bool Receiver::full() const noexcept
{
    return Receiver::UnspecifiedCapacity == m_maximum || this->size() == m_maximum;
}

std::size_t Receiver::headroom() const noexcept
{
    // Don't use headroom if the sink's maximum is unspecified.
    assert(Receiver::UnspecifiedCapacity != m_maximum);
    assert(this->size() <= m_maximum);

    return m_maximum - this->size();
}

std::size_t Receiver::headroom(std::size_t limit) const noexcept
{
    if (Receiver::UnspecifiedCapacity == m_maximum) {
        return limit;
    }

    return std::min(headroom(), limit);
}

void* Receiver::accept(std::size_t size) noexcept
{
    assert(Receiver::UnspecifiedCapacity == m_maximum || this->size() + size <= m_maximum);

    std::size_t before_resize = this->size();
    std::uint8_t* ptr = static_cast<std::uint8_t*>(resize(before_resize + size));
    if (nullptr == ptr) {
        return nullptr;
    }

    return ptr + before_resize;
}

