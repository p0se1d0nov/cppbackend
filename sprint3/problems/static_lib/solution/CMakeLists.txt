cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)
add_definitions(-D_WIN32_WINNT=0x0601)
include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

get_property(importTargets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message(STATUS "${importTargets}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(model STATIC
    src/model.cpp
    src/model.h
    src/loot_generator.cpp
    src/loot_generator.h
    src/extradata.cpp
    src/extradata.h
    src/tagged.h
    src/player.h
    src/players.h
    src/ticker.h
    src/app.h
    src/app.cpp
    src/token_generator.h
)

target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(model
    PUBLIC CONAN_PKG::boost
)


add_executable(game_server
        src/main.cpp
        src/http_server.cpp
        src/http_server.h
        src/sdk.h
        src/boost_json.cpp
        src/json_loader.h
        src/json_loader.cpp
        src/request_handler.cpp
        src/request_handler.h
        src/logger.h
        src/logger.cpp
        src/command_line_parser.h src/command_line_parser.cpp
)
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server
    PRIVATE
        model
        CONAN_PKG::boost
        Threads::Threads
)

# add_executable(game_server_test
#         src/model.h
#         src/model.cpp
#         src/tagged.h
#         src/loot_generator.h
#         src/loot_generator.cpp
#         tests/loot_generator_tests.cpp
# )
# target_include_directories(game_server_test PRIVATE  CONAN_PKG::boost)
# target_link_libraries(game_server_test PRIVATE CONAN_PKG::catch2 CONAN_PKG::boost Threads::Threads)
# #

