#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"
#include "Mediator/Mediator.h"

class BacnetipControl : public Control
{
public:
    BacnetipControl(const std::vector<iot_gateway>& v_gateway);
    ~BacnetipControl();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:
    
    void controlBacnetIP(int gateway_id, const std::string &value, const iot_device &device, iot_template &templat, const std::vector<controlmediator> &v_controlmediator);
    void strToBacnetIPValue(const std::string &value, enum_data_type valueType, frame &data);
};