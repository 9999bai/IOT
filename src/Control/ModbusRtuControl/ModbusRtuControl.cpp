#include "ModbusRtuControl/ModbusRtuControl.h"

ModbusRtuControl::ModbusRtuControl(const std::vector<iot_gateway>& v_gateway)
                : Control(v_gateway)
{

}

ModbusRtuControl::~ModbusRtuControl()
{
    
}

void ModbusRtuControl::ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator)
{
    iot_device device;
    iot_template templat;
    iot_sub_template sub_templat;
    enum_pro_name pro_name;

    if(findControlParam(item, device, templat, sub_templat))
    {
        controlModbusRTU(item.gateway_id, item.value, device, templat, sub_templat, v_controlmediator);
    }
    else
    {
        LOG_ERROR("ModbusRTU findModbusParam error...");
    }
}

void ModbusRtuControl::controlModbusRTU(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const iot_sub_template& sub_templat, const std::vector<controlmediator>& v_controlmediator)
{
    frame t_frame;
    t_frame.emplace_back(atoi(device.device_addr.c_str()));
    t_frame.emplace_back((int)sub_templat.w_func);

    switch((int)sub_templat.w_func)
    {
        case enum_w_func_0x05:
        {
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            if(value == "0") // OFF
            {
                uint16_To_char2(0, t_frame);
            }
            else            // ON
            {
                uint16_To_char2(0xFF, t_frame);
            }
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]);//crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        case enum_w_func_0x06:
        {
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_frame);
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]); // crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        case enum_w_func_0x0F:
        {

        }
            break;
        case enum_w_func_0x10: // 这里只写一个寄存器
        {
            uint16_To_char2(std::atoi(sub_templat.register_addr.c_str()), t_frame);
            uint16_To_char2(2, t_frame); // 寄存器个数
            t_frame.emplace_back(0x04);  // 字节数
            switch (sub_templat.data_type)
            {
                case enum_data_type_int16_hex:
                {
                    frame td;
                    union_uint16TOuchar data;
                    data.u16_data = std::atoi(value.c_str());
                    td.emplace_back(data.uchar_data[0]);
                    td.emplace_back(data.uchar_data[1]);
                    
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                    // uint16_To_char2((u_int16_t)std::atoi(value.c_str()), t_data);
                }
                    break;
                case enum_data_type_int32_hex:
                {
                    frame td;
                    // u_int32_t u32data;
                    union_uchar4TOuint32 data;
                    data.u32_data = std::atoi(value.c_str());
                    // 大端
                    td.emplace_back(data.uchar_data[3]);
                    td.emplace_back(data.uchar_data[2]);
                    td.emplace_back(data.uchar_data[1]);
                    td.emplace_back(data.uchar_data[0]);
                    WriteBytetypeData(sub_templat.byte_order, td, t_frame);
                }
                    // uint32_To_char4((u_int32_t)std::atoi(value.c_str()), t_frame);
                    break;
                case enum_data_type_float:
                {
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
            u_int16_t crc = usMBCRC16(t_frame, t_frame.size());
            union_uint16TOuchar crctmp;
            crctmp.u16_data = crc;
            t_frame.emplace_back(crctmp.uchar_data[0]);//crc16校验 低地址在前
            t_frame.emplace_back(crctmp.uchar_data[1]);
        }
            break;
        default:
            LOG_ERROR("GatewayManage::ModbusRtuControl error... w_func = %d ", (int)sub_templat.w_func);
            break;
    }
    printFrame("Control --------------------- ", t_frame);
    for (auto it = v_controlmediator.begin(); it != v_controlmediator.end(); it++)
    {
        if(gateway_id == it->first)
        {
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(t_frame, pair_frame(device, v_templat));
            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}
