#include "OpcuaControl/OpcuaControl.h"

OpcuaControl::OpcuaControl(const std::vector<iot_gateway>& v_gateway)
            : Control(v_gateway)
{

}

OpcuaControl::~OpcuaControl()
{
    
}

void OpcuaControl::ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator)
{
    iot_device device;
    iot_template templat;
    iot_sub_template sub_templat;
    enum_pro_name pro_name;

    if(findControlParam(item, device, templat, sub_templat))
    {
        controlOPC(item.gateway_id, item.value, device, templat, v_controlmediator);
    }
    else
    {
        LOG_ERROR("OpcuaControl findModbusParam error...");
    }
}

void OpcuaControl::controlOPC(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const std::vector<controlmediator>& v_controlmediator)
{
    for (auto it = v_controlmediator.begin(); it != v_controlmediator.end(); it++)
    {
        if(gateway_id == it->first)
        {
            // LOG_INFO("****************%d", gateway_id);
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(frame(value.begin(), value.end()), pair_frame(device, v_templat));
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

    