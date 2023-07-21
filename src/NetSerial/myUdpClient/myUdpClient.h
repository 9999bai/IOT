#pragma once 

#include "NetSerial/NetSerial.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/UdpClient.h"

class Buffer;

class myUdpClient : public NetSerial
{
public:
    myUdpClient(EventLoop *loop, const iot_gateway& gateway, const std::string& localIP);
    ~myUdpClient();

    void start(double interval);
    void restart() { udpClient_.restart(); }
    void SendData(const std::string &buf);
    void originTimer();
    
private:
    void getMsg();

    void getNextFrame();  // 定时发送数据请求帧

    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time);//有新消息
    void onConnection(const ConnectionPtr &conn);
    void onClose(const ConnectionPtr& conn);  //有关闭信号

    std::atomic_bool boolconn_;
    ConnectionPtr conn_;
    Buffer buff_;       //存储接收到的数据buffPtr_

    UdpClient udpClient_;
};
