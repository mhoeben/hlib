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

template<typename T>
class Box final
{
    static_assert(true == std::is_trivial<T>::value, "T must be a trivial type");

    HLIB_NOT_COPYABLE(Box);
    HLIB_NOT_MOVABLE(Box);

public:
    typedef std::function<void(T&)> Destructor;

public:
    Box(T const& value, Destructor destructor)
        : m_value{ value }
        , m_destructor(std::move(destructor))
    {
    }

    ~Box()
    {
        m_destructor(m_value);
    }

    operator T const&() const noexcept
    {
        return m_value;
    }

    operator T&() noexcept
    {
        return m_value;
    }

    T const& operator*() const noexcept
    {
        return m_value;
    }

    T& operator*() noexcept
    {
        return m_value;
    }

    T const* operator->() const noexcept
    {
        return &m_value;
    }

    T* operator->() noexcept
    {
        return &m_value;
    }

    T const& get() const noexcept
    {
        return m_value;
    }

    T& get() noexcept
    {
        return m_value;
    }

private:
    T m_value;
    Destructor m_destructor;
};

template<typename T, T invalid_handle = 0>
class Handle final
{
    static_assert(true == std::is_trivial<T>::value, "T must be a trivial type");

    HLIB_NOT_COPYABLE(Handle);

public:
    typedef T Type;
    typedef std::function<void(T)> Destructor;

public:
    Handle(Destructor destructor) noexcept
        : m_destructor(std::move(destructor))
    {
        assert(nullptr != m_destructor);
    }

    Handle(T handle, Destructor destructor) noexcept
        : m_handle{ handle }
        , m_destructor(std::move(destructor))
    {
    }

    Handle(Handle&& that) noexcept
        : m_handle{ that.m_handle }
        , m_destructor(std::move(that.m_destructor))
    {
        that.m_handle = invalid_handle;
    }

    ~Handle()
    {
        reset();
    }

    Handle& operator=(Handle&& that) noexcept
    {
        reset();

        std::swap(m_handle, that.m_handle);
        m_destructor = std::move(that.m_destructor);
        return *this;
    }

    T const& operator *() const noexcept
    {
        return m_handle;
    }

    T& operator *() noexcept
    {
        return m_handle;
    }

    T const& operator ->() const noexcept
    {
        return m_handle;
    }

    T& operator ->() noexcept
    {
        return m_handle;
    }

    bool operator ==(Handle const& that) const noexcept
    {
        return m_handle == that.m_handle;
    }

    bool operator ==(T const& that) const noexcept
    {
        return m_handle == that;
    }

    bool operator !=(Handle const& that) const noexcept
    {
        return m_handle != that.m_handle;
    }

    bool operator !=(T const& that) const noexcept
    {
        return m_handle != that;
    }

    T const& get() const noexcept
    {
        return m_handle;
    }

    bool empty() const noexcept
    {
        return invalid_handle == m_handle;
    }

    T& get() noexcept
    {
        return m_handle;
    }

    T* reset() noexcept
    {
        reset(invalid_handle);
        return &m_handle;
    }

    void reset(T handle) noexcept
    {
        if (invalid_handle != m_handle) {
            m_destructor(m_handle);
        }
        m_handle = std::move(handle);
    }

    void swap(Handle& that) noexcept
    {
        std::swap(m_handle, that.m_handle);
        std::swap(m_destructor, that.m_destructor);
    }

    T release() noexcept
    {
        T handle = invalid_handle;
        std::swap(m_handle, handle);
        return handle;
    }

private:
    T m_handle{ invalid_handle };
    Destructor m_destructor;
};

} // namespace hlib

