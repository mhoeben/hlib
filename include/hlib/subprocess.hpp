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

#include "hlib/buffer.hpp"
#include <string>
#include <vector>

namespace hlib
{

class Subprocess final
{
    HLIB_NOT_COPYABLE(Subprocess);

public:
    Subprocess() = default;
    Subprocess(Subprocess&& that) noexcept;
    Subprocess(std::string const& command, std::vector<std::string> const& args);

    Subprocess& operator =(Subprocess&& that) noexcept;

    int returnCode() const;
    Buffer const& output() const;
    Buffer const& error() const;

    void run(std::string const& command, std::vector<std::string> const& args);

private:
    int m_return_code{ -1 };
    Buffer m_output;
    Buffer m_error;
};

} // namespace hlib
