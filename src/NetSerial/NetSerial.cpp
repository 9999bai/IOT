#include "NetSerial.h"
#include "/usr/include/mymuduo/Buffer.h"
#include "/usr/include/mymuduo/TcpConnection.h"

NetSerial::NetSerial(EventLoop* loop) : loop_(loop) 
{
    // LOG_INFO("NetSerial::NetSerial() ctor...");
    
}

NetSerial::~NetSerial() 
{

}

