set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/http_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/http_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/authorization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/authorization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ticker.h
	${CMAKE_CURRENT_SOURCE_DIR}/ticker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_worker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/json_loader.h
	${CMAKE_CURRENT_SOURCE_DIR}/json_loader.cpp
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)