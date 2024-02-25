#include "Dlt645Analyse.h"
#include <algorithm>

Dlt645Analyse::Dlt645Analyse()
{

}
Dlt645Analyse::~Dlt645Analyse()
{

}

void Dlt645Analyse::AnalyseFunc(const std::string &msg, const nextFrame &nextframe)
{
    v_data.insert(v_data.end(), msg.begin(), msg.end());
    if (DLT645AnalyseFrame_Minsize > v_data.size())
    {
        return;
    }

    for(int i=0; i<4; i++)// 前四个字节有可能是0xFE，删除
    {
        auto it = v_data.begin();
        if((*it)& 0xFE == 0xFE)
        {
            v_data.erase(it);
        }
    }

    bool resOK = false; // 解析是否成功
    enum_RW resRW;
    int index = 0;
    iot_device device = nextframe.second.first;
    std::vector<iot_template> v_templat = nextframe.second.second;

    if(v_templat.size() < 0)
    {
        LOG_FATAL("Dlt645Analyse::AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);

    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            auto it = v_data.begin();
            if(!DLTCheckHead(v_data, index))
            {
                LOG_ERROR("Dlt645Analyse::AnalyseFunc DLTCheckHead error...");
            }
            if(DLTCheckLength(v_data, v_data.at(9)))      // 整体长度校验
            {
                return; // 数据接收不全
            }
            // 校验和
            if(!DLT645DataCheck(frame(it, it + v_data.size()-2), v_data.at(v_data.size()-2)))
            {
                LOG_ERROR("Dlt645Analyse::AnalyseFunc DLT645DataCheck error...");
                v_data.clear();
            }
            index++;
            if(!DLT645MeteridCheck(frame(it + index, it + index + 6), device.device_addr))
            {
                LOG_ERROR("Dlt645Analyse::AnalyseFunc meter error...");
            }
            index += 7;
            switch(v_data.at(index++))
            {
                // 功能：请求读电能表数据 控制码：C=11H
                case 0x91:  // 从站正常应答, 无后续数据帧
                {
                    u_int8_t datalength = v_data.at(index);
                    index++;
                    std::string res = DLTAnalyseData(frame(it + index, it + index + datalength), templat);
                    // LOG_INFO("Dlt645Analyse %s = %s", templat.param_name.c_str(), res.c_str());
                    QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, res));
                    resOK = true;
                }
                    break;
                case 0xB1:  // 从站正常应答, 有后续数据帧
                {
                    LOG_ERROR("Dlt645Analyse errorID = 0XB1");
                }
                    break;
                case 0xD1:  // 从站异常应答
                    break;
                // 功能：请求读后续数据   控制码：C=12H
                case 0x92:  // 从站正常应答, 无后续数据帧
                    break;
                case 0xB2:  // 从站正常应答, 有后续数据帧
                    break;
                case 0xD2:  // 从站异常应答
                    break;
                // 功能：主站向从站请求设置数据(或编程) 控制码：C=14H
                case 0x94:  // 写成功
                    break;
                case 0xD4:  // 写失败
                    break;
                // 功能：请求读电能表通信地址，仅支持点对点通信  控制码：C=13H
                case 0x93:  // 从站正常应答帧
                    break;
                // 功能：设置某从站的通信地址，仅支持点对点通信  控制码：C=15H
                case 0x95:  // 从站正常应答帧
                    break;
                default:
                    break;
                }
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("Dlt645Analyse out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("Dlt645Analyse::Analyse error other...");
            v_data.clear();
            resOK = false;
        }
        v_data.clear();
    }
    else if(templat.rw == enum_write)
    {
        resRW = enum_write;
        int index = 0;
        auto it = v_data.begin();
        if(!DLTCheckHead(v_data, index))
        {
            LOG_ERROR("Dlt645Analyse::AnalyseFunc DLTCheckHead error...");
        }
        if(DLTCheckLength(v_data, v_data.at(9)))      // 整体长度校验
        {
            return; // 数据接收不全
        }
        if(!DLT645DataCheck(frame(it, it + v_data.size()-2), v_data.at(v_data.size()-2)))
        {
            LOG_ERROR("Dlt645Analyse::AnalyseFunc DLT645DataCheck error...");
            v_data.clear();
        }
        switch(v_data.at(8))
        {
            // 功能：主站向从站请求设置数据(或编程) 控制码：C=14H
            case 0x94:  // 写成功
                resOK = true;
                break;
            case 0xD4:  // 写失败
                resOK = false;
                break;
        }
        v_data.clear();
    }    
    // 解析完成
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame/*std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame)*/);
    }
}

std::string Dlt645Analyse::DLTAnalyseData(const frame& src, const iot_template& templat)
{
    if(src.size() == 8)
    {
        // 数据项没有对比
        frame tmp(src.begin() + 4, src.begin() + 8);
        HandleByte_order(tmp, templat.byte_order);
        // Dlt -0x33
        for (auto it = tmp.begin(); it != tmp.end(); it++)
        {
            *it -= 0x33;
        }
        std::string res = HandleData_type(tmp, templat.data_type, templat.correct_mode);
        return res;
    }
    else
    {
        LOG_ERROR("Dlt645Analyse::DLTAnalyseData src.size() != 8 , %d", (int)src.size());
    }
}

bool Dlt645Analyse::DLTCheckHead(const frame& src, const int& index)
{
    if ((src.at(index) & 0x68) == 0x68          // 起始帧 0x68
        && src.at(index + 7) & 0x68 == 0x68)     // 起始帧 0x68
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Dlt645Analyse::DLTCheckLength(const frame& src, const char& length)
{
    if(src.size() < (u_int8_t)length + 12)
    {
        return false;
    }
    return true;
}

bool Dlt645Analyse::DLT645DataCheck(const frame& src, const char& check)
{
    return check == DLTCheck(src);
}

bool Dlt645Analyse::DLT645MeteridCheck(const frame& hexMeter, const std::string& strMeter)
{
    frame tmp = HexStrToByteArray(strMeter);
    return hexMeter == tmp;
}