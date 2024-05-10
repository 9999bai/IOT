add_library(open62541 SHARED "")


add_definitions("-std=c99")

target_sources(open62541 
PRIVATE
    open62541_gm.c
PUBLIC
    open62541_gm.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_include_directories(open62541 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)