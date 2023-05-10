#pragma once

#include "Factory/Factory.h"
#include "Frame/ModbusTcpFrame/ModbusTcpFrame.h"
#include "Analyse/ModbusTcpAnalyse/ModbusTcpAnalyse.h"
#include "myHelper/myHelper.h"


class ModbusTcpFactory : public Factory
{
public:
    ModbusTcpFactory(EventLoop *loop);
    ~ModbusTcpFactory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();

private:

};
