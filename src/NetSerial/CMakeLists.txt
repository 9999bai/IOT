add_library(NetSerial SHARED "")

target_sources(NetSerial 
PRIVATE
    NetSerial.cpp
    mySerialPort/mySerialPort.cpp
    myTcpClient/myTcpClient.cpp
    myUdpClient/myUdpClient.cpp
PUBLIC
    NetSerial.h
    mySerialPort/mySerialPort.h
    myTcpClient/myTcpClient.h
    myUdpClient/myUdpClient.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_include_directories(NetSerial 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)