#include "ModbusTcpFrame.h"

ModbusTcpFrame::ModbusTcpFrame(const iot_gateway& gatewayConf)
    : Frame(gatewayConf, 0, "modbusTcp")
    , frameNumber_(0)
{
    
}

ModbusTcpFrame::~ModbusTcpFrame()
{
    
}

void ModbusTcpFrame::start()
{
    // std::unique_lock<std::mutex> lock(R_VectorMutex);
    for(iot_device& device : gatewayConf_.v_device)
    {
        for(iot_template& templat : device.v_template)
        {
            if(templat.priority != enum_priority_write)
            {
                templat.rw = enum_read;
                std::vector<char> frame;
                templat.other = std::to_string(frameNumber_);   // other用来传递事务值
                uint16_To_char2(frameNumber_++, frame);         // 报文序列号
                uint16_To_char2(ModbusTcpIdentity, frame);      // modbustcp标志
                uint16_To_char2(ModbusTcpReadLength, frame);    // 数据长度
                frame.emplace_back(getDeviceAddr(device));      // 单元标识符（设备地址）
                frame.emplace_back(getDeviceReadFuncCode(templat));// 功能码
                uint16_To_char2(getRegisterAddr(templat), frame);
                uint16_To_char2(getRegisterQuantity(templat), frame);

                std::vector<iot_template> v_templat;
                v_templat.emplace_back(templat);

                nextFrame nextf(frame, pair_frame(device, v_templat));
                R_Vector.emplace_back(nextf);

                frame.clear();
                LOG_INFO("modbusTcp v_frame ++ ");
            }
        }
    }
    LOG_INFO("modbusTcp  device size = %d, R_Vector.size = %d\n", (int)gatewayConf_.v_device.size(), (int)R_Vector.size());
}
