#pragma once

// #include <semaphore.h>
#include "/usr/include/mymuduo/ThreadPool.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/Logger.h"
#include "Mediator/ModbusRtuMediator/ModbusRtuMediator.h"
#include "Mediator/ModbusTcpMediator/ModbusTcpMediator.h"
#include "Mediator/Dlt645Mediator/Dlt645Mediator.h"
#include "Mediator/IEC104Mediator/IEC104Mediator.h"
#include "Mediator/BacnetipMediator/BacnetipMediator.h"
#include "Mediator/Pro188Mediator/Pro188Mediator.h"
#include "Mediator/OpcuaMediator/OpcuaMediator.h"
#include "Observer/MqttClient/MqttClient.h"
#include "myHelper/myHelper.h"

#include "ModbusRtuControl/ModbusRtuControl.h"
#include "ModbusTcpControl/ModbusTcpControl.h"
#include "BacnetipControl/BacnetipControl.h"
#include "OpcuaControl/OpcuaControl.h"
#include "Control/Control.h"
// // #include "Observer/RabbitMQ/RabbitMQClient.h"

class GatewayManage
{
public:

    GatewayManage(EventLoop *loop, const std::vector<iot_gateway> &v_gateway, const mqtt_info& mqttconf);
    ~GatewayManage();

    void start();
    void onObserverRecv(const std::string& topic, const std::string& msg);

    // void setControlFrameCallback(ControlFrameCallback cb) { controlFrameCallback_ = cb; }

private:
    void secTimer();
    void sendDataTimer();
    void sendDataRD();
    void sendDataTD();

    bool findProname(const int& gatewayId, enum_pro_name& pro_name);

    void CreateModbusRTUFactory(const iot_gateway& gateway);
    void CreateModbusTCPFactory(const iot_gateway& gateway);
    void CreateDLT645Factory(const iot_gateway& gateway);
    void CreateBUS188Factory(const iot_gateway& gateway);
    void CreateIEC104Factory(const iot_gateway& gateway);
    void CreateBAcnetIPFactory(const iot_gateway& gateway);
    void CreateOPCUAFactory(const iot_gateway& gateway);

    void ThreadPoolInit(int size);

    void onAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type);

    EventLoop *loop_;

    std::mutex lock_;
    int sendFinish_;
    std::shared_ptr<ThreadPool> poolPtr_;
    std::vector<iot_gateway> v_gateway_;
    std::vector<controlmediator> v_controlmediator_;
    ObserverPtr observerPtr_;
};