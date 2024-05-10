#include "Control.h"


Control::Control(const std::vector<iot_gateway>& v_gateway)
        : v_gateway_(v_gateway)
{

}

Control::~Control()
{

}

bool Control::findControlParam(const iot_data_item& item, iot_device& dest_device, iot_template& dest_templat, iot_sub_template& dest_subtemplat)
{
    for(auto gateway : v_gateway_)
    {
        if(item.gateway_id == gateway.gateway_id)
        {
            for(auto device : gateway.v_device)
            {
                if(item.device_id == device.device_id)
                {
                    for(auto templat : device.v_template)
                    {
                        if(0 == templat.param_id && 0 != templat.sub_template_id)
                        {
                            for(auto subtemplat : templat.v_sub_template)
                            {
                                if(item.param_id == subtemplat.param_id)
                                {
                                    // pro_name = gateway.pro_name;
                                    dest_device = device;
                                    // dest_templat = templat;
                                    // sub_status = true;
                                    dest_subtemplat = subtemplat;
                                    dest_templat.rw = enum_write;
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            if(item.param_id == templat.param_id)
                            {
                                // pro_name = gateway.pro_name;
                                dest_device = device;
                                dest_templat = templat;
                                dest_templat.rw = enum_write;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

