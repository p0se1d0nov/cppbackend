cmake_minimum_required(VERSION 3.10)
project(event2dot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pg")

set(SOURCES 
    main.cpp
    config.cpp
    dotgen.cpp
    graph.cpp
    readfile.cpp)


add_executable(${PROJECT_NAME} ${SOURCES})