target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_request_handler.h
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)