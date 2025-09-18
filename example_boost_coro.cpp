#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/attributes.hpp>
#include "async_file_hasher.h"

using namespace boost::asio;

// Context for async operations
struct AsyncContext {
  io_context* io;
  std::string hash;
  bool error = true;
  bool completed = false;
  std::function<void()> resume_coro;
};

// Coroutine to process a single file
void process_file_coro(io_context& io, RuntimeHandle* runtime, const std::string& filename, yield_context yield) {
  auto ctx = std::make_shared<AsyncContext>();
  ctx->io = &io;

  // Set up the resume function
  ctx->resume_coro = [ctx, yield]() mutable {
    // Post to the io_context to resume the coroutine on the correct thread
    ctx->io->post([ctx]() {
        // This will effectively resume the suspended coroutine
        ctx->completed = true;
      });
  };

  // Call the async Rust function
  calculate_md5_hash_c(
      runtime,
      filename.c_str(),
      [](char* hash, void* user_data) {
        auto* ctx = static_cast<AsyncContext*>(user_data);

        if (hash != nullptr) {
          ctx->hash = std::string(hash);
          ctx->error = false;
          free_string(hash);
        }

        // Resume the coroutine by posting to the io_context
        ctx->io->post([ctx]() {
          ctx->completed = true;
        });
    },
    ctx.get()
  );

  // Wait for the async operation to complete
  while (!ctx->completed) {
    // Yield control back to the io_context
    post(io, yield);
  }

  // Print the result
  if (ctx->error) {
    std::cout << filename << ": ERROR" << std::endl;
  } else {
    std::cout << filename << ": " << ctx->hash << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file1> [file2] [file3] ..." << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  for (int i = 1; i < argc; i++) {
    files.push_back(argv[i]);
  }

  std::cout << "\nProcessing " << files.size() << " files using Boost.Asio coroutines:" << std::endl;

  // Create Tokio runtime
  RuntimeHandle* runtime = create_runtime();
  if (!runtime) {
    std::cerr << "Failed to create Tokio runtime" << std::endl;
    return 1;
  }

  auto start = std::chrono::high_resolution_clock::now();

  io_context io;

  // Spawn coroutines for each file
  for (const auto& file : files) {
    spawn(io,
        [&io, runtime, file](yield_context yield) {
          process_file_coro(io, runtime, file, yield);
      }
    );
  }

  // Run the event loop
  io.run();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Boost.Asio coroutine processing time: " << duration.count() / 1000.0 << " ms" << std::endl;

  // Clean up runtime
  free_runtime(runtime);

  return 0;
}

