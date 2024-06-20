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
#include "hlib/type_traits.hpp"
#include <cstring>
#include <limits>
#include <memory>

namespace hlib
{

class Receiver
{
public:
    static constexpr std::size_t UnspecifiedCapacity = 0;
    static constexpr std::size_t InfiniteCapacity = std::numeric_limits<std::size_t>::max();

public:
    virtual std::size_t size() const noexcept = 0;
    virtual void* resize(std::size_t size) noexcept = 0;

    bool full() const noexcept;
    bool overflow() const noexcept;

    std::size_t headroom() const noexcept;
    std::size_t headroom(std::size_t limit) const noexcept;

    void* accept(std::size_t size) noexcept;

protected:
    Receiver() = default;
    Receiver(std::size_t maximum);

    ~Receiver() = default;

private:
    std::size_t const m_maximum{ InfiniteCapacity };
};

template<typename T>
class ReceiverAdapter final : public Receiver
{
    HLIB_NOT_COPYABLE(ReceiverAdapter);

public:
    typedef T Type;

public:
    ReceiverAdapter() noexcept = default;

    ReceiverAdapter(std::size_t maximum, T data = T()) noexcept
        : Receiver(maximum)
        , m_data(std::move(data))
    {
        if (Receiver::InfiniteCapacity != maximum) {
            if constexpr(true == is_unique_ptr<T>::value || true == is_shared_ptr<T>::value) {
                static_assert(true == has_reserve_method<typename T::element_type>::value);
                m_data->reserve(maximum);
            }
            else {
                static_assert(true == has_reserve_method<T>::value);
                m_data.reserve(maximum);
            }
        }
    }

    ReceiverAdapter(ReceiverAdapter&& that) noexcept
        : m_data(std::move(that.m_data))
    {
    }

    ReceiverAdapter& operator=(ReceiverAdapter&& that) noexcept
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

    std::size_t size() const noexcept
    {
        if constexpr(true == is_unique_ptr<T>::value || true == is_shared_ptr<T>::value) {
            static_assert(true == has_size_method<typename T::element_type>::value);
            return m_data->size();
        }
        else {
            static_assert(true == has_size_method<T>::value);
            return m_data.size();
        }
    }

    void* resize(std::size_t size) noexcept
    {
        if constexpr(true == is_unique_ptr<T>::value || true == is_shared_ptr<T>::value) {
            static_assert(true == has_resize_method<typename T::element_type>::value);
            m_data->resize(size);
            return m_data->data();
        }
        else {
            static_assert(true == has_resize_method<T>::value);
            m_data.resize(size);
            return m_data.data();
        }
    }
};

template<typename T>
ReceiverAdapter<T> make_receiver(std::size_t maximum, T data = T())
{
    return ReceiverAdapter<T>(maximum, std::move(data));
}

template<typename T>
std::shared_ptr<ReceiverAdapter<T>> make_shared_receiver(std::size_t maximum, T data = T())
{
    return std::make_shared<ReceiverAdapter<T>>(maximum, std::move(data));
}

template<typename T>
T const& get(Receiver const& receiver)
{
#ifdef HLIB_RTTI_ENABLED
    ReceiverAdapter<T> const* adapter = dynamic_cast<ReceiverAdapter<T> const*>(&receiver);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return static_cast<ReceiverAdapter<T> const&>(receiver).get();
#endif
}

template<typename T>
T& get(Receiver& receiver)
{
#ifdef HLIB_RTTI_ENABLED
    ReceiverAdapter<T>* adapter = dynamic_cast<ReceiverAdapter<T>*>(&receiver);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return static_cast<ReceiverAdapter<T>&>(receiver).get();
#endif
}

template<typename T>
T const& get(std::shared_ptr<Receiver const> const& receiver)
{
#ifdef HLIB_RTTI_ENABLED
    std::shared_ptr<ReceiverAdapter<T>> adapter = std::dynamic_pointer_cast<ReceiverAdapter<T>>(receiver);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return std::dynamic_pointer_cast<ReceiverAdapter<T>>(receiver)->get();
#endif
}

template<typename T>
T& get(std::shared_ptr<Receiver> const& receiver)
{
#ifdef HLIB_RTTI_ENABLED
    std::shared_ptr<ReceiverAdapter<T>> adapter = std::dynamic_pointer_cast<ReceiverAdapter<T>>(receiver);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return std::dynamic_pointer_cast<ReceiverAdapter<T>>(receiver)->get();
#endif
}

} // namespace hlib
 
