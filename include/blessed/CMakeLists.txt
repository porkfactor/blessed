cmake_minimum_required(VERSION 3.0)

target_sources(blessed PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bit.hpp
    ${CMAKE_CURRENT_LIST_DIR}/blessed.hpp
    ${CMAKE_CURRENT_LIST_DIR}/byteswap.hpp
    ${CMAKE_CURRENT_LIST_DIR}/string_view.hpp
    ${CMAKE_CURRENT_LIST_DIR}/uuid.hpp
)