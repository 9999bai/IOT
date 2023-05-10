#include "Dlt645Frame.h"

Dlt645Frame::Dlt645Frame(const iot_gateway& gatewayConf)
            : Frame(gatewayConf, 0, "Dlt645")
{

}

Dlt645Frame::~Dlt645Frame()
{

}

void Dlt645Frame::start()
{
    for (iot_device &device : gatewayConf_.v_device)
    {
        for(iot_template& templat : device.v_template)
        {
            if(templat.priority != enum_priority_write)
            {
                std::vector<char> tmp;
                tmp.emplace_back(0x68);
                frame addrHex = HexStrToByteArray(device.device_addr);
                for (auto it = addrHex.begin(); it != addrHex.end(); it++)
                {
                    tmp.emplace_back(*it);
                }

                tmp.emplace_back();
                tmp.emplace_back(0x68);
                tmp.emplace_back(DLT645ControlCode);
                tmp.emplace_back(templat.register_quantity);

                frame tframe = HexStrToByteArray(templat.register_addr);
                for (auto it = tframe.begin(); it != tframe.end(); it++)
                {
                    tmp.emplace_back((u_char)*it + 0x33);
                }
                tmp.emplace_back(DLTCheck(tmp));
                tmp.emplace_back(0x16);

                nextFrame nextf(tmp, pair_frame(device, templat));
                R_Vector.emplace_back(nextf);
                LOG_INFO("Dlt645Frame frame ++ ");
            }
        }
    }
    LOG_INFO("Dlt645Frame  device size = %d, R_Vector.size = %d\n", (int)gatewayConf_.v_device.size(), (int)R_Vector.size());
}
