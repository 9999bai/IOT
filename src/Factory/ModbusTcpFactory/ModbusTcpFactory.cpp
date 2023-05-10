#include "ModbusTcpFactory.h"

ModbusTcpFactory::ModbusTcpFactory(EventLoop *loop) : Factory(loop)
{

}

ModbusTcpFactory::~ModbusTcpFactory()
{

}

FramePtr ModbusTcpFactory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<ModbusTcpFrame>(gatewayConf);
    return tmp;
}

AnalysePtr ModbusTcpFactory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<ModbusTcpAnalyse>();
    return tmp;
}
