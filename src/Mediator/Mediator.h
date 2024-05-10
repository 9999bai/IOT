#pragma once

#include "/usr/include/mymuduo/Logger.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/ThreadPool.h"
#include "/usr/include/mymuduo/TcpConnection.h"

#include "myHelper/myHelper.h"

class Mediator
{
public:
    Mediator(EventLoop *loop, const iot_gateway &gateway, const std::shared_ptr<ThreadPool> &poolPtr);
    virtual ~Mediator();

    virtual void addControlFrame(const nextFrame& controlFrame) = 0;
    virtual void start() = 0;

    virtual void secTimer() = 0;

    void setAnalyseNotifyCallback(AnalyseFinishCallback cb) { analyseFinishCallback_ = cb; } 

protected:
    EventLoop *loop_;
    std::shared_ptr<ThreadPool> poolPtr_;
    iot_gateway gateway_;

    structNextFrame sendFrame_;     //当前发送的数据及解析参数
    // opcnextFrame opcSendFrame_; // opc

    AnalyseFinishCallback analyseFinishCallback_;
    int sec_;
};