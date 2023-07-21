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
    void setNewConnectionCallback(NewConnectionCallback cb) { newConnectionCallback_ = cb; }
    void setCloseCallback(CloseCallback cb) { closeCallback_ = cb; };

    virtual void SendData(const std::string &buf) = 0;
    virtual void start(double interval) = 0;
    virtual void restart() {}

    virtual void originTimer() = 0;

protected:
    NextFrameCallback nextFrameCallback_;
    MessageCallback messageCallback_;
    NewConnectionCallback newConnectionCallback_;
    CloseCallback closeCallback_;
    TimerId originTimerID_;
    int timerValue_; // origin_timer
    double nextFrameInterval_; // 获取下一帧数据时间间隔

    EventLoop *loop_;
};