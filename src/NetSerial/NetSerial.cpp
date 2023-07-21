#include "NetSerial.h"
#include "/usr/include/mymuduo/Buffer.h"
#include "/usr/include/mymuduo/TcpConnection.h"

NetSerial::NetSerial(EventLoop* loop) : loop_(loop) ,timerValue_(0), nextFrameInterval_(0)
{
    originTimerID_ = loop_->runEvery(Origin_Timer, std::bind(&NetSerial::originTimer, this));
}

NetSerial::~NetSerial() 
{

}

