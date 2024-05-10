#include "Dlt645Analyse.h"
#include <algorithm>

Dlt645Analyse::Dlt645Analyse()
{

}

Dlt645Analyse::~Dlt645Analyse()
{

}

void Dlt645Analyse::AnalyseFunc(const std::string &msg, const nextFrame &nextframe, void* pending)
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
    // iot_template templat = v_templat.at(0);

    auto it = v_data.begin();
    if(!DLTCheckHead(v_data, index))
    {
        LOG_ERROR("Dlt645Analyse::AnalyseFunc DLTCheckHead error...");
    }
    if(v_data.size() < ((int)v_data.at(9) + 12)) // 整体长度校验
    {
        LOG_ERROR("Dlt645Analyse v_data.size=%d, %d...", (int)v_data.size(), ((int)v_data.at(9) + 12));
        return;
    }
    // if (DLTCheckLength(v_data, v_data.at(9))) // 整体长度校验
    // {
    //     return; // 数据接收不全
    // }
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
    int type = v_data.at(index);
    index += 2;
    switch (type)
    {
        case 0x91:  // 从站正常应答, 无后续数据帧
        {
            resRW = enum_read;
            for (auto templat : v_templat)
            {
                try
                {
                    frame item = frame(it + index, it + index + 4);   // 获取数据项
                    for (auto it = item.begin(); it != item.end(); it++) // 数据项 - 0x33
                    {
                        *it -= 0x33;
                    }
                    index += 4;
                    frame tmp = HexStrToByteArray(templat.register_addr);
                    
                    if (tmp == item)
                    {
                        frame data = frame(it + index, it + index + templat.register_quantity);
                        index += templat.register_quantity;
                        std::string res = DLTAnalyseData(data, templat);
                        LOG_INFO("%s = %s", templat.param_name.c_str(), res.c_str());
                        QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, res));
                        resOK = true;
                    }
                    else
                    {
                        LOG_ERROR("Dlt645Analyse itemcode error");
                        resOK = false;
                    }
                }
                catch(std::out_of_range)
                {
                    LOG_ERROR("Dlt645Analyse out_of_range...");
                    resOK = false;
                    break;
                }
                catch(...)
                {
                    LOG_ERROR("Dlt645Analyse::Analyse error other...");
                    resOK = false;
                    break;
                }
            }
        }
            break;
        case 0xD1: // 从站异常应答, 读失败
        {
            resRW = enum_read;
            resOK = false;
            int errCode = v_data.at(10);
            LOG_ERROR("Dlt645Analyse errorID = 0XD1");
        }
            break;
        case 0x94: // 写成功
        {
            resRW = enum_write;
            resOK = true;
            LOG_ERROR("Dlt645Analyse write suc = 0X94");
        }
            break;
        case 0xD4:  // 从站异常应答,写失败
        {
            resRW = enum_write;
            resOK = false;
            int errCode = v_data.at(10);
            LOG_ERROR("Dlt645Analyse write error = 0XD4");
        }
            break;
        default:
        {
            LOG_ERROR("Dlt645Analyse errorID = %d", type);
        }
            break;
    }
    v_data.clear();
    //解析完成
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame);
    }
}

std::string Dlt645Analyse::DLTAnalyseData(const frame& src, const iot_template& templat)
{
    frame dest;
    HandleByte_order(src, dest, templat.byte_order);
    // Dlt -0x33
    for (auto it = dest.begin(); it != dest.end(); it++)
    {
        *it -= 0x33;
    }
    std::string res = HandleData_type(dest, templat.data_type, templat.correct_mode);
    return res;
}

bool Dlt645Analyse::DLTCheckHead(const frame& src, const int& index)
{
    if ((src.at(index) & 0x68) == 0x68          // 起始帧 0x68
        && (src.at(index + 7) & 0x68) == 0x68)     // 起始帧 0x68
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
    if(src.size() < ((u_int8_t)length + 12))
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
    frame dest;
    ReverseOrder(tmp, dest);
    return hexMeter == dest;
}