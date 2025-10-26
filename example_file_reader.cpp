#include "async_file_reader.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

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

  std::cout << "\nReading " << files.size() << " files using Boost.Asio async file operations (with IO_URING):" << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  try {
    asio::io_context io_context;

    // Spawn coroutines for each file
    for (const auto& filename : files) {
      asio::spawn(io_context, [&io_context, filename](asio::yield_context yield) {
          read_file_chunks(yield, io_context, filename);
          });
    }

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Boost.Asio coroutine reading time: " << duration.count() / 1000.0 << " ms" << std::endl;

  return 0;
}
