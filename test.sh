#!/bin/bash

for i in {1..10}; do
    filename="random_file_${i}.bin"
    filesize=$((64 + RANDOM % 65))
    if [ ! -f "$filename" ]; then
        echo "creating $filename..."
        dd if=/dev/urandom of="$filename" bs=1M count="$filesize"
    else
        echo "$filename already exists, skipping..."
    fi
done

cargo run --release -- random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_c random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_cpp random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_async random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_boost_coro random_file_*.bin
./example_file_reader random_file_*.bin
