#pragma once

#include "Mediator.h"
#include "Factory/Dlt645Factory/Dlt645Factory.h"
#include "myHelper/myHelper.h"

class Dlt645Mediator : public Mediator
{
public:
    Dlt645Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& dlt645Factory);
    ~Dlt645Mediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);

private:
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw);     //解析完成后
    
    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息

    nextFrame sendedFrame_;     //当前发送的数据及解析参数

    NetSerialPtr serialPortPtr_;
    AnalysePtr dlt645AnalysePtr_;
    FramePtr dlt645FramePtr_;
};