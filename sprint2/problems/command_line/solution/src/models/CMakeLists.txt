set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/details.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player.h
    ${CMAKE_CURRENT_SOURCE_DIR}/player.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/game_session.h
	${CMAKE_CURRENT_SOURCE_DIR}/game_session.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/players_handler.h
	${CMAKE_CURRENT_SOURCE_DIR}/players_handler.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/game.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/building.h
    ${CMAKE_CURRENT_SOURCE_DIR}/building.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/office.h
    ${CMAKE_CURRENT_SOURCE_DIR}/office.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/road.h
    ${CMAKE_CURRENT_SOURCE_DIR}/road.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/map.h
    ${CMAKE_CURRENT_SOURCE_DIR}/map.cpp
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)