#include "myTcpClient.h"
#include "/usr/include/mymuduo/AbstractConnection.h"

myTcpClient::myTcpClient(EventLoop *loop, const iot_gateway& gateway) 
            : NetSerial(loop)
            , tcpClient_(loop, "tcpclient", InetAddress(gateway.net.port, gateway.net.ip))
{
    tcpClient_.setMessageCallback(std::bind(&myTcpClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    tcpClient_.setConnectionCallback(std::bind(&myTcpClient::onConnection, this, std::placeholders::_1));
    // tcpClient_.setCloseCallback(std::bind(&myTcpClient::onClose,this, std::placeholders::_1));
    tcpClient_.enableRetry();
}

myTcpClient::~myTcpClient()
{

}

void myTcpClient::SendData(const std::string &buf)
{
    if(boolconn_ && conn_)
    {
        conn_->send(buf);
    }
}

void myTcpClient::getMsgTimer(double interval)  //定时获取缓存区数据
{
    getData_EveryT_ = loop_->runEvery(interval, std::bind(&myTcpClient::getMsg, this));
}

void myTcpClient::getMsg()
{
    if(buff_.readableBytes() > 0 && messageCallback_ && boolconn_)
    {
        if(messageCallback_)
        {
            messageCallback_(conn_, &buff_, Timestamp::now());
        }
    }
}

void myTcpClient::getNextFrameTimer(double interval)
{
    sendNext_EveryT_ = loop_->runEvery(interval, std::bind(&myTcpClient::getNextFrame, this));
}

void myTcpClient::getNextFrame()
{
    if(nextFrameCallback_)
    {
        nextFrameCallback_();
    }
}

void myTcpClient::onConnection(const ConnectionPtr &conn)
{
    if(conn->connected())
    {
        LOG_INFO("TcpClient onConnection UP: %s\n",conn->name().c_str());
        conn_ = conn;
        boolconn_ = true;
        getMsgTimer(GETMAG_TIMER_INTERVAL);//定时获取缓冲区数据
        getNextFrameTimer(MODBUSRTUNEXT_FREQ);//定时发送
    }
    else
    {
        boolconn_ = false;
        RequestTimer_stop();
        GetDataTimer_stop();
        LOG_INFO("TcpClient onConnection DOWN: %s\n", conn->name().c_str());
        LOG_INFO("everyT Timer  closed...");
    }
}

void myTcpClient::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    buff_.append(msg.c_str(), msg.size());
}

