#pragma once

#include "Factory/Factory.h"
#include "NetSerial/mySerialPort/mySerialPort.h"
#include "Frame/ModbusRtuFrame/ModbusRtuFrame.h"
#include "Analyse/ModbusRtuAnalyse/ModbusRtuAnalyse.h"
#include "myHelper/myHelper.h"


class ModbusRtuFactory : public Factory
{
public:
    ModbusRtuFactory(EventLoop *loop);
    ~ModbusRtuFactory();

    FramePtr createFrame(const iot_gateway &gatewayConf);
    AnalysePtr createAnalyse();

private:

};