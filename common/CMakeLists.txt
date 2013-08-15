set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(common
    memoryrange.cpp
    path.cpp
    tim.cpp
)
