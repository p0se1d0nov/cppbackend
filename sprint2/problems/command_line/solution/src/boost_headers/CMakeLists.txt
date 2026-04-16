set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    # ${CMAKE_CURRENT_SOURCE_DIR}/request_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost_beast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/boost_json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/boost_log.h
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)