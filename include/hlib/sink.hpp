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
#include <cstring>
#include <limits>
#include <memory>

namespace hlib
{

class Sink
{
public:
    static constexpr std::size_t Unspecified = 0;
    static constexpr std::size_t Infinite = std::numeric_limits<std::size_t>::max();

public:
    virtual std::size_t size() const noexcept = 0;
    virtual void* resize(std::size_t size) noexcept = 0;

    bool full() const noexcept;
    std::size_t headroom() const noexcept;
    std::size_t headroom(std::size_t limit) const noexcept;

    void* produce(std::size_t size) noexcept;
    std::size_t produce(void const* data, std::size_t size) noexcept;

protected:
    Sink() = default;
    Sink(std::size_t maximum);

    ~Sink() = default;

private:
    std::size_t m_maximum{ std::numeric_limits<std::size_t>::max() };
};

template<typename T,
         typename = std::enable_if_t<true == has_size_method<T>::value
                                  && true == has_resize_method<T>::value>>
class SinkAdapter final : public Sink
{
    HLIB_NOT_COPYABLE(SinkAdapter);

public:
    SinkAdapter() noexcept = default;

    SinkAdapter(std::size_t maximum) noexcept
        : Sink(maximum)
    {
        if (Sink::Infinite != maximum) {
            m_data.reserve(maximum);
        }
    }

    SinkAdapter(SinkAdapter&& that) noexcept
        : m_data(std::move(that.m_data))
    {
    }

    SinkAdapter& operator=(SinkAdapter&& that) noexcept
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
        return m_data.size();
    }

    void* resize(std::size_t size) noexcept
    {
        m_data.resize(size);
        return m_data.data();
    }
};

template<typename T>
SinkAdapter<T> make_sink(std::size_t maximum = 0)
{
    return SinkAdapter<T>(maximum);
}

template<typename T>
std::shared_ptr<SinkAdapter<T>> make_shared_sink(std::size_t maximum = 0)
{
    return std::make_shared<SinkAdapter<T>>(maximum);
}

template<typename T>
T const& get(Sink const& sink)
{
#ifdef HLIB_RTTI_ENABLED
    SinkAdapter<T> const* adapter = dynamic_cast<SinkAdapter<T> const*>(&sink);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return static_cast<SinkAdapter<T> const&>(sink).get();
#endif
}

template<typename T>
T& get(Sink& sink)
{
#ifdef HLIB_RTTI_ENABLED
    SinkAdapter<T>* adapter = dynamic_cast<SinkAdapter<T>*>(&sink);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return static_cast<SinkAdapter<T>&>(sink).get();
#endif
}

template<typename T>
T const& get(std::shared_ptr<Sink> const& sink)
{
#ifdef HLIB_RTTI_ENABLED
    std::shared_ptr<SinkAdapter<T>> adapter = std::dynamic_pointer_cast<SinkAdapter<T>>(sink);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return std::dynamic_pointer_cast<SinkAdapter<T>>(sink)->get();
#endif
}

template<typename T>
T& get(std::shared_ptr<Sink>& sink)
{
#ifdef HLIB_RTTI_ENABLED
    std::shared_ptr<SinkAdapter<T>> adapter = std::dynamic_pointer_cast<SinkAdapter<T>>(sink);
    assert(nullptr != adapter);
    return adapter->get();
#else
    return std::dynamic_pointer_cast<SinkAdapter<T>>(sink)->get();
#endif
}

} // namespace hlib
 
