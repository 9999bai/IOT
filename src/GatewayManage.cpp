#include "GatewayManage.h"

GatewayManage::GatewayManage(EventLoop* loop, const std::vector<iot_gateway>& v_gateway, const mqtt_info& mqttconf)
                : loop_(loop)
                , v_gateway_(v_gateway)
                , sendFinish_(0)
                , poolPtr_(std::make_shared<ThreadPool>("IOT_Gateway ThreadPool"))
{
    endianMode();//大小端
    ThreadPoolInit((v_gateway.size()*2) > THREADPOOL_MIN_THREAD_SIZE ? (v_gateway.size()*2) : THREADPOOL_MIN_THREAD_SIZE);

    // MQTT
    observerPtr_ = std::make_shared<MqttClient>(loop, mqttconf);
    // 解析控制帧
    observerPtr_->setObserverRecvCallback(std::bind(&GatewayManage::onObserverRecv, this, std::placeholders::_1, std::placeholders::_2));
    observerPtr_->start();
    
    loop_->runEvery(1, std::bind(&GatewayManage::secTimer, this));
}

GatewayManage::~GatewayManage()
{

}

void GatewayManage::ThreadPoolInit(int size)
{
    LOG_INFO("开启线程数量 = %d", size > THREADPOOL_MAX_THREAD_SIZE ? THREADPOOL_MAX_THREAD_SIZE : size);
    poolPtr_->setMaxQueueSize(THREADPOOL_MAX_QUEUE_SIZE);                                  // 队列最大容量
    poolPtr_->start(size > THREADPOOL_MAX_THREAD_SIZE ? THREADPOOL_MAX_THREAD_SIZE : size);// 开启线程数量
    LOG_INFO("GatewayManage::ThreadPoolInit over...");
}

void GatewayManage::onObserverRecv(const std::string& topic, const std::string& msg)
{
    LOG_INFO("GatewayManage::onObserverRecv-------------------%s", msg.c_str());
    std::vector<iot_data_item> control_item;
    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(msg.c_str(), msg.c_str() + msg.length(), root))
    {
        LOG_ERROR("JSON parse error...");
    }else{
        LOG_INFO("JSON parse success...");
    }
    int count = root.size();
    LOG_INFO("count = %d", count);
    for (auto i = 0; i < count; i++)
    {
        iot_data_item item;
        item.gateway_id = root[i]["gateway_id"].asInt();
        item.device_id = root[i]["device_id"].asInt();
        item.param_id = root[i]["param_id"].asInt();
        item.value = root[i]["value"].asString();
        control_item.emplace_back(item);
        LOG_INFO("recv : gateway_id=%d,device_d=%d, param_id=%d, value=%s", item.gateway_id, item.device_id, item.param_id, item.value.c_str());
    }
    
    for(auto item : control_item)
    {
        iot_device device;      // 要控制的设备所属的device
        iot_template templat;   // 要控制的设备所属的templat
        iot_sub_template sub_templat;
        enum_pro_name pro_name; // 所属协议

        if(findProname(item.gateway_id, pro_name)) // 查找确定协议类型
        {
            switch (pro_name)
            {
                case enum_pro_name_ModbusRTU:
                {
                    ControlPtr control = std::make_shared<ModbusRtuControl>(v_gateway_);
                    poolPtr_->run(std::bind(&Control::ControlFunc, control, item, v_controlmediator_));
                    break;
                }
                case enum_pro_name_ModbusTCP:
                {
                    ControlPtr control = std::make_shared<ModbusTcpControl>(v_gateway_);
                    poolPtr_->run(std::bind(&Control::ControlFunc, control, item, v_controlmediator_));
                    break;
                }
                case enum_pro_name_DLT645:
                    break;
                case enum_pro_name_CJT188:
                    break;
                case enum_pro_name_IEC104:
                    break;
                case enum_pro_name_BAcnetIP:
                {
                    LOG_INFO("control enum_pro_name_BAcnetIP...");
                    ControlPtr control = std::make_shared<BacnetipControl>(v_gateway_);
                    poolPtr_->run(std::bind(&Control::ControlFunc, control, item, v_controlmediator_));
                    break;
                }
                case enum_pro_name_OPCUA:
                {
                    LOG_INFO("control enum_pro_name_OPCUA...");
                    ControlPtr control = std::make_shared<OpcuaControl>(v_gateway_);
                    poolPtr_->run(std::bind(&Control::ControlFunc, control, item, v_controlmediator_));
                    break;
                }
                default:
                    LOG_ERROR("write pro_name error [wfunc code]=%d", (int)pro_name);
                    break;
            }
        }
        else
        {
            // 没有对应的设备
            LOG_ERROR("not finddevice...");
        }
    }
}

