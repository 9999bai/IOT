#pragma once


#include "Factory/Factory.h"
#include "Frame/IEC104Frame/IEC104Frame.h"
#include "Analyse/IEC104Analyse/IEC104Analyse.h"
#include "myHelper/myHelper.h"


class IEC104Factory : public Factory
{
public:
    IEC104Factory(EventLoop *loop);
    ~IEC104Factory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();


};
