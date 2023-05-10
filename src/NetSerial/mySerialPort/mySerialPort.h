#pragma once

#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/SerialPort.h"
#include "NetSerial/NetSerial.h"

class mySerialPort : public NetSerial
{
public:
    mySerialPort(EventLoop *loop, const iot_gateway& gateway);
    ~mySerialPort();

    void start() { serialPort_.connect(); }
    void SendData(const std::string &buf);

private:
    void getMsgTimer(double interval);
    void getMsg();  // 定时从缓冲区buff中获取数据，在ThreaPool中解析处理

    void getNextFrameTimer(double interval);
    void getNextFrame();  // 定时发送数据请求帧

    void RequestTimer_stop() { loop_->cancel(sendNext_EveryT_); }//发送定时器 关闭
    void GetDataTimer_stop() { loop_->cancel(getData_EveryT_); } //获取数据定时器 关闭 

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);//有新消息
    void onConnection(const TcpConnectionPtr &conn);    //有新连接
    void onClose(const TcpConnectionPtr& conn);     //有关闭信号

    TimerId sendNext_EveryT_;
    TimerId getData_EveryT_;

    std::atomic_bool boolconn_; 
    TcpConnectionPtr conn_;     //保存当前连接
    Buffer buff_;       //存储接收到的数据buffPtr_

    SerialPort serialPort_;
};