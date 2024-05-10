#include "BacnetipControl/BacnetipControl.h"


BacnetipControl::BacnetipControl(const std::vector<iot_gateway>& v_gateway)
                : Control(v_gateway)
{

}

BacnetipControl::~BacnetipControl()
{

}

void BacnetipControl::ControlFunc(const iot_data_item &item, const std::vector<controlmediator>& v_controlmediator)
{
    iot_device device;
    iot_template templat;
    iot_sub_template sub_templat;
    enum_pro_name pro_name;

    if(findControlParam(item, device, templat, sub_templat))
    {
        controlBacnetIP(item.gateway_id, item.value, device, templat, v_controlmediator);
    }
    else
    {
        LOG_ERROR("BacnetioControl findModbusParam error...");
    }
}

void BacnetipControl::controlBacnetIP(int gateway_id, const std::string& value, const iot_device& device, iot_template& templat, const std::vector<controlmediator>& v_controlmediator)
{
    // write 固定头部
    frame data = HexStrToByteArray("81 0A 00 00 01 04 00 03 64 0F");

    ObjectIdentifier(templat, data);
    int propertyValue = std::atoi(templat.correct_mode.c_str());
    if (propertyValue >0 && propertyValue<= 0xFF)
    {
        data.emplace_back(0x19);
        data.emplace_back(propertyValue);
    }
    else if(propertyValue > 0xFF && propertyValue <= 0xFFFF)
    {
        data.emplace_back(0x1A);
        data.emplace_back(propertyValue >> 8);
        data.emplace_back(propertyValue & 0xFF);
    }

    data.emplace_back(0x3E);
    strToBacnetIPValue(value, templat.data_type, data);
    data.emplace_back(0x3F);

    data.emplace_back(0x49);    // write 优先级16 
    data.emplace_back(0x10);

    // 更新数据长度
    u_int16_t length = data.size();
    data[2] = length >> 8;
    data[3] = length & 0xFF;

    printFrame("Bacnetip Control --------------------- ", data);

    for (auto it = v_controlmediator.begin(); it != v_controlmediator.end(); it++)
    {
        if(gateway_id == it->first)
        {
            std::vector<iot_template> v_templat;
            v_templat.emplace_back(templat);
            nextFrame controlFrame(data, pair_frame(device, v_templat));
            // LOG_INFO("---- %d, %d, %s, %d, %d", device.gateway_id, device.device_id, device.device_addr.c_str(), device.template_id, templat.param_id);
            it->second->addControlFrame(controlFrame);
            return;
        }
    }
}

void BacnetipControl::strToBacnetIPValue(const std::string& value, enum_data_type valueType, frame& data)
{
    switch(valueType)
    {
        case enum_data_type_normal:      // null
            data.emplace_back(0x00);
            break;
        case enum_data_type_BA_bool:     // bool
        {
            int tmp = std::atoi(value.c_str());
            if(tmp == 0)
            {
                data.emplace_back(0x10); // false
            }
            else{
                data.emplace_back(0x11); // true
            }
            break;
        }
        case enum_data_type_BA_uint:     // uint
        {
            u_int32_t tmp = std::atoi(value.c_str());
            if(tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x21);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x22);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x23);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }else {
                data.emplace_back(0x24);
                data.emplace_back(tmp >> 24);
                data.emplace_back((tmp >> 16) & 0xFF);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case enum_data_type_BA_int:     // int
        {
            int32_t tmp = std::atoi(value.c_str());
            if(tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x31);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x32);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x33);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }else {
                data.emplace_back(0x34);
                data.emplace_back(tmp >> 24);
                data.emplace_back((tmp >> 16) & 0xFF);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case enum_data_type_BA_float:     // real
        {
            data.emplace_back(0x44);
            float tmp = std::atof(value.c_str());
            u_int32_t res;
            float_To_IEEE754(tmp, res);
            LOG_INFO("real tmp=%2f, u32=%d", tmp, (int)res);

            data.emplace_back(res >> 24);
            data.emplace_back((res >> 16) & 0xFF);
            data.emplace_back((res >> 8) & 0xFF);
            data.emplace_back(res & 0xFF);
            break;
        }
        case enum_data_type_BA_double:     // double
            break;
        case enum_data_type_BA_octet_string:     // Octet String 字节串
            break;
        case enum_data_type_BA_character_string:     // Character String 字符串
        {

            break;
        }
        case enum_data_type_BA_bit_string:     // Bit String 比特串
            break;
        case enum_data_type_BA_enum:     // 枚举
        {
            int tmp = std::atoi(value.c_str());
            if (tmp >= 0 && tmp <= 0xFF)
            {
                data.emplace_back(0x91);
                data.emplace_back(tmp);
            }else if(tmp > 0xFF && tmp <= 0xFFFF)
            {
                data.emplace_back(0x92);
                data.emplace_back(tmp >> 8);
                data.emplace_back(tmp & 0xFF);
            }else if(tmp > 0xFFFF && tmp <= 0xFFFFFF)
            {
                data.emplace_back(0x93);
                data.emplace_back(tmp >> 16);
                data.emplace_back((tmp >> 8) & 0xFF);
                data.emplace_back(tmp & 0xFF);
            }
            break;
        }
        case enum_data_type_BA_date:     // date
            break;
        case enum_data_type_BA_time:     // time
            break;
        default:
            break;
    }
}


