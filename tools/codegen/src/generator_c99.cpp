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
#include "generator_c99.hpp"
#include "utility.hpp"
#include "hlib/string.hpp"
#include "hlib/uuid.hpp"
#include "hlib/c/codec.h"

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
        { Type::Bool,           "_Bool" },
        { Type::Int32,          "int32_t" },
        { Type::Int64,          "int64_t" },
        { Type::Float32,        "float" },
        { Type::Float64,        "double" },
        { Type::String,         "hlib_codec_string_t" },
        { Type::Binary,         "hlib_codec_binary_t" },
        { Type::BoolArray,      "hlib_vector_t" },
        { Type::Int32Array,     "hlib_vector_t" },
        { Type::Int64Array,     "hlib_vector_t" },
        { Type::Float32Array,   "hlib_vector_t" },
        { Type::Float64Array,   "hlib_vector_t" },
        { Type::StringArray,    "hlib_vector_t" },
        { Type::BinaryArray,    "hlib_vector_t" }
    };

    auto it = map.find(type);
    assert(map.end() != it);
    return map.end() != it ? it->second : "";
}

std::string to_extension(Type type)
{
    std::unordered_map<Type, std::string> map =
    {
        { Type::Bool,           "bool" },
        { Type::Int32,          "int32" },
        { Type::Int64,          "int64" },
        { Type::Float32,        "float" },
        { Type::Float64,        "double" },
        { Type::String,         "string" },
        { Type::Binary,         "binary" }
    };

    auto it = map.find(type);
    assert(map.end() != it);
    return map.end() != it ? it->second : "";
}

} // namespace

