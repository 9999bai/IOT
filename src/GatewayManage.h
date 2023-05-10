#pragma once

#include <semaphore.h>
#include "/usr/include/mymuduo/ThreadPool.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/Logger.h"
#include "Mediator/ModbusRtuMediator/ModbusRtuMediator.h"
#include "Mediator/ModbusTcpMediator/ModbusTcpMediator.h"
#include "Mediator/Dlt645Mediator/Dlt645Mediator.h"
#include "Observer/MqttClient/MqttClient.h"
#include "myHelper/myHelper.h"
// // #include "Observer/RabbitMQ/RabbitMQClient.h"

class GatewayManage
{
public:
    using controlmediator = std::pair<int, MediatorPtr>;

    GatewayManage(EventLoop *loop, const std::vector<iot_gateway> &v_gateway, const mqtt_info& mqttconf);
    ~GatewayManage();

    void start();
    void onObserverRecv(const std::string& topic, const std::string& msg);

    // void setControlFrameCallback(ControlFrameCallback cb) { controlFrameCallback_ = cb; }

private:
    void sendDataRD();
    void sendDataTD();

    bool findControlParam(const iot_data_item& item, int& gateway_id, enum_pro_name& pro_name, iot_device& dest_device, iot_template& dest_templat);

    //写单个线圈/寄存器
    void controlFrameModbusRTU(int gateway_id, const std::string& value, const iot_device& device, const iot_template& templat);
    void controlFrameModbusTCP(int gateway_id, const std::string& value, const iot_device& device, const iot_template& templat);

    void CreateModbusRTUFactory(const iot_gateway& gateway);
    void CreateModbusTCPFactory(const iot_gateway& gateway);
    void CreateDLT645Factory(const iot_gateway& gateway);
    void CreateBUS188Factory(const iot_gateway& gateway);
    void CreateIEC104Factory(const iot_gateway& gateway);
    void CreateBAcnetIPFactory(const iot_gateway& gateway);
    void CreateOPCUAFactory(const iot_gateway& gateway);

    void ThreadPoolInit(int size);

    EventLoop *loop_;
    sem_t sem_RD;
    sem_t sem_TD;
    std::shared_ptr<ThreadPool> poolPtr_;
    std::vector<iot_gateway> v_gateway_;
    std::vector<controlmediator> v_controlmediator_;
    ObserverPtr observerPtr_;

    // FactoryPtr modbusRtuFactory;
};