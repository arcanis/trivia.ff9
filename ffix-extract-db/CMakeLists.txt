include_directories(
    ../ffix-common
)

add_executable(ffix-extract-db
    main.cc
)

target_link_libraries(ffix-extract-db
    ffix-common
    boost_filesystem
    boost_system
)
