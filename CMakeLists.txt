project(FFIXBinmaster)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory("common")
add_subdirectory("ffix-extract-img")
add_subdirectory("ffix-extract-db")
add_subdirectory("ffix-convert-bs")
