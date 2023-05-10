#pragma once

#include "Frame/Frame.h"
#include "myHelper/myHelper.h"


class Pro188Frame : public Frame
{
public:
    Pro188Frame(const iot_gateway& gatewayConf);
    ~Pro188Frame();

    void start();

private:

};