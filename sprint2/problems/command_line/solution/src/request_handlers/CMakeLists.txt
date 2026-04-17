set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/content_type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_request_handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler.cpp
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)