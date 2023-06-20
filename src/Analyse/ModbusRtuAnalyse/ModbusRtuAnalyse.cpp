#include "ModbusRtuAnalyse.h"
#include "/usr/include/mymuduo/Logger.h"
#include "/usr/include/mymuduo/CurrentThread.h"

#include <exception>

ModbusRtuAnalyse::ModbusRtuAnalyse():Analyse()
{
    // LOG_INFO("ModbusRtuAnalyse::ModbusRtuAnalyse() ctor ...");
}

ModbusRtuAnalyse::~ModbusRtuAnalyse()
{

}

void ModbusRtuAnalyse::AnalyseFunc(const std::string& msg, const nextFrame& nextframe)
{
    // LOG_INFO("ModbusRtuAnalyse::AnalyseFunc v_data.capacity()=%d",v_data.capacity());
    v_data.insert(v_data.end(), msg.begin(), msg.end());
    
    LOG_INFO("ModbusRtuAnalyse::Analyse---start----   v_data.size = %d , curThread:id=%d", v_data.size(), CurrentThread::tid());

    if (ModbusRtuAnalyseFrame_MinSize > v_data.size())
    {
        return;
    }
    bool resOK = false; // 解析是否成功
    enum_RW resRW;
    int index = 0;
    iot_device device = nextframe.second.first;
    iot_template templat = nextframe.second.second;

    if(templat.rw == enum_read)// 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            while (!v_data.empty())
            {
                index = 0;
                
                if(!compairAddr(v_data, index, device.device_addr))// addr
                {
                    LOG_ERROR("Addr error...\n");
                    continue;
                }
                index++;
                
                if(!compairFuncCode(v_data, index, templat.r_func))// func code
                {
                    LOG_ERROR("FuncCode error...\n");
                    continue;
                }
                index++;
                break;
            }

            u_int8_t len = v_data.at(index);// len
            index++;

            if(index + len + 2 > v_data.size())//未接收完
            {
                // LOG_INFO("未接收完全...");
                return;
            }
            auto it = v_data.begin();

            // (要进行CRC校验的数据段, 报文最后俩字节CRC数据段)
            if (!CheckCRC(frame(it, it + index + len), frame(v_data.end()-2, v_data.end()))) // CRC校验
            {
                LOG_INFO("CRC error...\n");//清空
                v_data.clear();
                return;
            }

            if (!HandleData(frame(it + index, it + index + len), nextframe)) //数据解析部分
            {
                LOG_ERROR("ModbusRtuAnalyse::Analyse data error");
                resOK = false;
            }
            else
            {
                resOK = true;
            }

            v_data.clear();
            // index += len;
            // index += 2;
            // v_data.erase(it, it + index);//解析完一帧数据删除
        }
        catch(std::out_of_range)//数组越界
        {
            LOG_ERROR("ModbusRtuAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
            // return;
        }
        catch(...)
        {
            LOG_ERROR("ModbusRtuAnalyse::Analyse  error other...");
            v_data.clear();
            resOK = false;
            // return;
        }
    }
    else if(templat.rw == enum_write) // 写语句返回 解析
    {
        resRW = enum_write;
        try
        {
            while (!v_data.empty())
            {
                index = 0;
                if(!compairAddr(v_data, index, device.device_addr)) // addr
                {
                    LOG_ERROR("enum_write Addr error...\n");
                    continue;
                }
                index++;
                
                if(v_data.at(index) == templat.w_func)
                {
                    resOK = true;
                    break;
                    //解析完成 成功
                }
                else if(v_data.at(index) == (templat.w_func | 0x80))
                {
                    resOK = false;
                    break;
                    //解析完成 失败
                }
                break;
            }
        }
        catch(std::out_of_range)//数组越界
        {
            LOG_ERROR("ModbusRtuAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
            return;
        }
        catch(...)
        {
            LOG_ERROR("ModbusRtuAnalyse::Analyse  error ...");
            v_data.clear();
            resOK = false;
            return;
        }
        v_data.clear();
    }
    
    //解析完成
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame));
    }
}

bool ModbusRtuAnalyse::compairAddr(frame& v_data, int index, const std::string& s2)
{
    if(v_data.at(index) == std::atoi(s2.c_str()))
    {
        return true;
    }
    else
    {
        LOG_ERROR("%d : %d \n", (u_int8_t)v_data.at(index), std::atoi(s2.c_str()));
        v_data.erase(v_data.begin());
        return false;
    }
}

