#pragma once

#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/SerialPort.h"
#include "NetSerial/NetSerial.h"

class mySerialPort : public NetSerial
{
public:
    mySerialPort(EventLoop *loop, const iot_gateway& gateway);
    ~mySerialPort();

    void start(double interval)
    {
        nextFrameInterval_ = interval;
        serialPort_.connect();
    }
    void SendData(const std::string &buf);
    void originTimer();

private:
    void getMsg();  // 定时从缓冲区buff中获取数据，在ThreaPool中解析处理

    void getNextFrame();  // 定时发送数据请求帧

    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time);//有新消息
    void onConnection(const ConnectionPtr &conn);    //有新连接
    void onClose(const ConnectionPtr& conn);     //有关闭信号

    std::atomic_bool boolconn_; 
    ConnectionPtr conn_;     //保存当前连接
    Buffer buff_;            //存储接收到的数据buffPtr_

    SerialPort serialPort_;
};