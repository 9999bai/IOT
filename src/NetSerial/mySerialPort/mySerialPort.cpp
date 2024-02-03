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

void mySerialPort::getNextFrame()  // 定时发送数据请求帧
{
    if(nextFrameCallback_)
    {
        nextFrameCallback_();
    }
}

void mySerialPort::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    buff_.append(msg.c_str(), msg.size());
}

void mySerialPort::onConnection(const ConnectionPtr& conn)
{
    if(conn->connected())
    {
        LOG_INFO("SerialPort onConnection UP: %s\n",conn->name().c_str());
        conn_ = conn;
        boolconn_ = true;
        // getMsgTimer(Serial_GETMAG_TIMER_INTERVAL);//定时获取缓冲区数据
        // getNextFrameTimer(timer_Interval_);//定时发送
        // if (newConnectionCallback_)
        // {
        //     newConnectionCallback_();
        // }
    }
    else
    {
        boolconn_ = false;
        // RequestTimer_stop();
        // GetDataTimer_stop();
        LOG_INFO("SerialPort onConnection DOWN: %s\n", conn->name().c_str());
        LOG_INFO("everyT Timer  closed...");
    }
}

void mySerialPort::onClose(const ConnectionPtr& conn)
{
    boolconn_ = false;
    // RequestTimer_stop();
    // GetDataTimer_stop();
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

void mySerialPort::originTimer()
{
    if(boolconn_)
    {
        getMsg(); // 定时获取缓冲区数据

        if(++timerValue_ >= nextFrameInterval_) // 定时发送下一帧数据
        {
            timerValue_ = 0;
            getNextFrame();
        }
    }
}