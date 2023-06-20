#include "IEC104Factory.h"


IEC104Factory::IEC104Factory(EventLoop *loop) : Factory(loop)
{

}

IEC104Factory::~IEC104Factory()
{

}

FramePtr IEC104Factory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<IEC104Frame>(gatewayConf);
    return tmp;
}

AnalysePtr IEC104Factory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<IEC104Analyse>();
    return tmp;
}
