#pragma once

#include "myHelper/myHelper.h"
#include "Control/Control.h"

class Dlt645Control : public  Control
{
public:
    Dlt645Control(const std::vector<iot_gateway>& v_gateway);
    ~Dlt645Control();

    void ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator);

private:


};