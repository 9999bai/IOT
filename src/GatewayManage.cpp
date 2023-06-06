#include "GatewayManage.h"

GatewayManage::GatewayManage(EventLoop* loop, const std::vector<iot_gateway>& v_gateway, const mqtt_info& mqttconf)
                : loop_(loop)
                , v_gateway_(v_gateway)
                , sendFinish_(0)
                , poolPtr_(std::make_shared<ThreadPool>("IOT_Gateway ThreadPool"))
{
    endianMode();//大小端
    ThreadPoolInit((v_gateway.size()*2) > THREADPOOL_MIN_THREAD_SIZE ? (v_gateway.size()*2) : THREADPOOL_MIN_THREAD_SIZE);

    //MQTT
    observerPtr_ = std::make_shared<MqttClient>(mqttconf);
    // 解析MQTT控制json
    observerPtr_->setObserverRecvCallback(std::bind(&GatewayManage::onObserverRecv, this, std::placeholders::_1, std::placeholders::_2));
    observerPtr_->start();
    
    loop_->runEvery(1, std::bind(&GatewayManage::sendDataTimer, this));
}

GatewayManage::~GatewayManage()
{

}

void GatewayManage::ThreadPoolInit(int size)
{
    poolPtr_->setMaxQueueSize(THREADPOOL_MAX_QUEUE_SIZE);                                  // 队列最大容量
    poolPtr_->start(size > THREADPOOL_MAX_THREAD_SIZE ? THREADPOOL_MAX_THREAD_SIZE : size);//开启线程数量
    LOG_INFO("GatewayManage::ThreadPoolInit over...");
}

void GatewayManage::onObserverRecv(const std::string& topic, const std::string& msg)
{
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
        item.device_id = root[i]["device_id"].asInt();
        item.templat_id = root[i]["templat_id"].asInt();
        item.param_id = root[i]["param_id"].asInt();
        item.value = root[i]["value"].asString();
        control_item.emplace_back(item);
        LOG_INFO("recv : device_d=%d, param_id=%d, value=%s", item.device_id, item.param_id, item.value.c_str());
    }
    
    for(auto item : control_item)
    {
        int gateway_id;
        iot_device device;
        iot_template templat;
        enum_pro_name pro_name;
        if (findControlParam(item, gateway_id, pro_name, device, templat))
        {
            switch(pro_name)
            {
                case enum_pro_name_ModbusRTU:
                    LOG_INFO("control enum_pro_name_ModbusRTU...");
                    controlFrameModbusRTU(gateway_id, item.value, device, templat);
                    break;
                case enum_pro_name_ModbusTCP:
                    break;
                case enum_pro_name_DLT645:
                    break;
                case enum_pro_name_CJT188:
                    break;
                case enum_pro_name_IEC104:
                    break;
                case enum_pro_name_BAcnetIP:
                    break;
                case enum_pro_name_OPCUA:
                    break;
                default:
                    break;
            }
        }
        else
        {
            // 没有对应的设备
        }
    }
}

bool GatewayManage::findControlParam(const iot_data_item& item, int& gateway_id, enum_pro_name& pro_name, iot_device& dest_device, iot_template& dest_templat)
{
    for(auto gateway : v_gateway_)
    {
        for(auto device : gateway.v_device)
        {
            if(item.device_id == device.device_id)
            {
                for(auto templat: device.v_template)
                {
                    if(templat.template_id == item.templat_id && templat.param_id == item.param_id && templat.priority != enum_priority_read)
                    {
                        gateway_id = gateway.gateway_id;
                        pro_name = gateway.pro_name;
                        dest_device = device;
                        dest_templat = templat;
                        dest_templat.rw = enum_write;
                        return true;
                    }
                }
                return false;
            }
        }
    }
    return false;
}

