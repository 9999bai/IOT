#pragma once

#include "Mediator.h"
#include "Factory/ModbusRtuFactory/ModbusRtuFactory.h"
#include "myHelper/myHelper.h"
// #include <mutex>
// #include <atomic>

class ModbusRtuMediator : public Mediator
{
public:
    ModbusRtuMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& modbusRtuFactory);
    ~ModbusRtuMediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);
    void secTimer();

private:
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType);     //解析完成后

    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息

    // nextFrame sendedFrame_;     //当前发送的数据及解析参数

    NetSerialPtr serialPortPtr_;
    AnalysePtr modbusrtuAnalysePtr_;
    FramePtr modbusrtuFramePtr_;
};
