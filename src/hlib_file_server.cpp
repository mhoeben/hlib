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
#include "hlib/file_server.hpp"
#include "hlib/config.hpp"
#include "hlib/file.hpp"
#include <filesystem>

using namespace hlib;
using namespace file::server;

//
// Implementation (GetFile)
//
void GetFile::onSend(http::Server::Transaction& transaction, std::shared_ptr<Buffer const> buffer, std::size_t more)
{
    using namespace std::placeholders;

    // Complete file sent?
    if (0 == more) {
        return;
    }

    auto content = std::const_pointer_cast<Buffer>(buffer);

    // Clear content buffer.
    content->clear();

    // Read (more) content.
    file::read(m_stream, *content, std::min(content->capacity(), more));

    // Send next chunk of content.
    transaction.send(
        std::move(content),
        std::bind(&GetFile::onSend, this, _1, _2, _3)
    );
}

//
// Public (GetFile)
//
GetFile::GetFile(http::Server::Transaction& transaction, std::filesystem::path filepath)
{
    bool const flush = transaction.request_content_length > 0;

    if (false == std::filesystem::exists(filepath)) {
        transaction.respond(flush, http::StatusCode::NotFound, {});
        return;
    }
    if (false == std::filesystem::is_regular_file(filepath)
     || false == file::is_readable(filepath)) {
        transaction.respond(flush, http::StatusCode::Forbidden, {});
        return;
    }

    // Create a respond object, capturing the filepath.
    auto respond =
        [this, &transaction, path = std::move(filepath)]
    {
        std::size_t const content_length = std::filesystem::file_size(path);

        // Respond with content-length.
        transaction.respond(
            http::StatusCode::Ok, "",
            {
                { "Content-Type", file::get_mime_type_from_file(path, ::Config::httpServerDefaultMimeType()) }
            },
            content_length
        );

        // No content or head request?
        if (0 == content_length || http::Method::Head == transaction.request_method) {
            return;
        }

        // Open file as an input stream.
        m_stream.open(path, std::ios::binary);
        assert(true == m_stream.is_open());

        // Throw exceptions on failures.
        m_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // Progressively send content.
        onSend(
            transaction, 
            std::make_shared<Buffer>(hlib::Config::httpServerContentChunkSize()),
            content_length
        );
    };

    if (true == flush) {
        assert(transaction.request_content_length > 0);

        // Flush and then respond to request.
        transaction.flush(
            [respond = std::move(respond)]
            (http::Server::Transaction& /* transaction */)
            {
                // Respond to request.
                respond();
            }
        );
        return;
    }

    // Respond to request.
    respond();
}

//
// Implementation (PutFile)
//
void PutFile::onReceive(http::Server::Transaction& transaction, std::shared_ptr<Buffer> buffer, std::size_t more)
{
    using namespace std::placeholders;

    // Write (more) content.
    file::write(m_stream, *buffer);

    // Complete file received?
    if (0 == more) {
        // Respond with ok.
        return transaction.respond(false, http::StatusCode::Ok, {});
    }

    // Clear content buffer.
    buffer->clear();

    // Receive next chunk of content.
    transaction.receive(
        std::move(buffer),
        std::bind(&PutFile::onReceive, this, _1, _2, _3)
    );
}

//
// Public (PutFile)
//
PutFile::PutFile(http::Server::Transaction& transaction, std::filesystem::path filepath)
{
    using namespace std::placeholders;

    bool const flush = transaction.request_content_length > 0;

    //
    // TODO create a temporary file and move in place when the request
    // succesfully completes.
    //

    // Create file.
    m_stream.open(filepath);
    if (false == m_stream.is_open()) {
        transaction.respond(flush, http::StatusCode::Forbidden, {});
        return;
    }

    // Throw exceptions on failures.
    m_stream.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    // Content in PUT request?
    if (0 == transaction.request_content_length) {
        transaction.respond(false, http::StatusCode::Ok, {});
        return;
    }

    // Progressively receive and write content.
    transaction.receive(
        std::make_shared<Buffer>(hlib::Config::httpServerContentChunkSize()),
        std::bind(&PutFile::onReceive, this, _1, _2, _3)
    );
}

//
// Public (DeleteFile)
//
DeleteFile::DeleteFile(http::Server::Transaction& transaction, std::filesystem::path filepath)
{
    bool const flush = transaction.request_content_length > 0;

    if (false == std::filesystem::exists(filepath)) {
        transaction.respond(flush, http::StatusCode::NotFound, {});
        return;
    }
    else if (false == file::is_writable(filepath)) {
        transaction.respond(flush, http::StatusCode::Forbidden, {});
        return;
    }
    else if (std::filesystem::file_type::regular != std::filesystem::status(filepath).type()) {
        transaction.respond(flush, http::StatusCode::Forbidden, {});
        return;
    }

    // Remove file from filesystem.
    std::filesystem::remove(filepath);

    // Respond Ok.
    transaction.respond(flush, http::StatusCode::Ok, {});
}

