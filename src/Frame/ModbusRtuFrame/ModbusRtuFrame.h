#pragma once

#include "Frame/Frame.h"
#include "myHelper/myHelper.h"


class ModbusRtuFrame : public Frame
{
public:
    ModbusRtuFrame(const iot_gateway& gatewayConf);
    ~ModbusRtuFrame();

    void start();

private:
    // int index_;
    // std::mutex queueMutex;
    // std::queue<nextFrame> controlQueue; //控制数据帧集合
    // map_frame m_frame_;   //数据参数集合
    // vector_frame v_frame_;//数据帧集合
};