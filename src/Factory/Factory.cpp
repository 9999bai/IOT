#include "Factory.h"
#include "NetSerial/mySerialPort/mySerialPort.h"
#include "NetSerial/myTcpClient/myTcpClient.h"
#include "NetSerial/myUdpClient/myUdpClient.h"

Factory::Factory(EventLoop* loop) : loop_(loop) 
{
    // LOG_INFO("Factory::Factory ctor...");
}

Factory::~Factory()
{
    LOG_INFO("Factory::Factory dtor...................................");
}


NetSerialPtr Factory::createNetSerial(const enum_netserial_type& type, const iot_gateway& gateway)
{
    switch(type)
    {
        case enum_netserial_serialport:
        {
            NetSerialPtr tmp = std::make_shared<mySerialPort>(loop_, gateway);
            return tmp;
        }
        case enum_netserial_udp:
        {
            // NetSerialPtr tmp = std::make_shared<UdpClient>(loop_, gateway);
            // return tmp;
        }
            break;
        case enum_netserial_tcp:
        {
            // NetSerialPtr tmp = std::make_shared<TcpClient>(loop_, gateway);
            // return tmp;
        }
            break;
    }
}

ObserverPtr Factory::createObserver(const enum_observer_type& type)
{
    switch(type)
    {
        case enum_observer_type_tcp:
            // return std::make_shared<TcpClient>();
            break;
        case enum_observer_type_mqtt:
            // return std::make_shared<MqttClient>();
            break;
        case enum_observer_type_rabbitmq:
            // return std::make_shared<RabbitmqClient>();
            break;
    }
}
