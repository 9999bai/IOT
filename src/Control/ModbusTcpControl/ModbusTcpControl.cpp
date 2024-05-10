#include "ModbusTcpControl/ModbusTcpControl.h"

ModbusTcpControl::ModbusTcpControl(const std::vector<iot_gateway>& v_gateway)
                : Control(v_gateway)
{

}

ModbusTcpControl::~ModbusTcpControl()
{
    
}

void ModbusTcpControl::ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator)
{
    iot_device device;
    iot_template templat;
    iot_sub_template sub_templat;
    enum_pro_name pro_name;

    if(findControlParam(item, device, templat, sub_templat))
    {
        controlModbusTCP(item.gateway_id, item.value, device, templat, sub_templat, v_controlmediator);
    }
    else
    {
        LOG_ERROR("ModbusTcpControl findModbusParam error...");
    }
}

void ModbusTcpControl::controlModbusTCP(int gateway_id, const std::string& value, const iot_device& t_device, iot_template& templat, const iot_sub_template& sub_templat, const std::vector<controlmediator>& v_controlmediator)
{
    frame t_frame;
    static int identity = 0;
    if(identity >= 0xFF)
        identity = 0;

    templat.other = std::to_string(identity);
    uint16_To_char2(identity++, t_frame);        // 报文序列号
    uint16_To_char2(ModbusTcpIdentity, t_frame); // modbustcp标志
    
    switch((int)sub_templat.w_func)
    {
        case enum_w_func_0x05:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x06);
            t_frame.emplace_back((u_int8_t)std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            if(value == "0") // OFF
            {
                t_frame.emplace_back(0x00);
                t_frame.emplace_back(0x00);
                // uint16_To_char2(0, t_frame);
            }
            else // ON
            {
                t_frame.emplace_back(0xFF);
                t_frame.emplace_back(0x00);
                // uint16_To_char2(0xFF, t_frame);
            }
        }
            break;
        case enum_w_func_0x06:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x06);
            t_frame.emplace_back((u_int8_t)std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2(std::atoi(value.c_str()), t_frame);
        }
            break;
        case enum_w_func_0x0F:
        {
            
        }
            break;
        case enum_w_func_0x10:
        {
            t_frame.emplace_back(0x00);
            t_frame.emplace_back(0x0B);
            t_frame.emplace_back(std::atoi(t_device.device_addr.c_str())); //设备地址
            t_frame.emplace_back((int)sub_templat.w_func);   // 功能码
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            
            switch (sub_templat.data_type)
            {
                case enum_data_type_int16_hex:
                {
                    uint16_To_char2(1, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x02);  // 字节数
                    frame td;
                    union_uint16TOuchar data;
                    data.u16_data = std::atoi(value.c_str());
                    td.emplace_back(data.uchar_data[0]);
                    td.emplace_back(data.uchar_data[1]);
                    
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    break;
                case enum_data_type_int32_hex:
                {
                    uint16_To_char2(2, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x04);  // 字节数
                    frame td;
                    union_uchar4TOuint32 data;
                    data.u32_data = std::atoi(value.c_str());
                    // 大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    break;
                case enum_data_type_float:
                {
                    uint16_To_char2(2, t_frame); // 寄存器个数
                    t_frame.emplace_back(0x04);  // 字节数
                    frame td;
                    u_int32_t u32data;
                    float_To_IEEE754(std::atof(value.c_str()), u32data);
                    union_uchar4TOuint32 data;
	                data.u32_data = u32data;
                    //大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                    // BigEndianStrToU32Bit();
                    // LittleEndianStrToU32Bit();
                }
                    break;
            }
        }
            break;
        default:
        LOG_ERROR("GatewayManage::ModbusTcpControl error... w_func = %d ", (int)sub_templat.w_func);
    }
    printFrame("Control --------------------- ", t_frame);
    for (auto it = v_controlmediator.begin(); it != v_controlmediator.end(); it++)
    {
        if(gateway_id == it->first)
        {
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(t_frame, pair_frame(t_device, v_templat));
            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

