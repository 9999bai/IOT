#include "IEC104Control/IEC104Control.h"

IEC104Control::IEC104Control(const std::vector<iot_gateway>& v_gateway)
            : Control(v_gateway)
{

}

IEC104Control::~IEC104Control()
{
    
}

void IEC104Control::ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator)
{

}