#pragma once

#include "Mediator.h"
#include "Factory/Pro188Factory/Pro188Factory.h"
#include "myHelper/myHelper.h"

class Pro188Mediator : public Mediator
{
public:
    Pro188Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& pro188Factory);
    ~Pro188Mediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);
    void secTimer();
private:
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType);     //解析完成后

    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息


    // nextFrame sendedFrame_;     //当前发送的数据及解析参数

    NetSerialPtr serialPortPtr_;
    AnalysePtr pro188AnalysePtr_;
    FramePtr pro188FramePtr_;

};
