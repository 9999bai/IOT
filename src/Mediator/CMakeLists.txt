add_library(Mediator SHARED "")

target_sources(Mediator 
PRIVATE
    Mediator.cpp
    BacnetipMediator/BacnetipMediator.cpp
    Dlt645Mediator/Dlt645Mediator.cpp
    IEC104Mediator/IEC104Mediator.cpp
    ModbusRtuMediator/ModbusRtuMediator.cpp
    ModbusTcpMediator/ModbusTcpMediator.cpp
    OpcuaMediator/OpcuaMediator.cpp
    Pro188Mediator/Pro188Mediator.cpp
PUBLIC
    Mediator.h
    BacnetipMediator/BacnetipMediator.h
    Dlt645Mediator/Dlt645Mediator.h
    IEC104Mediator/IEC104Mediator.h
    ModbusRtuMediator/ModbusRtuMediator.h
    ModbusTcpMediator/ModbusTcpMediator.h
    OpcuaMediator/OpcuaMediator.h
    Pro188Mediator/Pro188Mediator.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Mediator 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)