add_library(Observer SHARED "")

target_sources(Observer 
PRIVATE
    Observer.cpp
    MqttClient/MqttClient.cpp
    RabbitmqClient/RabbitmqClient.cpp
PUBLIC
    Observer.h
    MqttClient/MqttClient.h
    RabbitmqClient/RabbitmqClient.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_include_directories(Observer 
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)