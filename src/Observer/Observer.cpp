#include "Observer.h"

Observer::Observer(EventLoop *loop, const mqtt_info& mqttconf) 
                    : loop_(loop)
                    , mqttconf_(mqttconf)
{

}

Observer::~Observer()
{

}