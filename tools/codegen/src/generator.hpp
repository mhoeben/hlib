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

#include "types.hpp"
#include "hlib/json.hpp"
#include <memory>
#include <stdio.h>
#include <vector>
#include <unordered_map>

class Generator
{
    HLIB_NOT_COPYABLE(Generator);
    HLIB_NOT_MOVABLE(Generator);

public:
    static std::unique_ptr<Generator> create(Language language);

public:
    Generator() = default;
    virtual ~Generator() = default;

    virtual int generate(FILE* output, FILE* input, Side side);

protected:
    hlib::JSON m_input;
    FILE* m_output{ nullptr };
    Side m_side{ Side::Both };

    std::vector<std::string> m_copyright;
    std::string m_version;
    std::vector<std::string> m_namespace;
    int m_base_id{ 0 };

    struct Declaration
    {
        std::string name;
        Side orientation{ Side::Invalid };

        struct Member
        {
            std::string name;
            Type type;
            bool array;
        };
        std::vector<Member> members;
    };

    int parseDeclarations(std::vector<Declaration>& declarations);
};

