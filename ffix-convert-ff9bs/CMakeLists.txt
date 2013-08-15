set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ../common
)

add_executable(ffix-convert-ff9bs
    main.cc
)

target_link_libraries(ffix-convert-ff9bs
    common
    boost_filesystem
    boost_program_options
    boost_system
)
