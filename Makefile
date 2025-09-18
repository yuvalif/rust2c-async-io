CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11
CXXFLAGS = -Wall -Wextra -std=c++20
BOOST_FLAGS = $(CXXFLAGS) -lboost_context -lboost_coroutine
C_TARGET = example_c
CPP_TARGET = example_cpp
ASYNC_TARGET = example_async
BOOST_TARGET = example_boost_coro
RUST_LIB = libasync_file_hasher.so

.PHONY: all clean rust-lib

all: rust-lib $(C_TARGET) $(CPP_TARGET) $(ASYNC_TARGET) $(BOOST_TARGET)

rust-lib:
	cargo build --release

$(C_TARGET): example.c $(RUST_LIB)
	$(CC) $(CFLAGS) -o $(C_TARGET) example.c -L./target/release -lasync_file_hasher

$(CPP_TARGET): example.cpp $(RUST_LIB)
	$(CXX) $(CXXFLAGS) -o $(CPP_TARGET) example.cpp -L./target/release -lasync_file_hasher

$(ASYNC_TARGET): example_async.cpp $(RUST_LIB)
	$(CXX) $(CXXFLAGS) -o $(ASYNC_TARGET) example_async.cpp -L./target/release -lasync_file_hasher

$(BOOST_TARGET): example_boost_coro.cpp $(RUST_LIB)
	$(CXX) $(BOOST_FLAGS) -o $(BOOST_TARGET) example_boost_coro.cpp -L./target/release -lasync_file_hasher

$(RUST_LIB): rust-lib

clean:
	rm -f $(C_TARGET) $(CPP_TARGET) $(ASYNC_TARGET) $(BOOST_TARGET)
	cargo clean

