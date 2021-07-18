cmake_minimum_required(VERSION 3.19)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_package(benchmark REQUIRED)

set(SRCS
    input_parser.h
    input_parser.cpp
    graph.h
    graph.cpp
    single_thread.h
    single_thread.cpp
    multi_thread_one.h
    multi_thread_one.cpp
    multi_thread_two_batches.h
    multi_thread_two_batches.cpp)

add_executable(spreadsheet main.cpp ${SRCS})
target_link_libraries(spreadsheet Threads::Threads)

add_executable(benchmark benchmark.cpp ${SRCS})
target_link_libraries(benchmark benchmark::benchmark)