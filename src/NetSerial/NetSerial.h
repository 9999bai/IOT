#pragma once

#include "/usr/include/mymuduo/EventLoop.h"
#include "myHelper/myHelper.h"

class NetSerial
{
public:
    NetSerial(EventLoop *loop);
    virtual ~NetSerial();

    void setNextFrameCallback(NextFrameCallback cb) { nextFrameCallback_ = cb; }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; }

    virtual void SendData(const std::string &buf) = 0;
    virtual void start() = 0;

protected:
    NextFrameCallback nextFrameCallback_;
    MessageCallback messageCallback_;

    EventLoop *loop_;
};