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
        frame tmp;
        tmp.emplace_back(0x68);
        frame addrHex = HexStrToByteArray(device.device_addr);
        for (auto it = addrHex.begin(); it != addrHex.end(); it++)
        {
            tmp.emplace_back(*it);
        }
        tmp.emplace_back(0x68);
        tmp.emplace_back(DLT645ControlCode);
        tmp.emplace_back(0x00); // 长度

        std::vector<iot_template> v_templat;
        int num = 0;
        frame data = tmp;

        for(iot_template& templat : device.v_template)
        {
            if(templat.priority != enum_priority_write)
            {
                templat.rw = enum_read;
                v_templat.emplace_back(templat);

                frame t_frame = HexStrToByteArray(templat.register_addr);
                for (auto it = t_frame.begin(); it != t_frame.end(); it++)
                {
                    data.emplace_back((u_char)*it + 0x33);
                }

                if(++num >= 5) // 每次请求5个数据项
                {
                    num = 0;
                    data[9] = 20; // 4*5
                    data.emplace_back(DLTCheck(data));
                    data.emplace_back(0x16);

                    nextFrame nextf(data, pair_frame(device, v_templat));
                    R_Vector.emplace_back(nextf);
                    printFrame("----", data);
                    LOG_INFO("Dlt645Frame frame ++ ");
                    v_templat.clear();
                    data = tmp;
                }
            }
        }
        data[9] = ((data.size() - 10) / 4 ) * 4; // 长度
        data.emplace_back(DLTCheck(data));
        data.emplace_back(0x16);

        nextFrame nextf(data, pair_frame(device, v_templat));
        R_Vector.emplace_back(nextf);
        printFrame("-----", data);
        LOG_INFO("Dlt645Frame frame ++ ");
    }
    LOG_INFO("Dlt645Frame  device size = %d, R_Vector.size = %d\n", (int)gatewayConf_.v_device.size(), (int)R_Vector.size());
}



// tmp.emplace_back((u_int16_t)templat.register_quantity);

// frame tframe = HexStrToByteArray(templat.register_addr);
// for (auto it = tframe.begin(); it != tframe.end(); it++)
// {
//     tmp.emplace_back((u_char)*it + 0x33);
// }
// tmp.emplace_back(DLTCheck(tmp));
// tmp.emplace_back(0x16);

// std::vector<iot_template> v_templat;
// v_templat.emplace_back(templat);

// nextFrame nextf(tmp, pair_frame(device, v_templat));
// R_Vector.emplace_back(nextf);
// LOG_INFO("Dlt645Frame frame ++ ");