bool ModbusRtuAnalyse::compairFuncCode(frame& data, int index, const enum_r_func_code& funcCode)
{
    if(data.at(index) == funcCode)
    {
        return true;
    }
    else
    {
        int errF = data.at(index);
        switch (errF)
        {
            case 0x81:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x01--读线圈错误").c_str());data.clear();return false;
            case 0x82:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x02--读输入离散量错误").c_str());data.clear();return false;
            case 0x83:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x03--读输入寄存器错误").c_str());data.clear();return false;
            case 0x84:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x04--读多个寄存器错误").c_str());data.clear();return false;
            case 0x85:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x05--写单个线圈错误").c_str());data.clear();return false;
            case 0x86:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x06--写单个寄存器错误").c_str());data.clear();return false;
            case 0x8F:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x0F--写多个线圈错误").c_str());data.clear();return false;
            case 0x90:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x10--写多个寄存器错误").c_str());data.clear();return false;
            case 0x96:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x16--屏蔽写寄存器错误").c_str());data.clear();return false;
            case 0x97:
                LOG_ERROR("ERROR: RX--FuncCode = %d, %s", errF, std::string("0x17--读/写多个寄存器错误").c_str());data.clear();return false;
            default:
                LOG_ERROR("ERROR: RX--FuncCode = %d", (u_int8_t)v_data.at(index));data.clear();return false;
        }
        // LOG_ERROR("%d : %d  \n", (int)v_data.at(index), (int)funcCode);
        auto it = data.begin();
        data.erase(it, it + 1);
        return false;
    }
}


// void ModbusRtuAnalyse::updateDataTimer(const iot_data_item& item)
// {
//     // LOG_INFO("Data = %s", item.value.c_str());
//     std::unique_lock<std::mutex>(data_mutex_);
//     for (int i = 0; i < v_iotdataTimer_.size(); i++)
//     {
//         iot_data &data = v_iotdataTimer_[i];
//         if(data.gateway_id == item.gateway_id)
//         {
//             for (int j = 0; j < data.v_device.size(); j++)
//             {
//                 iot_data_device &device = data.v_device[j];
//                 device.m_field[item.param_name] = item;
//                 LOG_INFO("updateData: update suc..");
//                 return;
//             }
//             iot_data_device tmp;
//             tmp.device_addr = item.device_addr;
//             tmp.m_field[item.param_name] = item;
//             data.v_device.emplace_back(tmp);
//             LOG_INFO("updateData  iot_data_device++  v_device.size = %d",data.v_device.size());
//         }
//     }
//     iot_data dat;
//     dat.gateway_id = item.gateway_id;
//     iot_data_device device;
//     device.device_addr = item.device_addr;
//     device.m_field[item.param_name] = item;
//     dat.v_device.emplace_back(device);
//     v_iotdataTimer_.emplace_back(dat);
//     LOG_INFO("updateData  iot_data++  v_iotdataTimer.size = %d",v_iotdataTimer_.size());
// }




// void ModbusRtuAnalyse::handle16HL_HR(const frame& v_data, const enum_byte_order& type)
// {
//     LOG_INFO("handle16HL_HR");
//     if(2 != v_data.size())
//     {
//         LOG_INFO("handle16HL_HR  v_data.size = %d\n",v_data.size());
//         return;
//     }

//     switch (type)
//     {
//     case enum_byte_order_AB:
//         break;
//     case enum_byte_order_BA:
//         LOG_INFO("enum_byte_order_BA");
//         const_cast<frame&>(v_data) = char2_BA(v_data);
//         break;
//     case enum_byte_order_AB_CD:
//         break;
//     case enum_byte_order_CD_AB:
//         break;
//     case enum_byte_order_BA_DC:
//         break;
//     case enum_byte_order_DC_BA:
//         break;
//     case enum_byte_order_AB_CD_EF_GH:
//         break;
//     case enum_byte_order_GH_EF_CD_AB:
//         break;
//     case enum_byte_order_BA_DC_FE_HG:
//         break;
//     case enum_byte_order_HG_FE_DC_BA:
//         break;
//     }
// }

// void ModbusRtuAnalyse::handle32HL_HR(const frame& v_data, const enum_byte_order& type)
// {
//     LOG_INFO("handle32HL_HR");
//     if(4 != v_data.size())
//     {
//         LOG_ERROR("handle32HL_HR  v_data.size = %d\n",v_data.size());
//         return;
//     }
    
