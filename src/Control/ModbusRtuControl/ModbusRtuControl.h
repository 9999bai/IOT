#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"
#include "Mediator/Mediator.h"

class ModbusRtuControl : public  Control
{
public:
    ModbusRtuControl(const std::vector<iot_gateway>& v_gateway);
    ~ModbusRtuControl();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:
    void controlModbusRTU(int gateway_id, const std::string &value, const iot_device &device, iot_template &templat, const iot_sub_template &sub_templat, const std::vector<controlmediator>& v_controlmediator);

};