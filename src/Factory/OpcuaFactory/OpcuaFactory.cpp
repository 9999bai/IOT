#include "OpcuaFactory.h"

OpcUAFactory::OpcUAFactory(EventLoop *loop)
                : Factory(loop)
{

}

OpcUAFactory::~OpcUAFactory()
{

}

FramePtr OpcUAFactory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<OpcUAFrame>(gatewayConf);
    return tmp;
}

AnalysePtr OpcUAFactory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<OpcUAAnalyse>();
    return tmp;
}