//     switch (type)
//     {
//         case enum_byte_order_AB:
//             break;
//         case enum_byte_order_BA:
//             break;
//         case enum_byte_order_AB_CD:
//             break;
//         case enum_byte_order_CD_AB:
//             LOG_INFO("enum_byte_order_CD_AB");
//             const_cast<frame&>(v_data) = char4_CD_AB(v_data);
//             break;
//         case enum_byte_order_BA_DC:
//             LOG_INFO("enum_byte_order_BA_DC");
//             const_cast<frame&>(v_data) = char4_BA_DC(v_data);
//             break;
//         case enum_byte_order_DC_BA:
//             LOG_INFO("enum_byte_order_DC_BA");
//             const_cast<frame&>(v_data) = char4_DC_BA(v_data);
//             break;
//         case enum_byte_order_AB_CD_EF_GH:
//             break;
//         case enum_byte_order_GH_EF_CD_AB:
//             break;
//         case enum_byte_order_BA_DC_FE_HG:
//             break;
//         case enum_byte_order_HG_FE_DC_BA:
//             break;
//     }
// }

// void ModbusRtuAnalyse::handle64HL_HR(const frame& v_data, const enum_byte_order& type)
// {
//     LOG_INFO("handle64HL_HR");
//     if(8 != v_data.size())
//     {
//         LOG_ERROR("handle64HL_HR  v_data.size = %d\n",v_data.size());
//         return;
//     }
    
//     switch (type)
//     {
//         case enum_byte_order_AB:
//             break;
//         case enum_byte_order_BA:
//             break;
//         case enum_byte_order_AB_CD:
//             break;
//         case enum_byte_order_CD_AB:
//             break;
//         case enum_byte_order_BA_DC:
//             break;
//         case enum_byte_order_DC_BA:
//             break;
//         case enum_byte_order_AB_CD_EF_GH:
//             break;
//         case enum_byte_order_GH_EF_CD_AB:
//             LOG_INFO("enum_byte_order_GH_EF_CD_AB");
//             const_cast<frame&>(v_data) = char8_GH_EF_CD_AB(v_data);
//             break;
//         case enum_byte_order_BA_DC_FE_HG:
//             LOG_INFO("enum_byte_order_BA_DC_FE_HG");
//             const_cast<frame&>(v_data) = char8_BA_DC_FE_HG(v_data);
//             break;
//         case enum_byte_order_HG_FE_DC_BA:
//             LOG_INFO("enum_byte_order_HG_FE_DC_BA");
//             const_cast<frame&>(v_data) = char8_HG_FE_DC_BA(v_data);
//             break;
//     }
// }

// u_int8_t ModbusRtuAnalyse::handleData_8bitStr(const frame& v_data, const iot_template& templat)
// {
//     LOG_INFO("handleData_8bitStr");
//     if(templat.sub_template_id == 0)
//     {
//         return std::atoi(&v_data.at(0));
//     }
//     else
//     {
//         LOG_ERROR("ModbusRtuAnalyse::handleData_8bitStr");
//     }
// }

// u_int8_t ModbusRtuAnalyse::handleData_8bitHex(const frame& v_data)
// {
//     LOG_INFO("handleData_8bitHex");
//     return (u_int8_t)v_data.at(0);
// }

// std::unordered_map<int,std::pair<std::string,bool>> ModbusRtuAnalyse::handleData_8bitHex(const frame& v_data, const iot_template& templat)
// {
//     LOG_INFO("handleData_8bitHex");
//     std::unordered_map<int,std::pair<std::string,bool>> map_bool;
//     handleData_subTemplate(v_data, templat, map_bool);
//     return map_bool;
// }

// void ModbusRtuAnalyse::handleData_subTemplate(const frame& v_data, const iot_template& templat, std::unordered_map<int,std::pair<std::string,bool>> & map_bool)
// {
//     LOG_INFO("handleData_subTemplate");
//     u_char data = (u_char)v_data.at(0);
//     for (auto it = templat.v_sub_template.begin(); it != templat.v_sub_template.end(); it++)
//     {
//         bool res = false;
//         std::pair<std::string, bool> tmp(it->name, res);
//         switch (it->bit)
//         {
//         case enum_bit_0:
//             LOG_INFO("enum_bit_0");
//             res = data & 0x01;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_1:
//             LOG_INFO("enum_bit_1");
//             res = data & 0x02;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_2:
//             LOG_INFO("enum_bit_2");
//             res = data & 0x04;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_3:
//             LOG_INFO("enum_bit_3");
//             res = data & 0x08;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_4:
//             LOG_INFO("enum_bit_4");
//             res = data & 0x10;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_5:
//             LOG_INFO("enum_bit_5");
//             res = data & 0x20;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_6:
//             LOG_INFO("enum_bit_6");
//             res = data & 0x40;
//             map_bool[it->bit] = tmp;
//             break;
//         case enum_bit_7:
//             LOG_INFO("enum_bit_7");
//             res = data & 0x80;
//             map_bool[it->bit] = tmp;
//             break;
//         }
//         LOG_INFO("ModbusRtuAnalyse::handleData_8bit : name=%s,res=%d", it->name.c_str(), (int)res);
//     }
// }
