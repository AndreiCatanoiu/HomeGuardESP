# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

idf_component_register(SRCS ${app_sources})

target_add_binary_data(${COMPONENT_LIB}
    "${CMAKE_SOURCE_DIR}/certs/fullchain.pem"
    TEXT
)
