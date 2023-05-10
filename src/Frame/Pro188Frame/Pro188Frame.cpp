#include "Pro188Frame.h"

Pro188Frame::Pro188Frame(const iot_gateway& gatewayConf)
            : Frame(gatewayConf, 0, "Pro188Frame")
{

}

Pro188Frame::~Pro188Frame()
{

}

void Pro188Frame::start()
{
    for (iot_device &device : gatewayConf_.v_device)
    {
        for(iot_template& templat : device.v_template)
        {
            if(templat.priority != enum_priority_write)
            {
                std::vector<char> tmp;
                tmp.emplace_back(0x68); // 起始
                tmp.emplace_back(0x10); // 10:水表
                frame addrHex = HexStrToByteArray(device.device_addr);
                for (auto it = addrHex.begin(); it != addrHex.end(); it++)
                {
                    tmp.emplace_back(*it);           // 表记地址
                }
                    
                tmp.emplace_back(templat.r_func); // 控制码
                tmp.emplace_back(templat.register_quantity); // 数据域长度

                frame ident = HexStrToByteArray(templat.register_addr);
                for (auto it = ident.begin(); it != ident.end(); it++)
                {
                    tmp.emplace_back(*it);           // 数据标识
                }
                tmp.emplace_back(0x00); // 序列号
                tmp.emplace_back(DLTCheck(tmp));
                tmp.emplace_back(0x16); // 结束符

                nextFrame nextf(tmp, pair_frame(device, templat));
                R_Vector.emplace_back(nextf);
                LOG_INFO("CJT188Frame frame ++ ");
            }
        }
    }
    LOG_INFO("CJT188Frame  device size = %d, R_Vector.size = %d\n", (int)gatewayConf_.v_device.size(), (int)R_Vector.size());
}