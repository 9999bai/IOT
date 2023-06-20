#pragma once

#include "Frame/Frame.h"
#include "myHelper/myHelper.h"

class IEC104Frame : public Frame
{
public:
    IEC104Frame(const iot_gateway& gatewayConf);
    ~IEC104Frame();

    void start();


};
