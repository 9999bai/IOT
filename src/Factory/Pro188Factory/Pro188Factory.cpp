#include "Pro188Factory.h"

Pro188Factory::Pro188Factory(EventLoop *loop)
                : Factory(loop)
{

}

Pro188Factory::~Pro188Factory()
{

}

FramePtr Pro188Factory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<Pro188Frame>(gatewayConf);
    return tmp;
}

AnalysePtr Pro188Factory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<Pro188Analyse>();
    return tmp;
}
