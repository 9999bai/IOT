#include "mySerialPort.h"

mySerialPort::mySerialPort(EventLoop *loop, const iot_gateway &gateway)
    : NetSerial(loop)
    , serialPort_(loop, gateway.serial)
{
    // LOG_INFO("mySerialPort::mySerialPort() ctor...");
    serialPort_.setMessageCallback(std::bind(&mySerialPort::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    serialPort_.setConnectionCallback(std::bind(&mySerialPort::onConnection, this, std::placeholders::_1));
    serialPort_.setCloseCallback(std::bind(&mySerialPort::onClose,this, std::placeholders::_1));
    serialPort_.enableRetry();
}

mySerialPort::~mySerialPort()
{
    
}

void mySerialPort::getMsgTimer(double interval)  //定时获取缓存区数据
{
    getData_EveryT_ = loop_->runEvery(interval, std::bind(&mySerialPort::getMsg, this));
}

void mySerialPort::getMsg()
{
    if(buff_.readableBytes() > 0 && messageCallback_ && boolconn_)
    {
        if(messageCallback_)
        {
            messageCallback_(conn_, &buff_, Timestamp::now());
        }
    }
}

void mySerialPort::getNextFrameTimer(double interval)
{
    sendNext_EveryT_ = loop_->runEvery(interval, std::bind(&mySerialPort::getNextFrame, this));
}

void mySerialPort::getNextFrame()  // 定时发送数据请求帧
{
    if(nextFrameCallback_)
    {
        nextFrameCallback_();
    }
}

void mySerialPort::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    buff_.append(msg.c_str(), msg.size());
}

void mySerialPort::onConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        LOG_INFO("SerialPort onConnection UP: %s\n",conn->name().c_str());
        conn_ = conn;
        boolconn_ = true;
        getMsgTimer(GETMAG_TIMER_INTERVAL);//定时获取缓冲区数据
        getNextFrameTimer(MODBUSRTUNEXT_FREQ);//定时发送
        // if (newConnectionCallback_)
        // {
        //     newConnectionCallback_();
        // }
    }
    else
    {
        boolconn_ = false;
        RequestTimer_stop();
        GetDataTimer_stop();
        LOG_INFO("SerialPort onConnection DOWN: %s\n", conn->name().c_str());
        LOG_INFO("everyT Timer  closed...");
    }
}

void mySerialPort::onClose(const TcpConnectionPtr& conn)
{
    boolconn_ = false;
    RequestTimer_stop();
    GetDataTimer_stop();
    LOG_INFO("SerialPort conn [%s] closed... ",conn->name().c_str());
    LOG_INFO("everyT Timer  closed..."); 
}

void mySerialPort::SendData(const std::string& buf)
{
    if(boolconn_ && conn_)
    {
        conn_->send(buf);
    }
}