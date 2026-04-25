set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/route_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/iroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/map_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/map_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/player_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/players_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/players_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/game_join_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game_join_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/game_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/game_state_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game_state_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_versions_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_versions_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/action_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/action_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tick_route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tick_route.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/request_constant_values.h
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)