#include "IEC104Analyse.h"

IEC104Analyse::IEC104Analyse() : result_(ENUM_Normal)
{

}

IEC104Analyse::~IEC104Analyse()
{

}

void IEC104Analyse::AnalyseFunc(const std::string &msg, const nextFrame &nextframe)
{
    v_data.insert(v_data.end(), msg.begin(), msg.end());
    if (IEC104AnalyseFrame_Minsize > v_data.size())
    {
        return;
    }

    bool resOK = false; // 解析是否成功
    enum_RW resRW;
    result_ = ENUM_Normal;
    int IframeCount = 0;
    std::pair<int, IEC104FrameType> frameType(0, ENUM_Normal_Frame);
    iot_device device = nextframe.second.first;
    std::vector<iot_template> v_templat = nextframe.second.second;

    if(v_templat.size() < 0)
    {
        LOG_FATAL("IEC104Analyse::AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);

    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            std::vector<frame> framelist;
            if(!FrameListCount(v_data, framelist))
            {
                return;
            }
            for (int i = 0; i < framelist.size(); i++)
            {
                int index = 0;
                frame tmp = framelist.at(i);

                IEC104FrameType type = FrameType(tmp.at(2));
                index += 2;
                switch (type)
                {
                    case ENUM_U_Frame:
                    {
                        frameType.first = 0;
                        frameType.second = ENUM_U_Frame;
                        Analyse_U_Frame(tmp.at(2));
                        resOK = true;
                        break;
                    }
                    case ENUM_I_Frame:
                    {
                        IframeCount++;
                        frameType.first = IframeCount;
                        frameType.second = ENUM_I_Frame;

                        u_int16_t tx_sn;
                        frame tx_frame;
                        ReverseOrder(frame(tmp.begin() + index, tmp.begin() + index + 2), tx_frame); // tx倒序
                        char2or4Hex_To_uint16or32(tx_frame, tx_sn);
                        index += 2;

                        u_int16_t rx_sn;
                        frame rx_frame;
                        ReverseOrder(frame(tmp.begin() + index, tmp.begin() + index + 2), rx_frame); // rx倒序
                        char2or4Hex_To_uint16or32(rx_frame, rx_sn);
                        index += 2;
                        tx_sn >>= 1;
                        rx_sn >>= 1;
                        LOG_INFO("TX_SN=%d, %d, RX_SN=%d, %d", TX_SN_, rx_sn, RX_SN_, tx_sn);
                        if(TX_SN_ != rx_sn || RX_SN_ != tx_sn)
                        {
                            // 出错  重启tcp
                            LOG_INFO("---------------reboot------------");
                            result_ = ENUM_RebootSocket;
                        }
                        IncreaseRX(); // RX_SN++

                        // 结果集
                        std::vector<std::pair<int, std::string>> v_addrvalue;
                        v_addrvalue = AnalyseTypeIdentify(tmp, index); // 类型标识
                        LOG_INFO("v_addrvalue.size = %d", v_addrvalue.size());
                        if (v_addrvalue.size() > 0)
                        {
                            for (auto it = v_addrvalue.begin(); it != v_addrvalue.end(); it++)
                            {
                                for (auto t_templat = templat.v_sub_template.begin(); t_templat != templat.v_sub_template.end(); t_templat++)
                                {
                                    // LOG_INFO("it->first = %d, t_templat->s_addr = %d", it->first, t_templat->s_addr);
                                    if (it->first == t_templat->s_addr)
                                    {
                                        QueueData(t_templat->send_type, setItem(device.gateway_id, device.device_id, device.device_addr, t_templat->param_id, t_templat->param_name, it->second));
                                        break;
                                    }
                                }
                            }
                        }
                            resOK = true;
                        break;
                    }
                    case ENUM_S_Frame:
                    {
                        frameType.first = 0;
                        frameType.second = ENUM_S_Frame;
                        Analyse_S_Frame();
                        resOK = true;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("IEC104Analyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("IEC104Analyse::Analyse  error other...");
            v_data.clear();
            resOK = false;
        }
    }
    else if(templat.rw == enum_write)
    {
        resRW = enum_write;
    }

    v_data.clear();
    if (analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, result_,frameType.first, frameType.second);
    }
}

bool IEC104Analyse::FrameListCount(frame& data, std::vector<frame>& framelist)
{
    bool res = true;
    int index = 0;

    while(index < data.size())
    {
        if(data.at(index) != 0x68)
        {
            LOG_INFO("frame head != 0x68....");
            data.clear();
            framelist.clear();
            return false;
        }
        int len = (u_int8_t)data.at(index + 1) + 2; // 一帧数据长度
        LOG_INFO("len = %d, data.size = %d", len, (int)data.size());

        if (len + index > data.size())
        {
            // 没接收完整
            LOG_INFO("接收不完整....");
            return false;
        }
        frame tmp(data.begin() + index, data.begin() + index + len);
        framelist.emplace_back(tmp);
        index += len;
        LOG_INFO("IEC104 frame++  framelist.count = %d",framelist.size());
    }
    return res;
}

IEC104FrameType IEC104Analyse::FrameType(const char type)
{
    if((type & 0x03) == 0x03) // 控制域1的 bit0=1 并且 bit1=1
    {
        return ENUM_U_Frame;
    }
    else if((~type & 0x01) == 0x01) // 控制域1的 bit0=0
    {
        return ENUM_I_Frame;
    }
    else if((type & 0x01) == 0x01) // 控制域1的 bit0=1 并且 bit1=0
    {
        return ENUM_S_Frame;
    }
    return ENUM_Normal_Frame;
}

void IEC104Analyse::Analyse_U_Frame(const char ctr)
{
    switch(ctr)
    {
        case 0x0B:
        {
            LOG_INFO("U帧 启动确认帧...");
            // 总召唤
            result_ = ENUM_SendFirst_I_Frame;
            break;
        }
        case 0x13:
        {
            LOG_INFO("U帧 终止生效帧...");
            // 
            break;
        }
        case 0x23:
        {
            LOG_INFO("U帧 终止生效确认帧...");
            break;
        }
        case 0x43:
        {
            LOG_INFO("U帧 链路测试帧...");
            // 发送链路测试确认帧
            result_ = ENUM_Send_U_testRespFrame;
            break;
        }
        case 0x83:
        {
            LOG_INFO("U帧 链路测试确认帧...");
            break;
        }
    }
}

void IEC104Analyse::Analyse_S_Frame()
{

}

std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseTypeIdentify(const frame& data, int& index)
{
    IEC104TypeIdentity typeIdentity = (IEC104TypeIdentity)data.at(index);
    index++;
    return AnalyseVSQ(data, typeIdentity, index);
}

std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseVSQ(const frame& data, IEC104TypeIdentity typeIdentity, int& index)
{
    bool cont = false; // SQ=1: 连续  SQ=0：不连续
    if (((u_int8_t)data.at(index) & 0x80) == 0x80)
    {
        cont = true;
    }
    int objCount = data.at(index) & 0x7F;
    int del = (u_int8_t)data.at(index);
    index++;
    LOG_INFO("%d是否连续:%d, count=%d", del, (int)cont, objCount);
    return AnalyseCOT(data, typeIdentity, cont, objCount, index);
}

// 传送原因 2个字节
std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseCOT(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index)
{
    frame tmp;
    ReverseOrder(frame(data.begin()+index, data.begin()+index+2), tmp);
    index += 2;
    int cot = 0;
    char2or4Hex_To_uint16or32(tmp, cot);
    switch(cot)
    {
        case ENUM_I_COT_0x01: // = 0x01, // 周期、循环
        {
            LOG_INFO("传送原因cot=0x01 周期、循环");
            return AnalyseRTU(data, typeIdentity, cont, objCount, index);
            // break;
        }
        case ENUM_I_COT_0x02: //  = 0x02, // 双点遥控
        {
            LOG_INFO("传送原因cot=0x02 双点遥控");
            break;
        }
        case ENUM_I_COT_0x03: //  = 0x03, // 突变
        {
            LOG_INFO("传送原因cot=0x03 突变");
            return AnalyseRTU(data, typeIdentity, cont, objCount, index);
            // break;
        }
        case ENUM_I_COT_0x04: //  = 0x04, // 初始化
        {
            LOG_INFO("传送原因cot=0x04 初始化");
            break;
        }
        case ENUM_I_COT_0x05: //  = 0x05, // 请求或被请求
        {
            LOG_INFO("传送原因cot=0x05 请求或被请求");
            break;
        }
        case ENUM_I_COT_0x06: //  = 0x06, // 激活
        {
            LOG_INFO("传送原因cot=0x06 激活");
            break;
        }
        case ENUM_I_COT_0x07: //  = 0x07, // 激活确认
        {
            LOG_INFO("传送原因cot=0x07 激活确认");

            frame tmp;
            ReverseOrder(frame(data.begin()+index, data.begin()+index+2), tmp);
            int rtu = 0;
            char2or4Hex_To_uint16or32(tmp, rtu);
            LOG_INFO("RTU = %d", rtu);
            std::vector<std::pair<int, std::string>> v_addrvalue;
            return v_addrvalue;
            // break;
        }
        case ENUM_I_COT_0x08: //  = 0x08, // 停止激活
        {
            LOG_INFO("传送原因cot=0x08 停止激活");
            break;
        }
        case ENUM_I_COT_0x09: //  = 0x09, // 停止激活确认
        {
            LOG_INFO("传送原因cot=0x09 停止激活确认");
            break;
        }
        case ENUM_I_COT_0x0A: //  = 0x0A, // 激活结束
        {
            LOG_INFO("传送原因cot=0x0A 激活结束");
            frame tmp;
            ReverseOrder(frame(data.begin()+index, data.begin()+index+2), tmp);
            int rtu = 0;
            char2or4Hex_To_uint16or32(tmp, rtu);
            LOG_INFO("RTU = %d", rtu);

            result_ = ENUM_SendNext_I_Frame; // 当前如果是遥测，发送遥脉召唤帧（支持的话）；当前是遥脉，等待下一次轮询
            LOG_INFO("------OVER-----");

            std::vector<std::pair<int, std::string>> v_addrvalue;
            return v_addrvalue;
            // break;
        }
        case ENUM_I_COT_0x14: //  = 0x14  // 响应总召唤
        {
            LOG_INFO("传送原因cot=0x14 响应总召唤");
            return AnalyseRTU(data, typeIdentity, cont, objCount, index);
            // break;
        }
        case ENUM_I_COT_0x25: // 遥脉
        {
            return AnalyseRTU(data, typeIdentity, cont, objCount, index);
            // break;
        }
        default:
            break;
    }
}

std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseRTU(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index)
{
    frame tmp;
    ReverseOrder(frame(data.begin()+index, data.begin()+index+2), tmp);
    index += 2;

    int rtu = 0;
    char2or4Hex_To_uint16or32(tmp, rtu);
    LOG_INFO("RTU = %d", rtu);
    return AnalyseBlock(data, typeIdentity, cont, objCount, index);
}

std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseBlock(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index)
{
    switch(typeIdentity)
    {
    // 遥信
        case ENUM_I_TypeIdentity_01: // = 0x01, // 不带时标的单点遥信， 每个遥信占一个字节
        {
            return AnalyseItem_01(data, cont, objCount, index);
        }
        case ENUM_I_TypeIdentity_03: // = 0x03, // 不带时标的双点遥信， 每个遥信占一个字节
        {
            break;
        }
        case ENUM_I_TypeIdentity_14: // = 0x14, // 具有状态变位检出的成组单点遥信，每个字节8个遥信
        {
            break;
        }
    //遥测
        case ENUM_I_TypeIdentity_09: // = 0x09, // 带品质描述的测量值， 每个遥测值占3个字节
        {
            return AnalyseItem_09(data, cont, objCount, index);
            // break;
        }
        case ENUM_I_TypeIdentity_0A: // = 0x0A, // 带3个字节时标的且具有品质描述的测量值，每个遥测值占6个字节
        {
            break;
        }
        case ENUM_I_TypeIdentity_0B: // = 0x0B, // 不带时标的标准化值，每个遥测值占3个字节
        {
            return AnalyseItem_0B(data, cont, objCount, index);
            // break;
        }
        case ENUM_I_TypeIdentity_0C: // = 0x0C, // 带3个时标的标准化值，每个遥测值占6个字节
        {
            break;
        }
        case ENUM_I_TypeIdentity_0D: // = 0x0D, // 带品质描述的浮点值，每个遥测值占5个字节
        {
            return AnalyseItem_0D(data, cont, objCount, index);
            // break;
        }
        case ENUM_I_TypeIdentity_0E: // = 0x0E, // 带三个字节时标且具有品质描述的浮点值，每个遥测值占8个字节
        {
            break;
        }
        case ENUM_I_TypeIdentity_15: // = 0x15, // 不带品质描述的遥测值，每个遥测值占一个字节
        {
            break;
        }
    // 遥脉
        case ENUM_I_TypeIdentity_0F: // = 0x0F, // 不带时标的电能量，每个电能量占5个字节
        {
            return AnalyseItem_0F(data, cont, objCount, index);
            // break;
        }
        case ENUM_I_TypeIdentity_10: // = 0x10, // 带三个字节短时标的电能量，每个电能量占8个字节
        {
            break;
        }
        case ENUM_I_TypeIdentity_25: // = 0x25, // 带七个字节短时标的电能量，每个电能量占12个字节
        {
            break;
        }
    // SOE
        case ENUM_I_TypeIdentity_02: // = 0x02, // 带3个字节短时标的单点遥信
        {
            break;
        }
        case ENUM_I_TypeIdentity_04: // = 0x04, // 带3个字节短时标的双点遥信
        {
            break;
        }
        case ENUM_I_TypeIdentity_1E: // = 0x1E, // 带7个字节短时标的单点遥信
        {
            break;
        }
        case ENUM_I_TypeIdentity_1F: // = 0x1F, // 带7个字节短时标的双点遥信
        {
            break;
        }
    // 其他
        case ENUM_I_TypeIdentity_2E: // = 0x2E, // 双点遥控
        {
            break;
        }
        case ENUM_I_TypeIdentity_2F: // = 0x2F, // 双点遥调
        {
            break;
        }
        case ENUM_I_TypeIdentity_64: // = 0x64, // 召唤全数据
        {
            break;
        }
        case ENUM_I_TypeIdentity_65: // = 0x65, // 召唤全电度
        {
            break;
        }
        case ENUM_I_TypeIdentity_67: // = 0x67  // 时钟同步
        {
            break;
        }
    }
}

void IEC104Analyse::AnalyseAddr(const frame& data, int& index, int& addr)
{
    frame tmp;
    ReverseOrder(frame(data.begin()+index, data.begin()+index+3), tmp); // 倒序
    char2or4Hex_To_uint16or32(tmp, addr);
}

void IEC104Analyse::AnalyseData_09(const frame& data,  int& index, int& value)
{
    frame tmp;
    ReverseOrder(frame(data.begin()+index, data.begin()+index+2), tmp);

    char2or4Hex_To_uint16or32(tmp, value);
}

void IEC104Analyse::AnalyseData_0D(const frame& data,  int& index, float& value)
{
    frame tmp;
    ReverseOrder(frame(data.begin() + index, data.begin() + index + 4), tmp);
    u_int32_t u32_data;
    char2or4Hex_To_uint16or32(tmp, u32_data);
    IEEE754_To_float(u32_data, value);

    printFrame("data=               ", tmp);
}

void IEC104Analyse::AnalyseData_0F(const frame &data, int &index, int &value)
{
    frame tmp;
    ReverseOrder(frame(data.begin() + index, data.begin() + index + 4), tmp);
    char2or4Hex_To_uint16or32(tmp, value);
    
    printFrame("data=               ", tmp);
}

void IEC104Analyse::AnalyseQDS(const frame& data, int& index) // 品质描述符
{
    int IV = 0; // 有效标志位(0有效，1无效)
    int NT = 0; // 刷新标志位(0刷新成功，1刷新未成功)
    if((data.at(index) & 0x80) == 0x80)
    {
        IV = 1;
    }
    if((data.at(index) & 0x40) == 0x40)
    {
        NT = 1;
    }
    // LOG_INFO("品质描述符 IV=%d, NT=%d", IV, NT);
}

std::vector<std::pair<int, std::string>> IEC104Analyse::AnalyseItem_01(const frame& data, bool cont, int objCount, int& index)
{
    std::vector<std::pair<int, std::string>> v_addrvalue;
    if (cont) // 连续 (地址，数据；数据；数据;...)
    {
        int addr = 0;
        AnalyseAddr(data, index, addr); // 地址
        index += 3;
        int number = 0;
        while (number < objCount)
        {
            int value = data.at(index);
            index++;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));
            // addr += number;
            number++;
        }
    }
    else // 不连续
    {
        int number = 0;
        while (number < objCount)
        {
            int addr = 0;
            AnalyseAddr(data, index, addr); // 地址
            index += 3;

            int value = data.at(index);
            index++;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));
            // addr += number;
            number++;
        }
    }
    return v_addrvalue;
}

