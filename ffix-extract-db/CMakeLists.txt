include_directories(
    ../common
)

add_executable(ffix-extract-db
    main.cc
)

target_link_libraries(ffix-extract-db
    common
    boost_filesystem
    boost_system
)
