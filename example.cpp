#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "async_file_hasher.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2] [file3] ..." << std::endl;
        return 1;
    }

    std::vector<std::string> files;
    for (int i = 1; i < argc; i++) {
        files.push_back(argv[i]);
    }

    std::cout << "\nProcessing " << files.size() << " files using C++ bindings:" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (const auto& file : files) {
        char* hash = calculate_md5_hash_sync_c(file.c_str());

        if (hash != nullptr) {
            std::cout << file << ": " << hash << std::endl;
            free_string(hash);
        } else {
            std::cerr << "Error processing file: " << file << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "C++ processing time: " << duration.count() / 1000.0 << " ms" << std::endl;

    return 0;
}
