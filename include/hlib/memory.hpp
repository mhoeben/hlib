//
// MIT License
//
// Copyright (c) 2023 Maarten Hoeben
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
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

namespace hlib
{

template<typename T, typename Lambda>
bool with_weak_ptr_locked(std::weak_ptr<T> const& weak_ptr, Lambda lambda)
{
    auto locked = weak_ptr.lock();
    if (nullptr == locked) {
        return false;
    }

    lambda(*locked);
    return true;
}

template<typename T, T invalid_value = 0, typename Destructor=void(T)>
class UniqueOwner final
{
    static_assert(true == std::is_trivial<T>::value, "T must be a trivial type");

    HLIB_NOT_COPYABLE(UniqueOwner);

public:
    typedef T Type;

public:
    UniqueOwner(Destructor destructor) noexcept
        : m_destructor(std::move(destructor))
    {
        assert(nullptr != m_destructor);
    }

    UniqueOwner(T value, Destructor destructor) noexcept
        : m_value{ value }
        , m_destructor(std::move(destructor))
    {
    }

    UniqueOwner(UniqueOwner&& that) noexcept
        : m_value{ that.m_value }
        , m_destructor(std::move(that.m_destructor))
    {
        that.m_value = invalid_value;
    }

    ~UniqueOwner()
    {
        reset();
    }

    UniqueOwner& operator=(UniqueOwner&& that) noexcept
    {
        reset();

        std::swap(m_value, that.m_value);
        m_destructor = std::move(that.m_destructor);
        return *this;
    }

    T const& operator *() const noexcept
    {
        return m_value;
    }

    T const& operator ->() const noexcept
    {
        return m_value;
    }

    T const& value() const noexcept
    {
        return m_value;
    }

    explicit operator bool() const noexcept
    {
        return invalid_value != m_value;
    }

    T* reset() noexcept
    {
        reset(invalid_value);
        return &m_value;
    }

    void reset(T value) noexcept
    {
        if (invalid_value != m_value) {
            m_destructor(m_value);
        }
        m_value = value;
    }

    void swap(UniqueOwner& that) noexcept
    {
        std::swap(m_value, that.m_value);
        std::swap(m_destructor, that.m_destructor);
    }

    T release() noexcept
    {
        T value = invalid_value;
        std::swap(m_value, value);
        return value;
    }

private:
    T m_value{ invalid_value };
    std::function<Destructor> m_destructor;
};

} // namespace hlib