bool GatewayManage::findProname(const int& gatewayId, enum_pro_name& pro_name)
{
    for(auto gateway : v_gateway_)
    {
        if(gatewayId == gateway.gateway_id)
        {
            pro_name = gateway.pro_name;
            return true;
        }
    }
    return false;
}

void GatewayManage::CreateModbusRTUFactory(const iot_gateway& gateway)
{
    FactoryPtr modbusRtuFactory = std::make_shared<ModbusRtuFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<ModbusRtuMediator>(loop_, gateway, poolPtr_, modbusRtuFactory);

    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    // recvAnalysePtr->setAnalyseFinishCallback(std::bind(&Mediator::onAnalyseFinish, mediatorPtr_, std::placeholders::_1));
    mediatorPtr->start();
}

void GatewayManage::CreateModbusTCPFactory(const iot_gateway& gateway)
{
    FactoryPtr modbusTcpFactory = std::make_shared<ModbusTcpFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<ModbusTcpMediator>(loop_, gateway, poolPtr_, modbusTcpFactory);
    
    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    
    mediatorPtr->start();
}

void GatewayManage::CreateDLT645Factory(const iot_gateway& gateway)
{
    FactoryPtr dlt645Factory = std::make_shared<Dlt645Factory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<Dlt645Mediator>(loop_, gateway, poolPtr_, dlt645Factory);
    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    
    mediatorPtr->start();
}

void GatewayManage::CreateBUS188Factory(const iot_gateway& gateway)
{

}

void GatewayManage::CreateIEC104Factory(const iot_gateway& gateway)
{
    FactoryPtr iec104Factory = std::make_shared<IEC104Factory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<IEC104Mediator>(loop_, gateway, poolPtr_, iec104Factory);
    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    mediatorPtr->start();
}

void GatewayManage::CreateBAcnetIPFactory(const iot_gateway& gateway)
{
    LOG_INFO("CreateBAcnetIPFactory...");
    FactoryPtr bacnetipFactory = std::make_shared<BacnetipFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<BacnetipMediator>(loop_, gateway, poolPtr_, bacnetipFactory);
    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    mediatorPtr->start();
}

