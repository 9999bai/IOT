#include "ModbusTcpAnalyse.h"

ModbusTcpAnalyse::ModbusTcpAnalyse()
{
    
}

ModbusTcpAnalyse::~ModbusTcpAnalyse()
{
    
}

void ModbusTcpAnalyse::AnalyseFunc(const std::string& msg, const nextFrame& nextframe, void* pending)
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
    int length = 0; // 除事务标志字节外总长度
    char2or4Hex_To_uint16or32(frame(it + 4, it + 4 + 2), length);

    if(length + 6 > v_data.size())
    {
        return; //数据未接收完全
    }

    iot_device device = nextframe.second.first;
    std::vector<iot_template> v_templat = nextframe.second.second;
    frame t_frame = nextframe.first;

    if(v_templat.size() < 0)
    {
        LOG_FATAL("ModbusTCPAnalyse::AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);

    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            // if (!ModbustTcpMBAP(frame(it, it + 7), device, templat))
            // {
            //     LOG_ERROR("ModbustTcpMBAP error... ");
            //     v_data.clear();
            //     return;
            // }
            index += 6;
            if (v_data.at(index) != t_frame.at(index))
            {
                LOG_ERROR("ModbustTcp deviceid error... %d, %d", (int)v_data.at(index), (int)t_frame.at(index));
                return;
            }
            index++;

            if((int)v_data.at(index) != (int)templat.r_func)
            {
                LOG_ERROR("ModbustTcp funcCode error... %d", v_data.at(index));
                // resOK = false;
                return;
            }
            index++;

            length = v_data.at(index++); // 数据部分长度
            if (!HandleData(frame(it + index, it + index + length), nextframe))
            {
                LOG_ERROR("ModbusTcpAnalyse::AnalyseData error");
                resOK = false;
            }
            else{
                resOK = true;
            }
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
            // if (!ModbustTcpMBAP(frame(it, it + 7), device, templat))
            // {
            //     LOG_ERROR("write ModbustTcpMBAP error... ");
            //     v_data.clear();
            //     return;
            // }
            index += 6;
            if(v_data.at(index) != t_frame.at(index))
            {
                LOG_ERROR("ModbusTCPAnalyse deviceID error %d, %d", (int)v_data.at(index), (int)t_frame.at(index));
                resOK = false;
            }
            index++;
            if (v_data.at(index) == t_frame.at(index))
            {
                resOK = true;
                //解析完成 成功
            }
            else if(v_data.at(index) == (templat.w_func | 0x80))
            {
                LOG_ERROR("ModbusTCPAnalyse func code error %d", (int)v_data.at(index));
                resOK = false;
                //解析完成 失败
            }else{
                LOG_ERROR("ModbusTCPAnalyse func code error %d, %d", (int)v_data.at(index), (int)templat.w_func);
                resOK = false;
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
    
    v_data.clear();

    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame/*std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame)*/);
    }
}

bool ModbusTcpAnalyse::ModbustTcpMBAP(const frame& src, const iot_device& device, const iot_template& templat)
{
    int value = 0;
    frame t_data = frame(src.begin(), src.begin() + 2);
    value += (u_int8_t)t_data.at(0) * 16 + (u_int8_t)t_data.at(1);

    // char2or4Hex_To_uint16or32(frame(src.begin(), src.begin() + 2), value);
    // printFrame("-----", src);

    // LOG_INFO("------%s", templat.other.c_str());
    if (value != std::atoi(templat.other.c_str()))
    {
        LOG_ERROR("ModbustTcpMBAP transaction identity error... %d, %d",value, std::atoi(templat.other.c_str()));
        return false;
    }

    int identity = 0;
    char2or4Hex_To_uint16or32(frame(src.begin() +2 , src.begin() + 4), identity);
    if(identity != 0)
    {
        LOG_ERROR("ModbustTcpMBAP identity error...%d", identity);
        return false;
    }

    if(src.at(6) != std::atoi(device.device_addr.c_str()))
    {
        LOG_ERROR("ModbustTcpMBAP deviceID error...%d", (int)src.at(6));
        return false;
    }
    return true;
}