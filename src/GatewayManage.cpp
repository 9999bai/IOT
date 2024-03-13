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
    observerPtr_ = std::make_shared<MqttClient>(mqttconf);
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
        // bool sub_status = false; // 子模板是否有数据

        if(findProname(item.gateway_id, pro_name)) // 查找确定协议类型
        {
            LOG_INFO("proname = %d", (int)pro_name);

            switch (pro_name)
            {
                case enum_pro_name_ModbusRTU:
                {
                    if(findModbusParam(item, device, templat, sub_templat))
                    {
                        templat.rw = enum_write; // 写
                        // templat.w_func = sub_templat.w_func; // 解析时需要，通过返回语句判断是否写成功
                        controlFrameModbusRTU(item.gateway_id, item.value, device, templat, sub_templat);
                    }else{
                        LOG_ERROR("ModbusRTU findModbusParam error...");
                    }
                }
                    break;
                case enum_pro_name_ModbusTCP:
                {
                    if(findModbusParam(item, device, templat, sub_templat))
                    {
                        templat.rw = enum_write; // 写
                        controlFrameModbusTCP(item.gateway_id, item.value, device, templat, sub_templat);
                    }else{
                        LOG_ERROR("ModbusTCP findModbusParam error...");
                    }
                }
                    break;
                case enum_pro_name_DLT645:
                    break;
                case enum_pro_name_CJT188:
                    break;
                case enum_pro_name_IEC104:
                    break;
                case enum_pro_name_BAcnetIP:
                {
                    LOG_INFO("control enum_pro_name_BAcnetIP...");
                    templat.rw = enum_write; // 写
                    controlFrameBacnetIP(item.gateway_id, item.value, device, templat);
                }
                    break;
                case enum_pro_name_OPCUA:
                    break;
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

bool GatewayManage::findModbusParam(const iot_data_item &item, iot_device &dest_device, iot_template& dest_templat, iot_sub_template &dest_subtemplat)
{
    for(auto t_gateway : v_gateway_)
    {
        if(item.gateway_id == t_gateway.gateway_id)
        {
            for(auto t_device : t_gateway.v_device)
            {
                if(item.device_id == t_device.device_id)
                {
                    for(auto t_templat : t_device.v_template)
                    {
                        for(auto t_subtemplat : t_templat.v_sub_template)
                        {
                            if(item.param_id == t_subtemplat.param_id)
                            {
                                dest_device = t_device;
                                dest_templat = t_templat;
                                dest_subtemplat = t_subtemplat;
                                LOG_INFO("find result --- device=%d,templat=%d,sub_templat=%d,paramid=%d,wfunc=%d",\
                                dest_device.device_id,dest_templat.template_id,dest_subtemplat.sub_template_id,\
                                dest_subtemplat.param_id,(int)dest_subtemplat.w_func);
                                return true;
                            }
                        }
                    }
                    return false;
                }
            }
        }
    }

}

bool GatewayManage::findControlParam(const iot_data_item& item, enum_pro_name& pro_name, iot_device& dest_device, iot_template& dest_templat, iot_sub_template& dest_subtemplat)
{
    for(auto gateway : v_gateway_)
    {
        if(item.gateway_id == gateway.gateway_id)
        {
            for(auto device : gateway.v_device)
            {
                if(item.device_id == device.device_id)
                {
                    for(auto templat : device.v_template)
                    {
                        if(0 == templat.param_id && 0 != templat.sub_template_id)
                        {
                            for(auto subtemplat : templat.v_sub_template)
                            {
                                if(item.param_id == subtemplat.param_id)
                                {
                                    pro_name = gateway.pro_name;
                                    dest_device = device;
                                    // dest_templat = templat;
                                    // sub_status = true;
                                    dest_subtemplat = subtemplat;
                                    dest_templat.rw = enum_write;
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            if(item.param_id == templat.param_id)
                            {
                                pro_name = gateway.pro_name;
                                dest_device = device;
                                dest_templat = templat;
                                dest_templat.rw = enum_write;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

void GatewayManage::WriteBytetypeData(enum_byte_order byteorder, const frame& td, frame& t_frame)
{
    switch(byteorder)
    {
        case enum_byte_order_A:
        {
            if(td.size()==1)
            {
                t_frame.emplace_back(td.at(0));
            }
        }
            break;
        case enum_byte_order_AB:
        {
            if(td.size()==2)
            {
                t_frame.emplace_back(td.at(0));
                t_frame.emplace_back(td.at(1));
            }
        }
            break;
        case enum_byte_order_BA:
        {
            if (td.size() == 2)
            {
                t_frame.emplace_back(td.at(1));
                t_frame.emplace_back(td.at(0));
            }
        }
            break;
        case enum_byte_order_AB_CD:
        {
            if(td.size()==4)
            {
                t_frame.emplace_back(td.at(0));
                t_frame.emplace_back(td.at(1));
                t_frame.emplace_back(td.at(2));
                t_frame.emplace_back(td.at(3));
            }
        }
            break;
        case enum_byte_order_CD_AB:
        {
            if(td.size()==4)
            {
                t_frame.emplace_back(td.at(2));
                t_frame.emplace_back(td.at(3));
                t_frame.emplace_back(td.at(0));
                t_frame.emplace_back(td.at(1));
            }
        }
            break;
        case enum_byte_order_BA_DC:
        {
            if(td.size()==4)
            {
                t_frame.emplace_back(td.at(1));
                t_frame.emplace_back(td.at(0));
                t_frame.emplace_back(td.at(3));
                t_frame.emplace_back(td.at(2));
            }
        }
            break;
        case enum_byte_order_DC_BA:
        {
            if(td.size()==4)
            {
                t_frame.emplace_back(td.at(3));
                t_frame.emplace_back(td.at(2));
                t_frame.emplace_back(td.at(1));
                t_frame.emplace_back(td.at(0));
            }
        }
            break;
    }
}

void GatewayManage::controlFrameModbusRTU(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const iot_sub_template& sub_templat)
{
    frame t_frame;
    t_frame.emplace_back(atoi(device.device_addr.c_str()));
    t_frame.emplace_back((int)sub_templat.w_func);

    switch((int)sub_templat.w_func)
    {
        case enum_w_func_0x05:
        {
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            if(value == "0") // OFF
            {
                uint16_To_char2(0, t_frame);
            }
            else            // ON
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
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_frame);
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]); // crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        case enum_w_func_0x0F:
        {

        }
            break;
        case enum_w_func_0x10: // 这里只写一个寄存器
        {
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2(2, t_frame); // 寄存器个数
            t_frame.emplace_back(0x04);  // 字节数
            switch (sub_templat.data_type)
            {
                case enum_data_type_int16_hex:
                {
                    frame td;
                    union_uint16TOuchar data;
                    data.u16_data = std::atoi(value.c_str());
                    td.emplace_back(data.uchar_data[0]);
                    td.emplace_back(data.uchar_data[1]);
                    
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                    // uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_data);
                }
                    break;
                case enum_data_type_int32_hex:
                {
                    frame td;
                    // u_int32_t u32data;
                    union_uchar4TOuint32 data;
                    data.u32_data = std::atoi(value.c_str());
                    // 大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    // uint32_To_char4((u_int32_t)std::atoi(value.c_str()), t_frame);
                    break;
                case enum_data_type_float:
                {
                    frame td;
                    u_int32_t u32data;
                    float_To_IEEE754(std::atof(value.c_str()), u32data);
                    union_uchar4TOuint32 data;
	                data.u32_data = u32data;
                    //大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                    // BigEndianStrToU32Bit();
                    // LittleEndianStrToU32Bit();
                }
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
            LOG_ERROR("GatewayManage::controlFrameModbusRTU error... w_func = %d ", (int)sub_templat.w_func);
            break;
    }
    printFrame("Control --------------------- ", t_frame);
    for (auto it = v_controlmediator_.begin(); it != v_controlmediator_.end(); it++)
    {
        if(gateway_id == it->first)
        {
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(t_frame, pair_frame(device, v_templat));
            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

void GatewayManage::controlFrameModbusTCP(int gateway_id, const std::string& value, const iot_device& t_device, iot_template& templat, const iot_sub_template& sub_templat)
{
    frame t_frame;
    static int identity = 0;
    if(identity >= 0xFF)
        identity = 0;

    templat.other = std::to_string(identity);
    uint16_To_char2(identity++, t_frame);        // 报文序列号
    uint16_To_char2(ModbusTcpIdentity, t_frame); // modbustcp标志
    
    switch((int)sub_templat.w_func)
    {
        case enum_w_func_0x05:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x06);
            t_frame.emplace_back((u_int8_t)std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            if(value == "0") // OFF
            {
                t_frame.emplace_back(0x00);
                t_frame.emplace_back(0x00);
                // uint16_To_char2(0, t_frame);
            }
            else // ON
            {
                t_frame.emplace_back(0xFF);
                t_frame.emplace_back(0x00);
                // uint16_To_char2(0xFF, t_frame);
            }
        }
            break;
        case enum_w_func_0x06:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x06);
            t_frame.emplace_back((u_int8_t)std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2(std::atoi(value.c_str()), t_frame);
        }
            break;
        case enum_w_func_0x0F:
        {
            
        }
            break;
        case enum_w_func_0x10:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x0B);
            t_frame.emplace_back(std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            
            switch (sub_templat.data_type)
            {
                case enum_data_type_int16_hex:
                {
                    uint16_To_char2(1, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x02);  // 字节数
                    frame td;
                    union_uint16TOuchar data;
                    data.u16_data = std::atoi(value.c_str());
                    td.emplace_back(data.uchar_data[0]);
                    td.emplace_back(data.uchar_data[1]);
                    
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    break;
                case enum_data_type_int32_hex:
                {
                    uint16_To_char2(2, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x04);  // 字节数
                    frame td;
                    union_uchar4TOuint32 data;
                    data.u32_data = std::atoi(value.c_str());
                    // 大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    break;
                case enum_data_type_float:
                {
                    uint16_To_char2(2, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x04);  // 字节数
                    frame td;
                    u_int32_t u32data;
                    float_To_IEEE754(std::atof(value.c_str()), u32data);
                    union_uchar4TOuint32 data;
	                data.u32_data = u32data;
                    //大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                    // BigEndianStrToU32Bit();
                    // LittleEndianStrToU32Bit();
                }
                    break;
            }
        }
            break;
        default:
        LOG_ERROR("GatewayManage::controlFrameModbusTCP error... w_func = %d ", (int)sub_templat.w_func);
    }
    printFrame("Control --------------------- ", t_frame);
    for (auto it = v_controlmediator_.begin(); it != v_controlmediator_.end(); it++)
    {
        if(gateway_id == it->first)
        {
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(t_frame, pair_frame(t_device, v_templat));
            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

void GatewayManage::controlFrameBacnetIP(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat)
{
    // write 固定头部
    frame data = HexStrToByteArray("81 0A 00 00 01 04 00 03 64 0F");

    ObjectIdentifier(templat, data);
    int propertyValue = std::atoi(templat.correct_mode.c_str());
    if (propertyValue >0 && propertyValue<= 0xFF)
    {
        data.emplace_back(0x19);
        data.emplace_back(propertyValue);
    }
    else if(propertyValue > 0xFF && propertyValue <= 0xFFFF)
    {
        data.emplace_back(0x1A);
        data.emplace_back(propertyValue >> 8);
        data.emplace_back(propertyValue & 0xFF);
    }

    data.emplace_back(0x3E);
    strToBacnetIPValue(value, templat.register_quantity, data);
    data.emplace_back(0x3F);

    data.emplace_back(0x49);    // write 优先级16 
    data.emplace_back(0x10);

    // 更新数据长度
    u_int16_t length = data.size();
    data[2] = length >> 8;
    data[3] = length & 0xFF;

    printFrame("Bacnetip Control --------------------- ", data);

    for (auto it = v_controlmediator_.begin(); it != v_controlmediator_.end(); it++)
    {
        if(gateway_id == it->first)
        {
            // templat.rw = enum_write;
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(data, pair_frame(device, v_templat));

            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
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

void GatewayManage::strToBacnetIPValue(const std::string& value, int valueType, frame& data)
{
    switch(valueType)
    {
        case 0:     // null
            data.emplace_back(0x00);
            break;
        case 1:     // bool
        {
            int tmp = std::atoi(value.c_str());
            if(tmp == 0)
            {
                data.emplace_back(0x10); // false
            }
            else{
                data.emplace_back(0x11); // true
            }
            break;
        }
        case 2:     // uint
        {
            u_int32_t tmp = std::atoi(value.c_str());
            if(tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x21);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x22);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x23);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }else {
                data.emplace_back(0x24);
                data.emplace_back(tmp >> 24);
                data.emplace_back((tmp >> 16) & 0xFF);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case 3:     // int
        {
            int32_t tmp = std::atoi(value.c_str());
            if(tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x31);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x32);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x33);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }else {
                data.emplace_back(0x34);
                data.emplace_back(tmp >> 24);
                data.emplace_back((tmp >> 16) & 0xFF);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case 4:     // real
        {
            data.emplace_back(0x44);
            float tmp = std::atof(value.c_str());
            u_int32_t res;
            float_To_IEEE754(tmp, res);
            LOG_INFO("real tmp=%2f, u32=%d", tmp, (int)res);

            data.emplace_back(res >> 24);
            data.emplace_back((res >> 16) & 0xFF);
            data.emplace_back((res >> 8) & 0xFF);
            data.emplace_back(res & 0xFF);
            break;
        }
        case 5:     // double
            break;
        case 6:     // Octet String 字节串
            break;
        case 7:     // Character String 字符串
        {

            break;
        }
        case 8:     // Bit String 比特串
            break;
        case 9:     // 枚举
        {
            int tmp = std::atoi(value.c_str());
            if (tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x91);
                data.emplace_back(tmp);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x92);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x93);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case 10:     // date
            break;
        case 11:     // time
            break;
        default:
            break;
    }
}

void GatewayManage::onAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(rw == enum_write)
    {
        if(ok)
        {
            // 写 成功
            LOG_INFO("ModbusRtuMediator 写成功...");
            observerPtr_->publicTopic("21");
        }
        else
        {
            // 写 失败
            LOG_INFO("ModbusRtuMediator 写失败...");
            observerPtr_->publicTopic("20");
        }
    }
    else if(rw == enum_read)
    {
        if(ok)
        {
            // 读 成功
            LOG_INFO("ModbusRtuMediator 读成功...");
            observerPtr_->publicTopic("11");
        }
        else
        {
            // 读 失败
            LOG_INFO("ModbusRtuMediator 读失败...");
            observerPtr_->publicTopic("10");
        }
    }
}
