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
#include <cstring>
#include <limits>

namespace hlib
{

class Sink
{
public:
    virtual std::size_t size() const noexcept = 0;
    virtual void* resize(std::size_t size) noexcept = 0;

    std::size_t maximum() const noexcept
    {
        return m_maximum;
    }

    void setMaximum(std::size_t maximum) noexcept
    {
        m_maximum = maximum;
    }

    bool full() const noexcept
    {
        return 0 == m_maximum || this->size() == m_maximum;
    }

    std::size_t headroom() const noexcept
    {
        assert(m_maximum > 0 && this->size() <= m_maximum);
        return m_maximum - this->size();
    }

    std::size_t headroom(std::size_t limit) const noexcept
    {
        if (0 == m_maximum) {
            return limit;
        }

        return std::min(headroom(), limit);
    }

    void* extend(std::size_t size) noexcept
    {
        assert(0 == m_maximum || this->size() + size <= m_maximum);

        std::size_t before_resize = this->size();
        std::uint8_t* ptr = static_cast<std::uint8_t*>(resize(before_resize + size));
        if (nullptr == ptr) {
            return nullptr;
        }

        return ptr + before_resize;
    }

    std::size_t produce(void* data, std::size_t size) noexcept
    {
        void* ptr = this->extend(size);
        if (nullptr == ptr) {
            return 0;
        }

        memcpy(ptr, data, size);
        return this->size();
    }

protected:
    Sink() = default;
    Sink(std::size_t maximum)
        : m_maximum{ maximum }
    {
    }

    ~Sink() = default;

private:
    std::size_t m_maximum{ std::numeric_limits<std::size_t>::max() };
};

} // namespace hlib
 
