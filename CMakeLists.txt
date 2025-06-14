cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CacheIt)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(cacheit_server
    src/server.cpp
    src/conn.cpp
    src/command.cpp
    src/epoll_manager.cpp
    src/logger.cpp
    src/utils.cpp
    # add other needed source files
)

add_executable(cacheit_client
    src/client.cpp
    src/conn.cpp
    src/command.cpp
    src/epoll_manager.cpp
    src/logger.cpp
    src/utils.cpp
    # add other needed source files
)

add_definitions(-D_DEBUG)

set(PRINT true)