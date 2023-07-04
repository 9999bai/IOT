#include "myUdpClient.h"

myUdpClient::myUdpClient(EventLoop *loop, const iot_gateway& gateway, const std::string& localIP)
            : NetSerial(loop)
            , udpClient_(loop, "udpclient", InetAddress(gateway.net.port, gateway.net.ip), localIP)
{
    udpClient_.setMessageCallback(std::bind(&myUdpClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    udpClient_.setConnectionCallback(std::bind(&myUdpClient::onConnection, this, std::placeholders::_1));
    udpClient_.enableRetry();
}

myUdpClient::~myUdpClient()
{

}

void myUdpClient::start(double interval)
{
    timer_Interval_ = interval;
    udpClient_.connect();
}

void myUdpClient::SendData(const std::string &buf)
{
    if(boolconn_ && conn_)
    {
        conn_->send(buf);
    }
}

void myUdpClient::getMsgTimer(double interval)
{
    getData_EveryT_ = loop_->runEvery(interval, std::bind(&myUdpClient::getMsg, this));
}

void myUdpClient::getMsg()
{
    if(buff_.readableBytes() > 0 && messageCallback_ && boolconn_)
    {
        if(messageCallback_)
        {
            messageCallback_(conn_, &buff_, Timestamp::now());
        }
    }
}

void myUdpClient::getNextFrameTimer(double interval)
{
    sendNext_EveryT_ = loop_->runEvery(interval, std::bind(&myUdpClient::getNextFrame, this));
}

void myUdpClient::getNextFrame()  // 定时发送数据请求帧
{
    if(nextFrameCallback_)
    {
        nextFrameCallback_();
    }
}

void myUdpClient::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time) //有新消息
{
    std::string msg = buf->retrieveAllAsString();
    buff_.append(msg.c_str(), msg.size());
}

void myUdpClient::onConnection(const ConnectionPtr &conn)
{
    if(conn->connected())
    {
        LOG_INFO("UdpClient onConnection UP: %s\n",conn->name().c_str());
        conn_ = conn;
        boolconn_ = true;
        getMsgTimer(TCP_GETMAG_TIMER_INTERVAL); // 定时获取 接收缓冲区 数据
        getNextFrameTimer(timer_Interval_);     // 定时发送
        if(newConnectionCallback_)
        {
            newConnectionCallback_();
        }
    }
    else
    {
        boolconn_ = false;
        RequestTimer_stop();
        GetDataTimer_stop();
        LOG_INFO("UdpClient onConnection DOWN: %s\n", conn->name().c_str());
        LOG_INFO("everyT Timer  closed...");
    }
}

void myUdpClient::onClose(const ConnectionPtr& conn)  //有关闭信号
{
    if(closeCallback_)
    {
        closeCallback_(conn);
    }
}




