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
    void secTimer();
    void sendDataTimer();
    void sendDataRD();
    void sendDataTD();

    bool findProname(const int& gatewayId, enum_pro_name& pro_name);
    bool findModbusParam(const iot_data_item &item, iot_device &dest_device, iot_template& dest_templat, iot_sub_template &dest_subtemplat);
    bool findControlParam(const iot_data_item &item, enum_pro_name &pro_name, iot_device &device, iot_template &dest_templat, iot_sub_template &dest_subtemplat);

    void WriteBytetypeData(enum_byte_order byteorder, const frame& td, frame& t_frame);


    //写单个线圈/寄存器
    // void controlFrameModbusRTU(int gateway_id, const std::string& value, const iot_device& device, const iot_template& templat);
    void controlFrameModbusRTU(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const iot_sub_template& sub_templat);
    void controlFrameModbusTCP(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const iot_sub_template& sub_templat);
    void controlFrameBacnetIP(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat);

    void CreateModbusRTUFactory(const iot_gateway& gateway);
    void CreateModbusTCPFactory(const iot_gateway& gateway);
    void CreateDLT645Factory(const iot_gateway& gateway);
    void CreateBUS188Factory(const iot_gateway& gateway);
    void CreateIEC104Factory(const iot_gateway& gateway);
    void CreateBAcnetIPFactory(const iot_gateway& gateway);
    void CreateOPCUAFactory(const iot_gateway& gateway);

    void ThreadPoolInit(int size);

    void onAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type);


    // bacnetip
    void strToBacnetIPValue(const std::string &value, int valueType, frame &data);


    EventLoop *loop_;

    std::mutex lock_;
    int sendFinish_;
    std::shared_ptr<ThreadPool> poolPtr_;
    std::vector<iot_gateway> v_gateway_;
    std::vector<controlmediator> v_controlmediator_;
    ObserverPtr observerPtr_;

    // FactoryPtr modbusRtuFactory;
};