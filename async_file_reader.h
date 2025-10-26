#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <string>


#include <functional>

void read_file_chunks(boost::asio::yield_context yield, boost::asio::io_context& io_context, const std::string& filename, std::function<void(const char*, size_t, size_t, size_t)> chunk_callback);
