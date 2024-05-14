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

namespace hlib
{

class Source
{
public:
    virtual std::size_t size() const noexcept = 0;
    virtual void const* data() const noexcept = 0;

    bool empty() const noexcept
    {
        assert(m_progress <= size());
        return m_progress == size();
    }

    std::size_t available() const noexcept
    {
        assert(m_progress <= size());
        return size() - m_progress;
    }

    std::size_t progress() const noexcept
    {
        return m_progress;
    }

    void setProgress(std::size_t progress) noexcept
    {
        assert(progress <= size());
        m_progress = progress;
    }

    void const* consume() noexcept
    {
        assert(m_progress <= this->size());

        return static_cast<std::uint8_t const*>(this->data()) + m_progress;
    }

    void const* consume(std::size_t size) noexcept
    {
        assert(m_progress + size <= this->size());

        void const* data = consume();
        m_progress += size;
        return data;
    }

    void consume(void* data, std::size_t size) noexcept
    {
        memcpy(data, consume(size), size);
    }

protected:
    Source() = default;
    ~Source() = default;

private:
    std::size_t m_progress{ 0 };
};

} // namespace hlib

