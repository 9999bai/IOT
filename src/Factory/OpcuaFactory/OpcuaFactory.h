#pragma once


#include "Factory/Factory.h"
#include "Frame/OpcuaFrame/OpcuaFrame.h"
#include "Analyse/OpcuaAnalyse/OpcuaAnalyse.h"
#include "myHelper/myHelper.h"


class OpcUAFactory : public Factory
{
public:
    OpcUAFactory(EventLoop *loop);
    ~OpcUAFactory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();

};