std::vector<std::pair<int, std::string>>  IEC104Analyse::AnalyseItem_09(const frame& data, bool cont, int objCount, int& index)
{
    std::vector<std::pair<int, std::string>> v_addrvalue;
    if (cont) // 连续 (地址，数据；数据；数据;...)
    {
        int addr = 0;
        AnalyseAddr(data, index, addr); // 地址
        index += 3;

        int number = 0;
        while (number < objCount)
        {
            int value = 0;
            AnalyseData_09(data, index, value);
            index += 2;

            AnalyseQDS(data, index);
            index++;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));

            number++;
            // addr += number;
        }
    }
    else
    {
        int number = 0;
        while (number < objCount)
        {
            int addr = 0;
            AnalyseAddr(data, index, addr); // 地址
            index += 3;

            int value = 0;
            AnalyseData_09(data, index, value);
            index += 2;

            AnalyseQDS(data, index);
            index++;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));

            number++;
            // addr += number;
        }
    }
    return v_addrvalue;
}

std::vector<std::pair<int, std::string>>  IEC104Analyse::AnalyseItem_0B(const frame& data, bool cont, int objCount, int& index)
{
    std::vector<std::pair<int, std::string>> v_addrvalue;
    return v_addrvalue;
}

std::vector<std::pair<int, std::string>>  IEC104Analyse::AnalyseItem_0D(const frame& data, bool cont, int objCount, int& index)
{
    std::vector<std::pair<int, std::string>> v_addrvalue;
    if (cont) // 连续 (地址，数据；数据；数据;...)
    {
        int addr = 0;
        AnalyseAddr(data, index, addr); // 地址
        index += 3;

        int number = 0;
        while (number < objCount)
        {
            // LOG_INFO("连续  index=%d, number=%d", index, number);
            // 短浮点数
            float value;
            AnalyseData_0D(data, index, value); // 数据
            index += 4;
            AnalyseQDS(data, index); // 品质描述符
            index += 1;

            std::string str_value = floatToString(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));
            
            number++;
            // addr += number;
        }
    }
    else // 不连续 (地址，数据；地址，数据；地址，数据;...)
    {
        int number = 0;
        while (number < objCount)
        {
            // LOG_INFO("不连续  index=%d, number=%d", index, number);
            int addr = 0;
            AnalyseAddr(data, index, addr); // 地址
            index += 3;
            
            // 短浮点数
            float value;
            AnalyseData_0D(data, index, value); // 数据
            index += 4;
            AnalyseQDS(data, index); // 品质描述符
            index += 1;

            std::string str_value = floatToString(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));

            number++;
            // addr += number;
        }
    }
    return v_addrvalue;
}

