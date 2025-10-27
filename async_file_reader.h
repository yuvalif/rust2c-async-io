#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <string>

void read_file_chunks(const boost::asio::yield_context& yield,
    boost::asio::io_context& io_context,
    const std::string& filename,
    void (*chunk_callback)(const char*, size_t, void* user_date),
    void* user_date=nullptr);

