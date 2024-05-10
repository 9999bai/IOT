#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"
#include "Mediator/Mediator.h"

class OpcuaControl : public  Control
{
public:
    OpcuaControl(const std::vector<iot_gateway>& v_gateway);
    ~OpcuaControl();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:
    void controlOPC(int gateway_id, const std::string &value, const iot_device &device, iot_template &templat, const std::vector<controlmediator> &v_controlmediator);

};