#pragma once

#include "myHelper/myHelper.h"

class Observer
{
public:
    Observer(EventLoop *loop, const mqtt_info& mqttconf);
    virtual ~Observer();

    void setObserverRecvCallback(ObserverRecvCallabck cb) { observerRecvCallabck_ = cb; }
    
    virtual void start() = 0;
    virtual void publicTopic(const std::string& msg) = 0;

protected:
    ObserverRecvCallabck observerRecvCallabck_;
    mqtt_info mqttconf_;
    EventLoop *loop_;
};