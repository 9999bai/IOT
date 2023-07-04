#pragma once

#include "Mediator.h"
#include "Factory/BacnetipFactory/BacnetipFactory.h"
#include "myHelper/myHelper.h"

class BacnetipMediator : public Mediator
{
public:
    BacnetipMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& bacnetipFactory);
    ~BacnetipMediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);
    void secTimer(){}

private:

    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType); // 解析完成后
    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息

    NetSerialPtr udpClientPtr_;
    AnalysePtr bacnetipAnalysePtr_;
    FramePtr bacnetipFramePtr_;
};
