cmake_minimum_required(VERSION 3.19)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(spreadsheet main.cpp input_parser.cpp input_parser.h)
target_link_libraries(spreadsheet Threads::Threads)

add_executable(benchmark benchmark.cpp input_parser.cpp input_parser.h)
