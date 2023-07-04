#include "BacnetipFactory.h"

BacnetipFactory::BacnetipFactory(EventLoop *loop)
                : Factory(loop)
{

}

BacnetipFactory::~BacnetipFactory()
{
    
}

FramePtr BacnetipFactory::createFrame(const iot_gateway &gatewayConf)
{
    FramePtr tmp = std::make_shared<BacnetipFrame>(gatewayConf);
    return tmp;
}

AnalysePtr BacnetipFactory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<BacnetipAnalyse>();
    return tmp;
}
