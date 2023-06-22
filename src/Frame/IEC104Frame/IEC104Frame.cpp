#include "IEC104Frame.h"


IEC104Frame::IEC104Frame(const iot_gateway& gatewayConf) 
            : Frame(gatewayConf, 0, "IEC104")
{

}

IEC104Frame::~IEC104Frame()
{

}

void IEC104Frame::start()
{
    for (iot_device &device : gatewayConf_.v_device)
    {
        frame data;
        data.emplace_back(0x68);
        data.emplace_back(0x0e);
        uint16_To_char2(0, data);
        uint16_To_char2(0, data);
        for (iot_template &templat : device.v_template)
        {
            frame tmp = data;
            if (templat.template_id == device.template_id)
            {
                if(templat.register_addr == "64")
                {
                    templat.rw = enum_read;
                    tmp.emplace_back(0x64);     // 类型标识：64H（总召唤命令）
                    tmp.emplace_back(0x01);     // 可变结构限定词：01H
                    tmp.emplace_back(0x06);     // 传送原因：0600（2个字节，激活）
                    tmp.emplace_back(0x00);

                    u_int16_t u16int = atoi(device.device_addr.c_str());
                    union_uint16TOuchar u16char;
                    u16char.u16_data = u16int;
                    tmp.emplace_back(u16char.uchar_data[0]); // rtu
                    tmp.emplace_back(u16char.uchar_data[1]);

                    tmp.emplace_back(0x00);    // 信息体地址： 000000
                    tmp.emplace_back(0x00);    // 
                    tmp.emplace_back(0x00);    // 
                    tmp.emplace_back(0x14);    // 信息体元素：14H,为整个站的总召唤
                }
                else if(templat.register_addr == "65")//68 0e 00 00 00 00 65 01 06 00
                {
                    templat.rw = enum_read;
                    tmp.emplace_back(0x65);     // 类型标识：65H（遥脉）
                    tmp.emplace_back(0x01);     // 可变结构限定词：01H
                    tmp.emplace_back(0x06);     // 传送原因：0600（2个字节，激活）
                    tmp.emplace_back(0x00);
                    
                    u_int16_t u16int = atoi(device.device_addr.c_str());
                    union_uint16TOuchar u16char;
                    u16char.u16_data = u16int;
                    tmp.emplace_back(u16char.uchar_data[0]); // rtu
                    tmp.emplace_back(u16char.uchar_data[1]);

                    tmp.emplace_back(0x00);    // 信息体地址： 000000
                    tmp.emplace_back(0x00);    // 
                    tmp.emplace_back(0x00);    // 
                    tmp.emplace_back(0x00);    // 
                }else{
                    LOG_ERROR("IEC104 typeIdentity = %s", templat.register_addr.c_str());
                    continue;
                }
                nextFrame nextf(tmp, pair_frame(device, templat));
                R_Vector.emplace_back(nextf);
            }
        }
    }
}



