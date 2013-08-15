set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ../common
)

add_executable(ffix-extract-img
    main.cc
)

target_link_libraries(ffix-extract-img
    common
    boost_filesystem
    boost_program_options
    boost_system
)
