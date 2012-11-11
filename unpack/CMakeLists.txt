include_directories(
    ../common
)

add_executable(unpack
    unpack.cpp
)

target_link_libraries(unpack
    FFIXCommon
    boost_filesystem
    boost_system
)
