#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <string>


void read_file_chunks(boost::asio::yield_context yield, boost::asio::io_context& io_context, const std::string& filename);
