add_library(Analyse SHARED "")

target_sources(Analyse 
PRIVATE
    Analyse.cpp
    BacnetipAnalyse/BacnetipAnalyse.cpp
    Dlt645Analyse/Dlt645Analyse.cpp
    IEC104Analyse/IEC104Analyse.cpp
    ModbusRtuAnalyse/ModbusRtuAnalyse.cpp
    ModbusTcpAnalyse/ModbusTcpAnalyse.cpp
    OpcuaAnalyse/OpcuaAnalyse.cpp
    Pro188Analyse/Pro188Analyse.cpp
PUBLIC
    Analyse.h
    BacnetipAnalyse/BacnetipAnalyse.h
    Dlt645Analyse/Dlt645Analyse.h
    IEC104Analyse/IEC104Analyse.h
    ModbusRtuAnalyse/ModbusRtuAnalyse.h
    ModbusTcpAnalyse/ModbusTcpAnalyse.h
    OpcuaAnalyse/OpcuaAnalyse.h
    Pro188Analyse/Pro188Analyse.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Analyse 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)