add_executable(main GatewayManage.cpp main.cpp)

add_subdirectory(myHelper)
add_subdirectory(Database)
add_subdirectory(Analyse)
add_subdirectory(Frame)
add_subdirectory(Mediator)
add_subdirectory(NetSerial)
add_subdirectory(Observer)
add_subdirectory(Factory)
add_subdirectory(open62541)
add_subdirectory(Control)


target_link_libraries(main 
PRIVATE
    myHelper
    Database
    Analyse
    Frame
    Mediator
    NetSerial
    Observer
    Factory
    Control
    open62541
    mymuduo
    jsoncpp
    mysqlclient
    mosquitto
    mosquittopp
)