void GatewayManage::CreateOPCUAFactory(const iot_gateway& gateway)
{
    LOG_INFO("CreateOPCUAFactory...");
    FactoryPtr opcuaFactory = std::make_shared<OpcUAFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<OpcUAMeiator>(loop_, gateway, poolPtr_, opcuaFactory);
    mediatorPtr->setAnalyseNotifyCallback(std::bind(&GatewayManage::onAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); // 解析完成回调

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    mediatorPtr->start();
}

void GatewayManage::start()
{
    LOG_INFO("GatewayManage  v_gateway_.size = %d ", v_gateway_.size());
    for (auto it = v_gateway_.begin(); it != v_gateway_.end(); it++)
    {
        switch(it->pro_name)
        {
            case enum_pro_name_ModbusRTU:
                LOG_INFO("CreateModbusRTUFactory: enum_pro_name_ModbusRTU %s\n", it->gateway_name.c_str());
                CreateModbusRTUFactory(*it);
                break;
            case enum_pro_name_ModbusTCP:
                LOG_INFO("CreateModbusTCPFactory: enum_pro_name_ModbusTCP %s\n", it->gateway_name.c_str());
                CreateModbusTCPFactory(*it);
                break;
            case enum_pro_name_DLT645:
                LOG_INFO("CreateDLT645Factory: enum_pro_name_DLT645 %s\n", it->gateway_name.c_str());
                CreateDLT645Factory(*it);
                break;
            case enum_pro_name_CJT188:
                LOG_INFO("CreateCJT188Factory: enum_pro_name_CJT188 %s\n", it->gateway_name.c_str());
                CreateBUS188Factory(*it);
                break;
            case enum_pro_name_IEC104:
                LOG_INFO("CreateIEC104Factory: enum_pro_name_IEC104 %s\n", it->gateway_name.c_str());
                CreateIEC104Factory(*it);
                break;
            case enum_pro_name_BAcnetIP:
                LOG_INFO("CreateBAcnetIPFactory: enum_pro_name_BAcnetIP %s\n", it->gateway_name.c_str());
                CreateBAcnetIPFactory(*it);
                break;
            case enum_pro_name_OPCUA:
                LOG_INFO("CreateOPCUAFactory: enum_pro_name_OPCUA %s\n", it->gateway_name.c_str());
                CreateOPCUAFactory(*it);
                break;
            default:
                LOG_INFO("GatewayManage::start() error it->pro_name = %d\n", (int)it->pro_name);
                break;
        }
    }
}

void GatewayManage::secTimer()
{
    for(auto it : v_controlmediator_)
    {
        it.second->secTimer();
    }
    sendDataTimer();
}

void GatewayManage::sendDataTimer() // 定时发送 数据
{
    if(sendFinish_ != 0) // 上一次的数据还没发送完
        return;
    
    poolPtr_->run(std::bind(&GatewayManage::sendDataTD, this));
    poolPtr_->run(std::bind(&GatewayManage::sendDataRD, this));
}

void GatewayManage::sendDataRD()
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        sendFinish_++;
    }

    while (!queue_iotdataRD_.empty())
    {
        iot_data_item item;
        {
            std::unique_lock<std::mutex> lock(RD_mutex_);
            item = queue_iotdataRD_.front();
            queue_iotdataRD_.pop();
        }
        std::string jsonStr = itemToJson(RD_Data_Title, item);
        observerPtr_->publicTopic(jsonStr);
        // LOG_INFO("MQTT publish RD size = %d", jsonStr.size());
    }
    {
        std::unique_lock<std::mutex> lock(lock_);
        sendFinish_--;
    }
}

void GatewayManage::sendDataTD()
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        sendFinish_++;
    }

    static int staticcount = 0;
    if (++staticcount >= SendPeriodTimer)
    {
        staticcount = 0;
        std::unique_lock<std::mutex> lock(TD_mutex_);
        int count = 0;
        std::vector<iot_data_item> v_item;
        for (auto it = list_iotdataTD_.begin(); it != list_iotdataTD_.end(); it++)
        {
            v_item.emplace_back(*it);
            if (++count >= TD_DataSize)
            {
                std::string jsonStr = itemToJson(TD_Data_Title,v_item);
                observerPtr_->publicTopic(jsonStr);
                // LOG_INFO("MQTT publish TD size = %d", jsonStr.size());
                v_item.clear();
                count = 0;
            }
        }
        if(!v_item.empty())
        {
            std::string jsonStr = itemToJson(TD_Data_Title,v_item);
            observerPtr_->publicTopic(jsonStr);
            // LOG_INFO("MQTT publish TD size = %d", jsonStr.size());
        }
    }
    {
        std::unique_lock<std::mutex> lock(lock_);
        sendFinish_--;
    }
}

void GatewayManage::onAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(rw == enum_write)
    {
        if(ok)
        {
            // 写 成功
            LOG_INFO(" 写成功...");
            // observerPtr_->publicTopic("21");
        }
        else
        {
            // 写 失败
            LOG_INFO(" 写失败...");
            // observerPtr_->publicTopic("20");
        }
    }
    else if(rw == enum_read)
    {
        if(ok)
        {
            // 读 成功
            // LOG_INFO(" 读成功...");
            // observerPtr_->publicTopic("11");
        }
        else
        {
            // 读 失败
            LOG_INFO(" 读失败...");
            // observerPtr_->publicTopic("10");
        }
    }
}
