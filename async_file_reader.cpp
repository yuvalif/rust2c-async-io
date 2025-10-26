#include "async_file_reader.h"
#include <boost/asio/random_access_file.hpp>
#include <iostream>
#include <vector>

constexpr size_t CHUNK_SIZE = 4 * 1024 * 1024; // 4MB

namespace asio = boost::asio;

void read_file_chunks(asio::yield_context yield, asio::io_context& io_context, const std::string& filename, std::function<void(const char*, size_t, size_t, size_t)> chunk_callback) {
  try {
    asio::random_access_file file(io_context, filename, asio::file_base::read_only);
    std::vector<char> buffer(CHUNK_SIZE);
    size_t offset = 0;
    size_t chunk_number = 0;
    boost::system::error_code ec;

    while (true) {
      const size_t bytes_read = file.async_read_some_at(
          offset, 
          asio::buffer(buffer), 
          yield[ec]
          );
      if (ec) {
        if (ec != asio::error::eof) std::cerr << "Error reading file " << filename << ": " << ec.message() << std::endl;
        break;
      }
      if (bytes_read == 0) {
        break;
      }
      
      // Call the callback with buffer data, chunk number, offset, and bytes read
      chunk_callback(buffer.data(), chunk_number++, offset, bytes_read);
      
      offset += bytes_read;
      if (bytes_read < CHUNK_SIZE) {
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception reading file " << filename << ": " << e.what() << std::endl;
  }
}

