#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"

class Pro188Control : public  Control
{
public:
    Pro188Control(const std::vector<iot_gateway>& v_gateway);
    ~Pro188Control();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:


};