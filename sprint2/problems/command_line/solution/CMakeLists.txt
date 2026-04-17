cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")

set(CONAN_DISABLE_CHECK_COMPILER TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/main.cpp
	src/sdk.h
	src/boost_headers/boost_json.cpp
)

add_subdirectory(src/request_handlers)
add_subdirectory(src/boost_headers)
add_subdirectory(src/utils)
add_subdirectory(src/log_items)
add_subdirectory(src/routes)
add_subdirectory(src/models)
add_subdirectory(src/services)

target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server CONAN_PKG::boost) 

set_target_properties(game_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

