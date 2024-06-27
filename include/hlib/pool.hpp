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
#include <cassert>
#include <deque>
#include <functional>

namespace hlib
{

template<typename T>
class Pool final
{
    static_assert(true == std::is_move_constructible<T>::value, "T not move constructible");

    HLIB_NOT_COPYABLE(Pool);

public:
    typedef std::function<T()> Factory;

public:
    Pool(Factory factory, std::size_t maximum = 0, std::size_t initial = 0)
        : m_factory(std::move(factory))
        , m_maximum{ maximum }
        , m_count{ 0 }
    {
        for (std::size_t i = 0; i < initial; ++i) {
            add(m_factory());
        }
    }

    void add(T&& element)
    {
        if (0 != m_maximum && m_count >= m_maximum) {
            throw std::overflow_error("Maximum pool capacity reached");
        }

        m_pool.emplace_back(std::move(element));
    }

    T get()
    {
        if (true == m_pool.empty()) {
            add(m_factory());
        }

        T element = std::move(m_pool.front());
                              m_pool.pop_front();
        return element;
    }

    void put(T&& element)
    {
        m_pool.emplace_back(std::move(element));
        assert(0 == m_maximum || m_pool.size() <= m_maximum);
    }

private:
    Factory const m_factory;
    std::size_t const m_maximum;
    std::size_t m_count;
    std::deque<T> m_pool;
};

} // namespace hlib

