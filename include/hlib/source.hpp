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
#include "hlib/type_traits.hpp"
#include <memory>

namespace hlib
{

class Source
{
public:
    virtual std::size_t size() const noexcept = 0;
    virtual void const* data() const noexcept = 0;

    std::size_t available() const noexcept;
    bool empty() const noexcept;

    void const* consume() noexcept;
    void const* consume(std::size_t size) noexcept;
    void consume(void* data, std::size_t size) noexcept;

protected:
    Source() = default;
    ~Source() = default;

private:
    std::size_t m_progress{ 0 };
};

template<typename T>
class SourceAdapter final : public Source
{
    HLIB_NOT_COPYABLE(SourceAdapter);

public:
    typedef T Source;

public:
    SourceAdapter(T data) noexcept
        : m_data(std::move(data))
    {
    }

    SourceAdapter(SourceAdapter&& that) noexcept
        : m_data(std::move(that.m_data))
    {
    }

    SourceAdapter& operator=(SourceAdapter&& that) noexcept
    {
        m_data = std::move(that.m_data);
        return *this;
    }

    T const& get() const noexcept
    {
        return m_data;
    }

    T& get() noexcept
    {
        return m_data;
    }

private:
    T m_data;

    std::size_t size() const noexcept override
    {
        if constexpr (true == is_unique_ptr<T>::value || true == is_shared_ptr<T>::value) {
            static_assert(true == has_size_method<typename T::element_type>::value);
            return m_data->size();
        }
        else {
            static_assert(true == has_size_method<T>::value);
            return m_data.size();
        }
    }

    void const* data() const noexcept override
    {
        if constexpr (true == is_unique_ptr<T>::value || true == is_shared_ptr<T>::value) {
            static_assert(true == has_data_method<typename T::element_type>::value);
            return m_data->data();
        }
        else {
            static_assert(true == has_data_method<T>::value);
            return m_data.data();
        }
    }
};

template<typename T>
SourceAdapter<T> make_source(T data)
{
    return SourceAdapter<T>(std::move(data));
}

template<typename T>
std::shared_ptr<SourceAdapter<T>> make_shared_source(T data)
{
    return std::make_shared<SourceAdapter<T>>(std::move(data));
}

} // namespace hlib

