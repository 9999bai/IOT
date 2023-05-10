#pragma once 

#include "NetSerial/NetSerial.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/TcpClient.h"

class myTcpClient : public NetSerial
{
public:
    myTcpClient(EventLoop *loop, const iot_gateway& gateway);
    ~myTcpClient();

    void start() { tcpClient_.connect(); };
    void SendData(const std::string &buf);

private:
    void getMsgTimer(double interval);
    void getMsg();

    void getNextFrameTimer(double interval);
    void getNextFrame();  // 定时发送数据请求帧

    void RequestTimer_stop() { loop_->cancel(sendNext_EveryT_); }//发送定时器 关闭
    void GetDataTimer_stop() { loop_->cancel(getData_EveryT_); } //获取数据定时器 关闭 

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);//有新消息
    void onConnection(const TcpConnectionPtr &conn);
    // void onClose(const TcpConnectionPtr& conn);     //有关闭信号

    TimerId getData_EveryT_;
    TimerId sendNext_EveryT_;
    std::atomic_bool boolconn_;
    TcpConnectionPtr conn_;
    Buffer buff_;       //存储接收到的数据buffPtr_

    TcpClient tcpClient_;
};