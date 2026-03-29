#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <utility>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/attributes.hpp>
#include "async_file_hasher.h"

using namespace boost::asio;

using signature = void(boost::system::error_code, std::string);

// Context for async operations
struct AsyncContext {
  any_completion_handler<signature> handler;
};

// Coroutine to process a single file
template <completion_token_for<signature> CompletionToken>
auto async_process_file_coro(RuntimeHandle* runtime, const std::string& filename, CompletionToken&& token) {
  return async_initiate<CompletionToken, signature>(
    [] (auto h, RuntimeHandle* runtime, const std::string& filename) {
      auto ctx = new AsyncContext{std::move(h)};

      // Call the async Rust function
      calculate_md5_hash_c(
          runtime,
          filename.c_str(),
          [](char* hash, void* user_data) {
            auto* ctx = static_cast<AsyncContext*>(user_data);

            std::string result;
            if (hash != nullptr) {
              result = std::string(hash);
              free_string(hash);
            }

            auto handler = std::move(ctx->handler);
            delete ctx;

            boost::system::error_code ec;
            dispatch(append(std::move(handler), ec, std::move(result)));
          }, ctx);
    }, token, runtime, filename);
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

#ifndef STATIC_TOKIO
  // Create Tokio runtime
  RuntimeHandle* runtime = create_runtime();
  if (!runtime) {
    std::cerr << "Failed to create Tokio runtime" << std::endl;
    return 1;
  }
#else
  RuntimeHandle* runtime = nullptr;
  std::cout << "(using static tokio runtime)" << std::endl;
#endif

  auto start = std::chrono::high_resolution_clock::now();

  io_context io;
  auto work_guard = boost::asio::make_work_guard(io);
  std::atomic<size_t> work_count{files.size()};

  // Start worker threads before spawning coroutines
  const auto num_threads = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), files.size());
  std::vector<std::thread> threads;
  for (unsigned i = 0; i < num_threads; ++i) {
    threads.emplace_back([&io]() { io.run(); });
  }

  std::map<std::string, std::string> results;

  // Spawn coroutines for each file
  for (const auto& file : files) {
    spawn(io,
        [&io, runtime, file, &work_count, &work_guard, &results](yield_context yield) {
          boost::system::error_code ec;
          std::string hash = async_process_file_coro(runtime, file, yield[ec]);
          // Print the result
          if (--work_count == 0) {
            work_guard.reset();
          }
          std::stringstream ss;
          ss << "[thread " << std::this_thread::get_id() << "] " << file << ": ";
          if (ec) {
            ss << "ERROR: " << ec;
          } else {
            ss << hash;
          }
          results[file] = ss.str();
        }
      );
  }

  // Wait for all threads to finish
  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  for (const auto& result : results) {
    std::cout << result.second << std::endl;
  }

  std::cout << "Boost.Asio coroutine processing time: " << duration.count() / 1000.0 << " ms" << std::endl;

  // Clean up runtime
  free_runtime(runtime);

  return 0;
}

