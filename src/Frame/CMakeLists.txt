add_library(Frame SHARED "")

target_sources(Frame 
PRIVATE
    Frame.cpp
    BacnetipFrame/BacnetipFrame.cpp
    Dlt645Frame/Dlt645Frame.cpp
    IEC104Frame/IEC104Frame.cpp
    ModbusRtuFrame/ModbusRtuFrame.cpp
    ModbusTcpFrame/ModbusTcpFrame.cpp
    OpcuaFrame/OpcuaFrame.cpp
    Pro188Frame/Pro188Frame.cpp
PUBLIC
    Frame.h
    BacnetipFrame/BacnetipFrame.h
    Dlt645Frame/Dlt645Frame.h
    IEC104Frame/IEC104Frame.h
    ModbusRtuFrame/ModbusRtuFrame.h
    ModbusTcpFrame/ModbusTcpFrame.h
    OpcuaFrame/OpcuaFrame.h
    Pro188Frame/Pro188Frame.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Frame 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)