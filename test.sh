#!/bin/bash

for i in {1..10}; do
    filename="random_file_${i}.bin"
    if [ ! -f "$filename" ]; then
        echo "creating $filename..."
        dd if=/dev/urandom of="$filename" bs=1M count=128
    else
        echo "$filename already exists, skipping..."
    fi
done

cargo run --release -- random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_c random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_cpp random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_async random_file_*.bin
LD_LIBRARY_PATH=./target/release ./example_boost_coro random_file_*.bin
