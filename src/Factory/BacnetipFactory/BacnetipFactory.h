#pragma once

#include "Factory/Factory.h"
#include "NetSerial/myUdpClient/myUdpClient.h"
#include "Frame/BacnetipFrame/BacnetipFrame.h"
#include "Analyse/BacnetipAnalyse/BacnetipAnalyse.h"
#include "myHelper/myHelper.h"

class BacnetipFactory : public Factory
{
public:
    BacnetipFactory(EventLoop *loop);
    ~BacnetipFactory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();
    
private:

};
