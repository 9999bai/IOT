#pragma once

#include "Factory/Factory.h"
#include "NetSerial/mySerialPort/mySerialPort.h"
#include "Frame/Pro188Frame/Pro188Frame.h"
#include "Analyse/Pro188Analyse/Pro188Analyse.h"
#include "myHelper/myHelper.h"


class Pro188Factory : public Factory
{
public:
    Pro188Factory(EventLoop *loop);
    ~Pro188Factory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();

private:

};