//
// Public
//
int GeneratorC99::generate(FILE* output, FILE* input, Side side)
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

    std::string ns;
    for (std::string const& string : m_namespace) {
        ns += string + '_';
    }

    fmt::print(m_output,
        "#ifdef __cplusplus\n"
        "extern \"C\"\n"
        "{{\n"
        "#endif\n"
        "\n"
        "#ifndef {NS}H\n"
        "#define {NS}H\n"
        "\n"
        "#include \"hlib/c/codec.h\"\n"
        "#include \"hlib/c/vector.h\"\n"
        "\n",
        fmt::arg("NS", to_upper(ns))
    );

    // Declarations
    std::vector<Declaration> declarations;
    if (-1 == parseDeclarations(declarations)) {
        return -1;
    }

    // Type IDs.
    int id = m_base_id;

    fmt::print(m_output,
        "#define {NS}FIRST_ID_ ({id})\n",
            fmt::arg("NS", to_upper(ns)),
            fmt::arg("id", id)
    );
    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "#define {NS}{NAME}_ID ({id})\n",
            fmt::arg("NS", to_upper(ns)),
            fmt::arg("NAME", to_upper(declaration.name)),
            fmt::arg("id", id)
        );
        ++id;
    }
    fmt::print(m_output,
        "#define {NS}LAST_ID_ ({id})\n",
            fmt::arg("NS", to_upper(ns)),
            fmt::arg("id", id - 1)
    );

    for (Declaration const& declaration : declarations) {
        fmt::print(m_output,
            "\nstruct {ns}{name}\n"
            "{{\n"
            "    hlib_codec_type_id_t id_;\n"
            "    hlib_codec_type_size_t size_;\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        {
            if (false == declaration.members.empty()) {
                fmt::print(m_output, "\n");
            }

            // Member declarations.
            for (Declaration::Member const& member : declaration.members) {
                fmt::print(m_output,
                    "    {} {};\n",
                    to_string(member.type),
                    member.name
                );
            }
        }
        fmt::print(m_output,
            "}};\n"
            "\n"
        );

        fmt::print(m_output,
            "void {ns}{name}_init(struct {ns}{name}* object);\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        fmt::print(m_output,
            "void {ns}{name}_free(struct {ns}{name}* object);\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        fmt::print(m_output,
            "void {ns}{name}_encode(hlib_codec_encoder_t* encoder, struct {ns}{name}* const object);\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        fmt::print(m_output,
            "void {ns}{name}_decode(hlib_codec_decoder_t* decoder, struct {ns}{name}* object);\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
    }

    fmt::print(m_output,
        "\n"
        "#endif // {NS}H\n"
        "\n"
        "#ifdef {NS}IMPL\n",
        fmt::arg("NS", to_upper(ns))
    );

    for (Declaration const& declaration : declarations) {
        // Init function.
        fmt::print(m_output,
            "\nvoid {ns}{name}_init(struct {ns}{name}* self)\n"
            "{{\n"
            "    self->id_ = {NS}{NAME}_ID;\n"
            "    self->size_ = {size};\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name),
            fmt::arg("NS", to_upper(ns)),
            fmt::arg("NAME", to_upper(declaration.name)),
            fmt::arg("size", declaration.members.size())
        );
        {
            if (false == declaration.members.empty()) {
                fmt::print(m_output, "\n");
            }

            for (Declaration::Member const& member : declaration.members) {
                if (true == is_vector(member.type)) {
                    fmt::print(m_output, "    hlib_vector_init(&self->{}, sizeof({}));\n", member.name, to_string(to_underlying_type(member.type)));
                }
            }
        }
        fmt::print(m_output, "}}\n\n");

        // Free function.
        fmt::print(m_output,
            "void {ns}{name}_free(struct {ns}{name}* self)\n"
            "{{\n"
            "    (void)self;\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        {
            for (Declaration::Member const& member : declaration.members) {
                if (true == is_vector(member.type)) {
                    fmt::print(m_output, "    hlib_vector_free(&self->{});\n", member.name);
                }
            }
        }
        fmt::print(m_output, "}}\n\n");

        // Encoder function.
        fmt::print(m_output,
            "void {ns}{name}_encode(hlib_codec_encoder_t* encoder, struct {ns}{name}* const self)\n"
            "{{\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        {
            fmt::print(m_output,
                "    encoder->open_type(encoder, NULL, (hlib_codec_type_t const*)self);\n",
                fmt::arg("name", declaration.name),
                fmt::arg("size", declaration.members.size())
            );
            for (Declaration::Member const& member : declaration.members) {
                Type const underlying_type = to_underlying_type(member.type);

                if (true == is_vector(member.type)) {
                    fmt::print(m_output,
                        "    {{\n"
                        "        size_t array_size = hlib_vector_size(&self->{name});\n"
                        "        encoder->open_array(encoder, \"{name}\", array_size);\n"
                        "        {{\n"
                        "            {type}* array = ({type}*)hlib_vector_data(&self->{name});\n"
                        "            for (size_t i = 0; i < array_size; ++i) {{\n"
                        "                encoder->encode_{ext}(encoder, \"{name}\", {deref}array[i]);\n"
                        "            }}\n"
                        "        }}\n"
                        "        encoder->close(encoder);\n"
                        "    }}\n",
                        fmt::arg("type", to_string(underlying_type)),
                        fmt::arg("ext", to_extension(underlying_type)),
                        fmt::arg("name", member.name),
                        fmt::arg("deref", is_pointer(underlying_type) ? "&":"")
                    );
                }
                else {
                    fmt::print(m_output,
                        "    encoder->encode_{ext}(encoder, \"{name}\", {deref}self->{name});\n",
                        fmt::arg("ext", to_extension(underlying_type)),
                        fmt::arg("name", member.name),
                        fmt::arg("deref", is_pointer(underlying_type) ? "&":"")
                    );
                }
            }
            fmt::print(m_output, "    encoder->close(encoder);\n");
        }
        fmt::print(m_output, "}}\n\n");

        // Decoder function.
        fmt::print(m_output,
            "void {ns}{name}_decode(hlib_codec_decoder_t* decoder, struct {ns}{name}* self)\n"
            "{{\n"
            "    size_t array_size;\n"
            "    (void)array_size;\n",
            fmt::arg("ns", ns),
            fmt::arg("name", declaration.name)
        );
        {
            fmt::print(m_output,
                "    decoder->open_type(decoder, NULL, (hlib_codec_type_t*)self);\n",
                fmt::arg("name", declaration.name),
                fmt::arg("size", declaration.members.size())
            );
            for (Declaration::Member const& member : declaration.members) {
                Type const underlying_type = to_underlying_type(member.type);

                if (true == is_vector(member.type)) {
                    fmt::print(m_output,
                        "    decoder->open_array(decoder, \"{name}\", &array_size);\n"
                        "    {{\n"
                        "        {type}* array = ({type}*)hlib_vector_resize(&self->{name}, array_size);\n"
                        "        if (NULL == array) {{\n"
                        "            decoder->error = HLIB_ERROR_BAD_ALLOC;\n"
                        "            return;\n"
                        "        }}\n"
                        "        for (size_t i = 0; i < array_size; ++i) {{\n"
                        "            decoder->decode_{ext}(decoder, \"{name}\", &array[i]);\n"
                        "        }}\n"
                        "    }}\n"
                        "    decoder->close(decoder);\n",
                        fmt::arg("type", to_string(underlying_type)),
                        fmt::arg("ext", to_extension(underlying_type)),
                        fmt::arg("name", member.name)
                    );
                }
                else {
                    fmt::print(m_output,
                        "    decoder->decode_{ext}(decoder, \"{name}\", &self->{name});\n",
                        fmt::arg("ext", to_extension(underlying_type)),
                        fmt::arg("name", member.name)
                    );
                }
            }
            fmt::print(m_output, "    decoder->close(decoder);\n");
        }
        fmt::print(m_output, "}}\n\n");
    }

    fmt::print(m_output,
        "\n"
        "#endif // {NS}IMPL\n"
        "\n"
        "#ifdef __cplusplus\n"
        "}}\n"
        "#endif\n",
        fmt::arg("NS", to_upper(ns))
    );
    return 0;
}

