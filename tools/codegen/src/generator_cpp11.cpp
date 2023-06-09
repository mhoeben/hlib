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
#include "generator_cpp11.hpp"
#include "utility.hpp"
#include <unordered_map>

using namespace hlib;

//
// Implementation
//
namespace
{

std::string to_string(Type type)
{
    std::unordered_map<Type, std::string> map =
    {
        { Type::Bool,           "bool" },
        { Type::Int32,          "std::int32_t" },
        { Type::Int64,          "std::int64_t" },
        { Type::Float32,        "float" },
        { Type::Float64,        "double" },
        { Type::String,         "std::string" },
        { Type::Blob,           "hlib::Blob" },
        { Type::BoolArray,      "std::vector<bool>" },
        { Type::Int32Array,     "std::vector<std::int32_t>" },
        { Type::Int64Array,     "std::vector<std::int64_t>" },
        { Type::Float32Array,   "std::vector<float>" },
        { Type::Float64Array,   "std::vector<double>" },
        { Type::StringArray,    "std::vector<std::string>" },
        { Type::BlobArray,      "std::vector<hlib::Blob>" }
    };

    auto it = map.find(type);
    assert(map.end() != it);
    return map.end() != it ? it->second : "";
}

} // namespace

//
// Public
//
int GeneratorCPP11::generate(FILE* output, FILE* input, Side side)
{
    if (-1 == Generator::generate(output, input, side)) {
        return -1;
    }

    // Copyright and header.
    fmt::print(m_output, "//\n");
    for (std::string const& string : m_copyright) {
        fmt::print(m_output, string.empty() ? "//\n" : "// {}\n", string);
    }
    fmt::print(m_output, "//\n");
    fmt::print(m_output,
        "#pragma once\n"
        "\n"
        "#include \"hlib/codec.hpp\"\n"
        "#include <vector>\n"
        "\n"
    );

    for (std::string const& string : m_namespace ) {
        fmt::print(m_output,
            "namespace {}\n"
            "{{\n",
            string
        );
    }
    fmt::print(m_output, "\n");

    // Declarations.
    std::vector<Declaration> declarations;
    if (-1 == parseDeclarations(declarations)) {
        return -1;
    }

    int id = m_base_id;

    // Identifiers.
    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "static constexpr int {name}Id{{ {id} }};\n",
            fmt::arg("name", declaration.name),
            fmt::arg("id", id)
        );
        ++id;
    }

    id = m_base_id;

    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "\nstruct {name} : hlib::codec::Type\n"
            "{{\n",
            fmt::arg("name", declaration.name)
        );
        {
            // Member declarations.
            for (Declaration::Member const& member : declaration.members) {
                fmt::print(m_output, "    {} {};\n", to_string(member.type), member.name);
            }

            // Constructor and id operator.
            fmt::print(m_output,
                "\n"
                "    {name}() = default;\n"
                "\n"
                "    explicit operator hlib::codec::Type::Id() const override\n"
                "    {{\n"
                "        return {id};\n"
                "    }}\n"
                "\n"
                "    explicit operator std::size_t() const override\n"
                "    {{\n"
                "        return {size};\n"
                "    }}\n",
                fmt::arg("name", declaration.name),
                fmt::arg("id", id),
                fmt::arg("size", declaration.members.size())
            );

            // Encoder member function.
            fmt::print(m_output,
                "\n"
                "    void operator()(hlib::codec::Encoder& encoder) const override\n"
                "    {{\n"
                "        encoder.open(nullptr, *this);\n",
                fmt::arg("name", declaration.name)
            );

            for (Declaration::Member const& member : declaration.members) {
                fmt::print(m_output,
                    "        encoder.encode(\"{name}\", this->{name});\n",
                    fmt::arg("name", member.name)
                );
            }

            fmt::print(m_output,
                "        encoder.close();\n"
                "    }}\n"
            );

            // Decoder member function.
            fmt::print(m_output,
                "\n"
                "    void operator()(hlib::codec::Decoder& decoder) override\n"
                "    {{\n"
                "        decoder.open(nullptr, *this);\n",
                fmt::arg("name", declaration.name)
            );

            for (Declaration::Member const& member : declaration.members) {
                fmt::print(m_output,
                    "        decoder.decode(\"{name}\", this->{name});\n",
                    fmt::arg("name", member.name)
                );
            }

            fmt::print(m_output,
                "        decoder.close();\n"
                "    }}\n"
            );
        }
        fmt::print(m_output, "}};\n");

        ++id;
    }

    if (true != m_namespace.empty()) {
        // Trailer.
        fmt::print(m_output, "\n");
        for (std::string const& string : m_namespace ) {
            fmt::print(m_output, "}} // namespace {}\n", string);
        }
    }

    return 0;
}

