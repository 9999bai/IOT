#pragma once
// #include "Database/Database.h"
#include "NetSerial/NetSerial.h"
#include "Analyse/Analyse.h"
#include "Frame/Frame.h"
#include "/usr/include/mymuduo/InetAddress.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "myHelper/myHelper.h"

class Factory
{
public:
    Factory(EventLoop *loop);
    virtual ~Factory();

    NetSerialPtr createNetSerial(const enum_netserial_type& type, const iot_gateway& gateway);
    ObserverPtr createObserver(const enum_observer_type& type);

    virtual FramePtr createFrame(const iot_gateway &gatewayConf) = 0;
    virtual AnalysePtr createAnalyse() = 0;


protected:
    EventLoop *loop_;
};