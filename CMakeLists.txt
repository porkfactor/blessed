cmake_minimum_required(VERSION 3.10.0)

project(blessed VERSION 0.1.0)

add_library(blessed INTERFACE)

set_target_properties(blessed PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/include
)
    
