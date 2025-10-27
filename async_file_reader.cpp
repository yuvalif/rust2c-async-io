#include "async_file_reader.h"
#include <boost/asio/random_access_file.hpp>
#include <iostream>
#include <vector>

constexpr size_t CHUNK_SIZE = 4 * 1024 * 1024; // 4MB

void read_file_chunks(const boost::asio::yield_context& yield,
    boost::asio::io_context& io_context,
    const std::string& filename,
    void (*chunk_callback)(const char*, size_t, void* user_date),
    void* user_date) {
  try {
    boost::asio::random_access_file file(io_context, filename, boost::asio::file_base::read_only);
    std::vector<char> buffer(CHUNK_SIZE);
    size_t offset = 0;
    boost::system::error_code ec;

    while (true) {
      const size_t bytes_read = file.async_read_some_at(
          offset, 
          boost::asio::buffer(buffer), 
          yield[ec]
          );
      if (ec) {
        if (ec != boost::asio::error::eof) std::cerr << "Error reading file " << filename << ": " << ec.message() << std::endl;
        break;
      }
      if (bytes_read == 0) {
        break;
      }
      
      // Call the callback with buffer data, bytes read and user data
      chunk_callback(buffer.data(), bytes_read, user_date);
      
      offset += bytes_read;
      if (bytes_read < CHUNK_SIZE) {
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception reading file " << filename << ": " << e.what() << std::endl;
  }
}

