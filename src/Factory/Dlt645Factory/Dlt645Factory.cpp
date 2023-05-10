#include "Dlt645Factory.h"

Dlt645Factory::Dlt645Factory(EventLoop *loop)
              :  Factory(loop)
{

}

Dlt645Factory::~Dlt645Factory()
{

}

FramePtr Dlt645Factory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<Dlt645Frame>(gatewayConf);
    return tmp;
}

AnalysePtr Dlt645Factory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<Dlt645Analyse>();
    return tmp;
}

