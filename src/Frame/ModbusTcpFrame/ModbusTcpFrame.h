#pragma once

#include "Frame/Frame.h"

class ModbusTcpFrame : public Frame
{
public:
    ModbusTcpFrame(const iot_gateway& gatewayConf);
    ~ModbusTcpFrame();

    void start();

private:
    u_int16_t frameNumber_;   //编号

    // map_frame m_frame_;   //数据参数集合
    // vector_frame v_frame_;//数据帧集合
};