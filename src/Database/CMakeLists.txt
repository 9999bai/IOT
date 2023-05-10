add_library(Database SHARED "")

target_sources(Database 
PRIVATE
    Database.cpp
    MysqlDatabase/MysqlDatabase.cpp
    SqliteDatabase/SqliteDatabase.cpp
PUBLIC
    Database.h
    MysqlDatabase/MysqlDatabase.h
    SqliteDatabase/SqliteDatabase.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Database 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)


