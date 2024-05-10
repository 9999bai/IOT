#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"

class IEC104Control : public  Control
{
public:
    IEC104Control(const std::vector<iot_gateway>& v_gateway);
    ~IEC104Control();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:


};