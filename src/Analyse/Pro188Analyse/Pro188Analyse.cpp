#include "Pro188Analyse.h"

Pro188Analyse::Pro188Analyse()
{

}

Pro188Analyse::~Pro188Analyse()
{

}

void Pro188Analyse::AnalyseFunc(const std::string &msg, const nextFrame &nextframe, void* pending)
{
    v_data.insert(v_data.end(), msg.begin(), msg.end());
    if (CJT188AnalyseFrame_Minsize > v_data.size())
    {
        return;
    }

    for(int i=0; i<4; i++)// 前四个字节有可能是0xFE，删除
    {
        auto it = v_data.begin();
        if((*it) & 0xFE == 0xFE)
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
        LOG_FATAL("pro188Analyse::AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);


    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            auto it = v_data.begin();
            if(CJT188CheckHead(v_data, index))
            {
                LOG_ERROR("Pro188Analyse::AnalyseFunc CJT188CheckHead error...");
                v_data.clear();
            }
            if(CJT188CheckLength(v_data, v_data.at(10)))      // 整体长度校验
            {
                return; // 数据接收不全
            }
            if (!CJT188DataCheck(frame(it, it+v_data.size()-2), v_data.at(v_data.size()-2)))
            {
                LOG_ERROR("Pro188Analyse::AnalyseFunc CJT188DataCheck error...");
                v_data.clear();
            }
            index += 2;
            if(!CJT188MeteridCheck(frame(it + index, it + index + 7), device.device_addr))
            {
                LOG_ERROR("Pro188Analyse::AnalyseFunc meter error...");
            }
            index += 7;
            switch(v_data.at(index))
            {
                case 0x81:
                {
                    index += 2;
                    // 数据标识
                    // frame(it + index, it + index + 2);
                    index += 2; // 标识
                    index++; // 序号
                    std::string res = CJT188AnalyseData(frame(it + index, it + index + 4), templat);
                    // LOG_INFO("Pro188Analyse %s = %s", templat.param_name.c_str(), res.c_str());
                    QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, res));
                    resOK = true;
                }
                break;

            }
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("Pro188Analyse out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("Pro188Analyse::Analyse error other...");
            v_data.clear();
            resOK = false;
        }
        v_data.clear();
    }
    else if(templat.rw == enum_write)
    {

    }
    
    // 解析完成
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame/*std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame)*/);
    }
}

bool Pro188Analyse::CJT188CheckHead(const frame& data, const int& index)
{
    if(data.at(0) & 0x68 == 0x68 
       && data.at(1) & 0x10 == 0x10
       && data.at(data.size() - 1) & 0x16 == 0x16)
    {
        return true;
    }
    return false;
}
bool Pro188Analyse::CJT188CheckLength(const frame& data, const u_int8_t& lengthChar)
{
    if(data.size() < lengthChar + 13)
    {
        return false;
    }
    return true;
}

bool Pro188Analyse::CJT188DataCheck(const frame& data, const char& check)
{
    return check == DLTCheck(data);
}

bool Pro188Analyse::CJT188MeteridCheck(const frame& data, const std::string& strMeter)
{
    frame tmp = HexStrToByteArray(strMeter);
    return data == tmp;
}

std::string Pro188Analyse::CJT188AnalyseData(const frame& data, const iot_template& templat)
{
    frame dest;
    HandleByte_order(data, dest, templat.byte_order);
    std::string res = HandleData_type(dest, templat.data_type, templat.correct_mode);
    return res;
}
