include_directories(
    ../ffix-common
)

add_executable(ffix-extract-img
    main.cc
)

target_link_libraries(ffix-extract-img
    ffix-common
    boost_filesystem
    boost_system
)
