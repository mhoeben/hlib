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
#include "generator_typescript.hpp"
#include "hlib/format.hpp"
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
        { Type::Bool,           "boolean" },
        { Type::Int32,          "number" },
        { Type::Int64,          "number" },
        { Type::Float32,        "number" },
        { Type::Float64,        "number" },
        { Type::String,         "string" },
        { Type::Binary,         "Uint8Array" },
        { Type::BoolArray,      "Array<boolean>" },
        { Type::Int32Array,     "Array<number>" },
        { Type::Int64Array,     "Array<number>" },
        { Type::Float32Array,   "Array<number>" },
        { Type::Float64Array,   "Array<number>" },
        { Type::StringArray,    "Array<string>" },
        { Type::BinaryArray,    "Array<Uint8Array>" }
    };

    auto it = map.find(type);
    assert(map.end() != it);
    return map.end() != it ? it->second : "";
}

std::string to_extension(Type type)
{
    std::unordered_map<Type, std::string> map =
    {
        { Type::Bool,           "Bool" },
        { Type::Int32,          "Int" },
        { Type::Int64,          "Int" },
        { Type::Float32,        "Float32" },
        { Type::Float64,        "Float64" },
        { Type::String,         "String" },
        { Type::Binary,         "Binary" }
    };

    auto it = map.find(type);
    assert(map.end() != it);
    return map.end() != it ? it->second : "";
}

} // namespace

//
// Public
//
int GeneratorTypescript::generate(FILE* output, FILE* input, Side side)
{
    if (-1 == Generator::generate(output, input, side)) {
        return -1;
    }

    // Copyright and header.
    fmt::print(m_output, "//\n");
    for (std::string const& string : m_copyright) {
        fmt::print(m_output, string.empty() ? "//\n" : "// {}\n", string);
    }
    fmt::print(m_output,
        "//\n"
    );

    std::string ns;
    for (std::string const& string : m_namespace) {
        ns += string + '_';
    }
    ns.pop_back();

    fmt::print(m_output,
        "import {{ hlib_codec }} from \"hlib/codec.js\"\n"
        "\n"
        "export namespace {ns}\n"
        "{{\n"
        "\n",
        fmt::arg("ns", ns)
    );

    // Declarations
    std::vector<Declaration> declarations;
    if (-1 == parseDeclarations(declarations)) {
        return -1;
    }

    int id = m_base_id;

    // Identifiers.
    fmt::print(m_output,
        "export const FirstId_: number = {id};\n",
        fmt::arg("id", id)
    );
    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "export const {name}Id: number = {id};\n",
            fmt::arg("name", declaration.name),
            fmt::arg("id", id)
        );
        ++id;
    }
    fmt::print(m_output,
        "export const LastId_: number = {id};\n",
        fmt::arg("id", id)
    );

    id = m_base_id;

    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "\nexport class {name} implements hlib_codec.Type\n"
            "{{\n",
            fmt::arg("name", declaration.name)
        );
        {
            fmt::print(m_output,
                "    readonly __id: number = {id};\n"
                "    readonly __size: number = {size};\n"
                "\n",
                fmt::arg("id", id),
                fmt::arg("size", declaration.members.size())
            );

            for (Declaration::Member const& member : declaration.members) {
                fmt::print(m_output,
                    "    {name}: {type};\n",
                    fmt::arg("name", member.name),
                    fmt::arg("type", to_string(member.type))
                );
            }

            fmt::print(m_output,
                "\n"
                "    encode(encoder: hlib_codec.Encoder): void\n"
                "    {{\n"
                "        encoder.openType(null, this);\n"
            );
            {
                for (Declaration::Member const& member : declaration.members) {
                    if (true == is_vector(member.type)) {
                        Type const underlying_type = to_underlying_type(member.type);

                        fmt::print(m_output,
                            "        encoder.openArray(\"{name}\", this.{name}.length);\n"
                            "        for (const value of this.{name}) {{\n"
                            "            encoder.encode{ext}(null, value);\n"
                            "        }}\n"
                            "        encoder.close();\n",
                            fmt::arg("name", member.name),
                            fmt::arg("ext", to_extension(underlying_type))
                        );
                    }
                    else {
                        fmt::print(m_output,
                            "        encoder.encode{ext}(\"{name}\", this.{name});\n",
                            fmt::arg("ext", to_extension(member.type)),
                            fmt::arg("name", member.name)
                        );
                    }
                }
            }
            fmt::print(m_output,
                "        encoder.close();\n"
                "    }}\n"
            );

            fmt::print(m_output,
                "\n"
                "    decode(decoder: hlib_codec.Decoder): void\n"
                "    {{\n"
                "        decoder.openType(null);\n"
            );
            {
                for (Declaration::Member const& member : declaration.members) {
                    if (true == is_vector(member.type)) {
                        Type const underlying_type = to_underlying_type(member.type);

                        fmt::print(m_output,
                            "        this.{name}.length = encoder.openArray(\"{name}\");\n"
                            "        for (let i = 0; i < this.{name}.length; ++i) {{\n"
                            "            this.{name}[i](decoder.decode{ext}(null));\n"
                            "        }}\n"
                            "        decoder.close();\n",
                            fmt::arg("name", member.name),
                            fmt::arg("ext", to_extension(underlying_type))
                        );
                    }
                    else {
                        fmt::print(m_output,
                            "        this.{name} = decoder.decode{ext}(\"{name}\");\n",
                            fmt::arg("ext", to_extension(member.type)),
                            fmt::arg("name", member.name)
                        );
                    }
                }
            }
            fmt::print(m_output,
                "        decoder.close();\n"
                "    }}\n"
            );
        }
        fmt::print(m_output,
            "}};\n"
        );
    }

    fmt::print(m_output,
        "\n"
        "}} // namespace {ns}\n",
        fmt::arg("ns", ns)
    );

    return 0;
}

