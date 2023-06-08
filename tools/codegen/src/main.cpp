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
#include "generator.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "hlib/error.hpp"
#include "hlib/format.hpp"
#include "hlib/scope_guard.hpp"
#include <getopt.h>
#include <iostream>
#include <fstream>

using namespace hlib;

namespace
{

char const* const short_options = "hl:o:s:";

option const long_options[] =
{
    { "help",       no_argument,        nullptr, 'h' },
    { "language",   required_argument,  nullptr, 'l' },
    { "output",     required_argument,  nullptr, 'o' },
    { "side",       required_argument,  nullptr, 's' },
    {}
};

void usage()
{
    fmt::print(
        "SYNOPSIS\n"
        "     codegen OPTIONS <input-file>\n"
        "\n"
        "OPTIONS\n"
        "    -h, --help              this help\n"
        "    -l, --language <lang>   output for <lang> language\n"
        "    -o, --output <file>     output file\n"
        "    -s, --side <side>       output for <side> component\n"
        "\n"
        "WHERE\n"
        "   <lang> = cpp11*|c99|typescript\n"
        "   <side> = left*|right\n"
        "   * = default\n"
    );
}

} // namespace

int main(int argc, char* argv[])
{
    auto language = Language::CPP11;
    auto side = Side::Left;
    FILE* input = stdin;
    FILE* output = stdout;

    ScopeGuard cleanup([&] {
        if (nullptr != output && stdout != output) {
            fclose(output);
        }
        if (nullptr != input && stdin != input) {
            fclose(input);
        }
    });

    do {
        int long_option;
        int short_option = getopt_long(argc, argv, short_options, long_options, &long_option);
        if (-1 == short_option) {
            break;
        }

        switch (short_option) {
        case 'h':
            usage();
            return EXIT_SUCCESS;

        case 'l':
            language = to_language(optarg);
            if (Language::Invalid == language) {
                fmt::print(stderr, "codegen: invalid argument for -l/--language '{}'", optarg);
                return EXIT_FAILURE;
            }
            break;

        case 'o':
            output_filepath = optarg;
            output = fopen(optarg, "w");
            if (nullptr == output) {
                fmt::print(stderr, "codegen: failed to open '{}' for writing ({})", optarg, get_error_string(errno));
                return EXIT_FAILURE;
            }
            break;

        case 's':
            side = to_side(optarg);
            if (Side::Invalid == side) {
                fmt::print(stderr, "codegen: invalid argument for -s/--side '{}'", optarg);
                return EXIT_FAILURE;
            }
            break;

        default:
            return EXIT_FAILURE;
        }
    }
    while (true);

    argc -= optind;
    argv += optind;

    if (argc > 1) {
        fmt::print(stderr, "codegen: ignoring extraneous arguments...\n");
    }
    else if (argc == 1) {
        input_filepath = argv[0];
        input = fopen(argv[0], "r");
        if (nullptr == input) {
            fmt::print(stderr, "codegen: failed to open '{}' for reading ({})", argv[0], get_error_string(errno));
            return EXIT_FAILURE;
        }
    }

    if (-1 == Generator::create(language)->generate(output, input, side)) { 
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