void GatewayManage::controlFrameModbusRTU(int gateway_id, const std::string& value, const iot_device& device, const iot_template& templat)
{
    frame t_frame;
    t_frame.emplace_back(atoi(device.device_addr.c_str()));
    t_frame.emplace_back((int)templat.w_func);

    switch(templat.w_func)
    {
        case enum_w_func_0x05:
        {
            uint16_To_char2(std::atoi(templat.register_addr.c_str()), t_frame);
            if(value == "0")
            {
                uint16_To_char2(0, t_frame);
            }
            else
            {
                uint16_To_char2(0xFF, t_frame);
            }
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]);//crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        case enum_w_func_0x06:
        {
            uint16_To_char2(std::atoi(templat.register_addr.c_str()), t_frame);
            uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_frame);
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]);//crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        case enum_w_func_0x0F:
        {

        }
            break;
        case enum_w_func_0x10:
        {
            uint16_To_char2(std::atoi(templat.register_addr.c_str()), t_frame);
            uint16_To_char2(templat.register_quantity, t_frame);
            t_frame.emplace_back(templat.register_quantity * 2);
            switch (templat.data_type)
            {
                case enum_data_type_int16_hex:
                    uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_frame);
                    break;
                case enum_data_type_int32_hex:
                    uint32_To_char4((u_int32_t)std::atoi(value.c_str()), t_frame);
                    break;
                case enum_data_type_float:
                    // float_To_IEEE754();
                    break;
            }
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]);//crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        default:
            LOG_ERROR("GatewayManage::controlFrameModbusRTU error...");
            break;
    }
    printFrame("Control --------------------- ", t_frame);

    for (auto it = v_controlmediator_.begin(); it != v_controlmediator_.end(); it++)
    {
        if(gateway_id == it->first)
        {
            nextFrame controlFrame(t_frame, pair_frame(device, templat));
            LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

void GatewayManage::controlFrameModbusTCP(int gateway_id, const std::string& value, const iot_device& t_device, const iot_template& templat)
{

}

void GatewayManage::CreateModbusRTUFactory(const iot_gateway& gateway)
{
    FactoryPtr modbusRtuFactory = std::make_shared<ModbusRtuFactory>(loop_);
    // modbusRtuFactory = std::make_shared<ModbusRtuFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<ModbusRtuMediator>(loop_, gateway, poolPtr_, modbusRtuFactory);

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    // recvAnalysePtr->setAnalyseFinishCallback(std::bind(&Mediator::onAnalyseFinish, mediatorPtr_, std::placeholders::_1));
    mediatorPtr->start();
}

void GatewayManage::CreateModbusTCPFactory(const iot_gateway& gateway)
{
    FactoryPtr modbusTcpFactory = std::make_shared<ModbusTcpFactory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<ModbusTcpMediator>(loop_, gateway, poolPtr_, modbusTcpFactory);
    
    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    
    mediatorPtr->start();
}

void GatewayManage::CreateDLT645Factory(const iot_gateway& gateway)
{
    FactoryPtr dlt645Factory = std::make_shared<Dlt645Factory>(loop_);
    MediatorPtr mediatorPtr = std::make_shared<Dlt645Mediator>(loop_, gateway, poolPtr_, dlt645Factory);

    controlmediator tmp(gateway.gateway_id, mediatorPtr);
    v_controlmediator_.emplace_back(tmp);
    
    mediatorPtr->start();
}

void GatewayManage::CreateBUS188Factory(const iot_gateway& gateway)
{

}

void GatewayManage::CreateIEC104Factory(const iot_gateway& gateway)
{

}

void GatewayManage::CreateBAcnetIPFactory(const iot_gateway& gateway)
{

}

void GatewayManage::CreateOPCUAFactory(const iot_gateway& gateway)
{

}

void GatewayManage::start()
{
    LOG_INFO("GatewayManage  v_gateway_.size = %d \n", v_gateway_.size());
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

void GatewayManage::sendDataTimer()
{
    poolPtr_->run(std::bind(&GatewayManage::sendDataTD, this));
    if(sendFinish_ != 0)
    {
        LOG_INFO("sendFinish_ != 0  Sending is not complete....");
        return;
    }
    sendFinish_ = 1;
    poolPtr_->run(std::bind(&GatewayManage::sendDataRD, this));
}

void GatewayManage::sendDataRD()
{
    while (!queue_iotdataRD_.empty())
    {
        iot_data_item item;
        {
            std::unique_lock<std::mutex> lock(RD_mutex_);
            item = queue_iotdataRD_.front();
            queue_iotdataRD_.pop();
        }
        std::string jsonStr = itemToJson("RD", item);
        observerPtr_->publicTopic(jsonStr);
        // LOG_INFO("MQTT publish RD size = %d", jsonStr.size());
    }
    sendFinish_--;
}

void GatewayManage::sendDataTD()
{
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
            if (++count >= 10)
            {
                std::string jsonStr = itemToJson("TD",v_item);
                observerPtr_->publicTopic(jsonStr);
                // LOG_INFO("MQTT publish TD size = %d", jsonStr.size());
                v_item.clear();
                count = 0;
            }
        }
        if(!v_item.empty())
        {
            std::string jsonStr = itemToJson("TD",v_item);
            observerPtr_->publicTopic(jsonStr);
            // LOG_INFO("MQTT publish TD size = %d", jsonStr.size());
        }
    }
}