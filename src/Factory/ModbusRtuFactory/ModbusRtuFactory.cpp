#include "ModbusRtuFactory.h"


ModbusRtuFactory::ModbusRtuFactory(EventLoop* loop) : Factory(loop)
{
    // LOG_INFO("ModbusRtuFactory::ModbusRtuFactory ctor...");
}

ModbusRtuFactory::~ModbusRtuFactory()
{
    // LOG_INFO("ModbusRtuFactory::ModbusRtuFactory dtor..............");
}

FramePtr ModbusRtuFactory::createFrame(const iot_gateway& gatewayConf)
{
    FramePtr tmp = std::make_shared<ModbusRtuFrame>(gatewayConf);
    return tmp;
}

AnalysePtr ModbusRtuFactory::createAnalyse()
{
    AnalysePtr tmp = std::make_shared<ModbusRtuAnalyse>();
    return tmp;
}