std::vector<std::pair<int, std::string>>  IEC104Analyse::AnalyseItem_0F(const frame& data, bool cont, int objCount, int& index)
{
    std::vector<std::pair<int, std::string>> v_addrvalue;
    if (cont) // 连续 (地址，数据；数据；数据;...)
    {
        int addr = 0;
        AnalyseAddr(data, index, addr); // 地址
        index += 3;

        int number = 0;
        while (number < objCount)
        {
            int value;
            AnalyseData_0F(data, index, value); // 数据
            index += 4;
            AnalyseQDS(data, index); // 描述信息
            index += 1;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));

            number++;
            // addr += number;
            // break;
        }
    }
    else // 不连续 (地址，数据；地址，数据；地址，数据;...)
    {
        int number = 0;
        while (number < objCount)
        {
            int addr = 0;
            AnalyseAddr(data, index, addr); // 地址
            index += 3;
            
            int value;
            AnalyseData_0F(data, index, value); // 数据
            index += 4;
            AnalyseQDS(data, index); // 描述信息
            index += 1;

            std::string str_value = std::to_string(value);
            LOG_INFO("addr=%d, data=%s", addr + number, str_value.c_str());
            v_addrvalue.emplace_back(std::pair<int, std::string>(addr + number, str_value));

            number++;
            // addr += number;
            // break;
        }
    }
    return v_addrvalue;
}