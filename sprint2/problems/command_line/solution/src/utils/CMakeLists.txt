set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-enum-constexpr-conversion)

target_sources(game_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/path_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/path_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/request_handler_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/randomizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/randomizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tagged.h
)

target_include_directories(game_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}      
    ${CMAKE_CURRENT_SOURCE_DIR}/..  
)