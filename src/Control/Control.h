#pragma once

#include "myHelper/myHelper.h"

class Control
{
public:
    Control(const std::vector<iot_gateway>& v_gateway);
    virtual ~Control();

    virtual void ControlFunc(const iot_data_item& item, const std::vector<controlmediator>& v_controlmediator) = 0;

protected:
    bool findControlParam(const iot_data_item &item, iot_device &dest_device, iot_template &dest_templat, iot_sub_template &dest_subtemplat);

    std::vector<iot_gateway> v_gateway_;
};