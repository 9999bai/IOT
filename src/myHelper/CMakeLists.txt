add_library(myHelper SHARED "")

target_sources(myHelper 
PRIVATE
    myHelper.cpp
PUBLIC
    myHelper.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_include_directories(myHelper 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)