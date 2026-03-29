#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <thread>
#include "async_file_hasher.h"

// Function to process a single file
std::string process_file_async(const std::string& filename) {
    char* hash = calculate_md5_hash_sync_c(filename.c_str());

    std::stringstream ss;
    ss << "[thread " << std::this_thread::get_id() << "] " << filename << ": ";

    if (hash != nullptr) {
      ss  << hash;
      free_string(hash);
    } else {
      ss << "ERROR"; 
    }
    return ss.str();
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

    std::cout << "\nProcessing " << files.size() << " files using C++20 async:" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // Launch async tasks for each file
    std::vector<std::future<std::string>> futures;
    for (const auto& file : files) {
        futures.emplace_back(std::async(std::launch::async, process_file_async, file));
    }

    // Collect results
    for (auto& future : futures) {
        std::cout << future.get() << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "C++20 async processing time: " << duration.count() / 1000.0 << " ms" << std::endl;

    return 0;
}

