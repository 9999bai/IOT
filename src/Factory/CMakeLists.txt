add_library(Factory SHARED "")

target_sources(Factory 
PRIVATE
    Factory.cpp
    BacnetipFactory/BacnetipFactory.cpp
    Dlt645Factory/Dlt645Factory.cpp
    IEC104Factory/IEC104Factory.cpp
    ModbusRtuFactory/ModbusRtuFactory.cpp
    ModbusTcpFactory/ModbusTcpFactory.cpp
    OpcuaFactory/OpcuaFactory.cpp
    Pro188Factory/Pro188Factory.cpp
PUBLIC
    Factory.h
    BacnetipFactory/BacnetipFactory.h
    Dlt645Factory/Dlt645Factory.h
    IEC104Factory/IEC104Factory.h
    ModbusRtuFactory/ModbusRtuFactory.h
    ModbusTcpFactory/ModbusTcpFactory.h
    OpcuaFactory/OpcuaFactory.h
    Pro188Factory/Pro188Factory.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Factory
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)