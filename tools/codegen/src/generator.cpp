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
#include "config.hpp"
#include "generator.hpp"
#include "generator_c99.hpp"
#include "generator_cpp11.hpp"
#include "generator_typescript.hpp"
#include "utility.hpp"
#include "hlib/file.hpp"
#include "hlib/format.hpp"

using namespace hlib;

//
// Implementation
//
int Generator::parseDeclarations(std::vector<Declaration>& declarations)
{
    JSON const decls = m_input.at("declarations");
    if (JSON::Array != decls.type()) {
        return error_at(decls, "'declarations' need to be declared as an array");
    }
    
    for (JSON const& decl : decls) {
        Declaration declaration;

        // Get the declaration's name.
        declaration.name = decl.at("name").value();
        if (true == declaration.name.empty()) {
            return error_at(decl, "No 'name' attribute specified for declaration");
        }

        // Get the declaration's flow.
        std::string flow = decl.at("flow").value();
        if (true == flow.empty()) {
            return error_at(decl, "No 'flow' attribute specified for declaration");
        }
        declaration.flow = to_flow(flow);
        if (Flow::Invalid == declaration.flow) {
            return error_at(decl.at("flow"), "Invalid flow attribute value '{}'", flow);
        }

        // Get and iterate the declaration's members.
        JSON const mbrs = decl.at("members");

        for (JSON const& mbr : mbrs) {
            Declaration::Member member;

            // Get the member's name.
            member.name = mbr.at("name").value();
            if (true == member.name.empty()) {
                return error_at(mbr, "No 'name' attribute specified for member");
            }

            // Get the member's type.
            std::string type = mbr.at("type").value();
            if (true == type.empty()) {
                return error_at(mbr, "No 'type' attribute specified for member");
            }
            member.type = to_type(type);
            if (Type::Invalid == member.type) {
                return error_at(mbr.at("type"), "Invalid type attribute value '{}'", type);
            }

            declaration.members.emplace_back(std::move(member));
        }

        declarations.emplace_back(std::move(declaration));
    }

    return 0;
}

//
// Public
//
std::unique_ptr<Generator> Generator::create(Language language)
{
    std::unique_ptr<Generator> generator;

    switch (language) {
    case Language::CPP11:
        generator.reset(new GeneratorCPP11);
        break;

    case Language::C99:
        generator.reset(new GeneratorC99);
        break;

    case Language::Typescript:
        generator.reset(new GeneratorTypescript);
        break;

    default:
        assert(false);
        break;
    }

    return generator;
}

int Generator::generate(FILE* output, FILE* input, Side side)
{
    try {
        m_input.parse(to_string(file::read(input, ::Config::inputProgressiveReadSize())));
    }
    catch (std::exception const& e) {
        fmt::print(stderr, "codegen: failed to read or parse input: {}\n", e.what());
        return -1;
    }

    m_output = output;
    m_side = side;

    // Get copyright.
    JSON copyright = m_input.at("copyright");
    for (JSON const& line : copyright) {
        m_copyright.push_back(line.value());
    }

    // Get version.
    m_version = m_input.at("version").value();

    // Get namespace.
    JSON namespaces = m_input.at("namespace");
    for (JSON const& ns : namespaces) {
        m_namespace.push_back(ns.value());
    }

    // Get optional base id.
    if (true == m_input.contains("base_id")) {
        m_base_id = json_to<int>(m_input.at("base_id"));
    }

    JSON declarations = m_input.at("declarations");
    switch (declarations.type()) {
    case JSON::Array:
        break;

    case JSON::Undefined:
        fmt::print(stderr, "codegen: no declarations specified\n");
        return -1;

    default:
        fmt::print(stderr, "codegen: declarations is not of type array\n");
        return -1;
    }

    return 0;
}

