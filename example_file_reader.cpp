#include "async_file_reader.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace asio = boost::asio;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file1> [file2] [file3] ..." << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  for (int i = 1; i < argc; i++) {
    files.push_back(argv[i]);
  }

  std::cout << "\nProcessing " << files.size() << " files using Boost.Asio async file operations (with IO_URING):" << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  asio::io_context io_context;

  // Spawn coroutines for each file
  for (const auto& filename : files) {
    asio::spawn(io_context, [&io_context, filename](asio::yield_context yield) {
      EVP_MD_CTX* ctx = EVP_MD_CTX_new();
      EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
        read_file_chunks(yield, io_context, filename, 
            [ctx, &filename](const char* buffer, size_t chunk_number, size_t offset, size_t bytes_read) {
            // Update MD5 with this chunk
            EVP_DigestUpdate(ctx, buffer, bytes_read);
            });

        // Finalize MD5 hash for this file
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        EVP_DigestFinal_ex(ctx, hash, &hash_len);

        // Convert to hex string
        std::stringstream ss;
        for (unsigned int i = 0; i < hash_len; i++) {
          ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        std::cout << filename << ": " << ss.str() << std::endl;
        EVP_MD_CTX_free(ctx);
        });
  }

  io_context.run();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Boost.Asio async file operations (with IO_URING) processing time: " << duration.count() / 1000.0 << " ms" << std::endl;

  return 0;
}
