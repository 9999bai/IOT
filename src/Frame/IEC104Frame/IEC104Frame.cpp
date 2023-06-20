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
            if(templat.template_id == device.template_id)
            {
                if(templat.register_addr == "64")
                {
                    // mark_ |= 0x01;
                    // data.emplace_back(HexStrToByteArray(64 01 06 00 ));
                    data.emplace_back(0x64);    // 类型标识：64H（总召唤命令）
                    data.emplace_back(0x01);    // 可变结构限定词：01H
                    data.emplace_back(0x0600);  // 传送原因：0600（2个字节，激活）
                    u_int16_t tmp;
                    char2or4Hex_To_uint16or32(HexStrToByteArray(device.device_addr), tmp);
                    data.emplace_back(tmp & 0xFF00);//rtu
                    data.emplace_back(tmp & 0x00FF);
                    data.emplace_back(0x00);    // 信息体地址： 000000
                    data.emplace_back(0x00);    // 
                    data.emplace_back(0x00);    // 
                    data.emplace_back(0x14);    // 信息体元素：14H,为整个站的总召唤
                }
                else if(templat.register_addr == "65")//68 0e 00 00 00 00 65 01 06 00
                {
                    // mark_ |= 0x02;
                    data.emplace_back(0x65);    // 类型标识：65H（遥脉）
                    data.emplace_back(0x01);    // 可变结构限定词：01H
                    data.emplace_back(0x0600);  // 传送原因：0600（2个字节，激活）
                    u_int16_t tmp;
                    char2or4Hex_To_uint16or32(HexStrToByteArray(device.device_addr), tmp);
                    data.emplace_back(tmp & 0xFF00);//rtu
                    data.emplace_back(tmp & 0x00FF);
                    data.emplace_back(0x00);    // 信息体地址： 000000
                    data.emplace_back(0x00);    // 
                    data.emplace_back(0x00);    // 
                    data.emplace_back(0x00);    // 
                }else{
                    LOG_ERROR("IEC104 typeIdentity = %s", templat.register_addr.c_str());
                    continue;
                }
                nextFrame nextf(data, pair_frame(device, templat));
                R_Vector.emplace_back(nextf);
            }
        }
    }
}



