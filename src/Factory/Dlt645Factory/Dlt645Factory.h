#pragma once

#include "Factory/Factory.h"
#include "NetSerial/mySerialPort/mySerialPort.h"
#include "Frame/Dlt645Frame/Dlt645Frame.h"
#include "Analyse/Dlt645Analyse/Dlt645Analyse.h"
#include "myHelper/myHelper.h"

class Dlt645Factory : public Factory
{
public:
    Dlt645Factory(EventLoop *loop);
    ~Dlt645Factory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();


private:


};