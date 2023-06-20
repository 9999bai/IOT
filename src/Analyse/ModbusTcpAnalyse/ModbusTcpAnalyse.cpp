#include "ModbusTcpAnalyse.h"

ModbusTcpAnalyse::ModbusTcpAnalyse()
{
    
}

ModbusTcpAnalyse::~ModbusTcpAnalyse()
{
    
}

void ModbusTcpAnalyse::AnalyseFunc(const std::string& msg, const nextFrame& nextframe)
{
    v_data.insert(v_data.end(), msg.begin(), msg.end());
    if(ModbusTcpAnalyseFrame_Minsize > v_data.size())
    {
        return;
    }

    bool resOK = false;
    enum_RW resRW;
    int index = 0;

    auto it = v_data.begin();
    // int length = v_data.at(5);
    int length = 0;
    char2or4Hex_To_uint16or32(frame(it + 4, it + 4 + 2), length);

    if(length + 6 > v_data.size())
    {
        return;//数据未接收完全
    }

    iot_device device = nextframe.second.first;
    iot_template templat = nextframe.second.second;
    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            if (!ModbustTcpMBAP(frame(it, it + 7), device, templat))
            {
                LOG_ERROR("ModbustTcpMBAP error... ");
                v_data.clear();
                return;
            }
            index += 7;

            if(v_data.at(index) != (int)templat.r_func)
            {
                LOG_ERROR("ModbustTcp funcCode error... %d", v_data.at(index));
                v_data.clear();
                return;
            }
            index++;

            if(HandleData(frame(it+index, it+index+length), nextframe))
            {
                LOG_ERROR("ModbusTcpAnalyse::AnalyseData error");
                resOK = false;
            }
            else{
                resOK = true;
            }
            v_data.clear();
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("ModbusTcpAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("ModbusTcpAnalyse::Analyse  error other...");
            v_data.clear();
            resOK = false;
        }
    }
    else if(templat.rw == enum_write)
    {
        resRW = enum_write;
        try
        {
            if (!ModbustTcpMBAP(frame(it, it + 7), device, templat))
            {
                LOG_ERROR("write ModbustTcpMBAP error... ");
                v_data.clear();
                return;
            }
            index += 7;
            if(v_data.at(index) == templat.w_func)
            {
                resOK = true;
                //解析完成 成功
            }
            else if(v_data.at(index) == (templat.w_func | 0x80))
            {
                resOK = false;
                //解析完成 失败
            }
        }
        catch(const std::out_of_range)
        {
            LOG_ERROR("ModbusTcpAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
            return;
        }
        catch(...)
        {
            LOG_ERROR("ModbusTcpAnalyse::Analyse  error ...");
            v_data.clear();
            resOK = false;
            return;
        }
    }

    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame));
    }
}

bool ModbusTcpAnalyse::ModbustTcpMBAP(const frame& src, const iot_device& device, const iot_template& templat)
{
    int value = 0;
    char2or4Hex_To_uint16or32(frame(src.begin(), src.begin() + 2), value);

    if( value != std::atoi(templat.other.c_str()))
    {
        LOG_ERROR("ModbustTcpMBAP transaction identity error...");
        return false;
    }

    int identity = 0;
    char2or4Hex_To_uint16or32(frame(src.begin() +2 , src.begin() + 4), identity);
    if(identity != 0)
    {
        LOG_ERROR("ModbustTcpMBAP identity error...");
        return false;
    }

    if(src.at(6) != std::atoi(device.device_addr.c_str()))
    {
        LOG_ERROR("ModbustTcpMBAP deviceID error...%d", (int)src.at(6));
        return false;
    }
    return true;
}