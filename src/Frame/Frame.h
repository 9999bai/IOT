#pragma once

#include "myHelper/myHelper.h"

#include <vector>

class Frame
{
public:
    Frame(const iot_gateway &gatewayConf, int index, const std::string& name);
    virtual ~Frame();

    int getDeviceAddr(const iot_device& device) { return std::atoi(device.device_addr.c_str()); }
    int getDeviceReadFuncCode(const iot_template &templat) { return templat.r_func; }
    uint16_t getRegisterAddr(const iot_template &templat) { return std::atoi(templat.register_addr.c_str()); }
    uint16_t getRegisterQuantity(const iot_template& templat) { return templat.register_quantity; }

    virtual void start() = 0;

    bool cycleFinish();
    bool getNextReadFrame(nextFrame &next_frame);           // 下一帧数据
    void addControlFrame(const nextFrame &controlFrame);    // 添加控制数据帧

    // int getMark() { return mark_; }

protected:
    std::string name_;
    iot_gateway gatewayConf_;

    int index_;
    std::mutex W_QueueMutex;
    std::queue<nextFrame> W_Queue; // 控制数据帧集合

    std::mutex R_VectorMutex;
    std::vector<nextFrame> R_Vector;// 读数据帧集合

    // int mark_; // 0x00:normal  0x01:YC  0x02:YM  0x03:YC+YM
};