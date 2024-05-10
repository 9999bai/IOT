add_library(Control SHARED "")

target_sources(Control 
PRIVATE
    Control.cpp
    BacnetipControl/BacnetipControl.cpp
    Dlt645Control/Dlt645Control.cpp
    IEC104Control/IEC104Control.cpp
    ModbusRtuControl/ModbusRtuControl.cpp
    ModbusTcpControl/ModbusTcpControl.cpp
    OpcuaControl/OpcuaControl.cpp
    Pro188Control/Pro188Control.cpp
PUBLIC
    Control.h
    BacnetipControl/BacnetipControl.h
    Dlt645Control/Dlt645Control.h
    IEC104Control/IEC104Control.h
    ModbusRtuControl/ModbusRtuControl.h
    ModbusTcpControl/ModbusTcpControl.h
    OpcuaControl/OpcuaControl.h
    Pro188Control/Pro188Control.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Control 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)