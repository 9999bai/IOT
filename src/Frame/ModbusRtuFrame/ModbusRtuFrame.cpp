#include "ModbusRtuFrame.h"
// #include "/usr/include/mymuduo/Logger.h"
// #include <vector>

ModbusRtuFrame::ModbusRtuFrame(const iot_gateway& gatewayConf)
               : Frame(gatewayConf, 0, "modbusRtu")
{
    
}

ModbusRtuFrame::~ModbusRtuFrame()
{

}

void ModbusRtuFrame::start()
{
    for (iot_device &device : gatewayConf_.v_device)
    {
        std::vector<char> frame;
        frame.emplace_back(getDeviceAddr(device));
        for(iot_template& templat : device.v_template)
        {
            if(templat.priority != enum_priority_write)
            {
                templat.rw = enum_read; // 当前帧为读
                std::vector<char> tmp(frame);
                frame.emplace_back(getDeviceReadFuncCode(templat));

                uint16_To_char2(getRegisterAddr(templat), frame);
                uint16_To_char2(getRegisterQuantity(templat), frame);

                u_int16_t crc = usMBCRC16(frame, frame.size());
                union_uint16TOuchar crctmp;
                crctmp.u16_data = crc;
                frame.emplace_back(crctmp.uchar_data[0]);//发送时modbusRTU数据帧时 crc16校验 低地址在前
                frame.emplace_back(crctmp.uchar_data[1]);

                std::vector<iot_template> v_templat;
                v_templat.emplace_back(templat);

                nextFrame nextf(frame, pair_frame(device, v_templat));
                R_Vector.emplace_back(nextf);

                frame = tmp;
                LOG_INFO("modbusrtu frame ++ ");
            }
        }
    }
    LOG_INFO("modbusRTU  device size = %d, R_Vector.size = %d\n", (int)gatewayConf_.v_device.size(), (int)R_Vector.size());
}


// void ModbusRtuFrame::addControlFrame(const nextFrame& controlFrame)
// {
//     {
//         std::unique_lock<std::mutex> lock(queueMutex);
//         controlQueue.push(controlFrame);
//         LOG_INFO("control frame ++++++++++++");
//     }
// }

// bool ModbusRtuFrame::getNextReadFrame(nextFrame& next_frame)
// {
//     if(!controlQueue.empty())
//     {
//         {
//             std::unique_lock<std::mutex> lock(queueMutex);
//             next_frame = controlQueue.front();
//             controlQueue.pop();
//         }
//         return true;
//     }

//     if(0 == v_frame_.size())
//     {
//         LOG_FATAL("ModbusRtuFrame::getNextMapFrame error : v_frame.size == 0");
//         return false;
//     }
//     try
//     {
//         next_frame.first = v_frame_.at(index_);
//         next_frame.second = m_frame_.at(index_);
//         ++index_ >= v_frame_.size() ? (index_ = 0) : 0;
//     }
//     catch(std::out_of_range)
//     {
//         LOG_FATAL("ModbusRtuFrame::getNextMapFrame  out_of_range...");
//         // abort();
//         next_frame.first = v_frame_.at(0);
//         next_frame.second = m_frame_.at(0);
//         index_ = 0;
//     }
//     return true;
// }
