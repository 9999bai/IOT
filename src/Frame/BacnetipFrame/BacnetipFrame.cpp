#include "BacnetipFrame.h"

BacnetipFrame::BacnetipFrame(const iot_gateway& gatewayConf)
             : Frame(gatewayConf, 0, "bacnetip")
             , BVLC_(HexStrToByteArray("81 0A 00 00"))
             , NPDU_(HexStrToByteArray("01 04"))
             , pdutype_(0)
             , maxAPDUlength_(0x04)
             , invodeID_(11)
             , readMultiple_(0x0E)
             , openingContextTag_(0x1E)
             , closingContextTag_(0x1F)
{

}

BacnetipFrame::~BacnetipFrame()
{

}

frame BacnetipFrame::headFrame()
{
    frame tmp;
    tmp.insert(tmp.end(), BVLC_.begin(), BVLC_.end());
    tmp.insert(tmp.end(), NPDU_.begin(), NPDU_.end());
    tmp.emplace_back(pdutype_);
    tmp.emplace_back(maxAPDUlength_);
    tmp.emplace_back(invodeID_);
    tmp.emplace_back(readMultiple_);
    return tmp;
}

void BacnetipFrame::start()
{
    int count = 0;
    frame tmp = headFrame();
    std::vector<iot_template> v_templat;
    iot_device tmp_device;

    for(iot_device& device : gatewayConf_.v_device)
    {
        tmp_device = device;
        for (iot_template &templat : device.v_template)
        {
            ObjectIdentifier(templat, tmp);
            PropertyReferences(templat, tmp, count);
            templat.rw = enum_read;
            v_templat.emplace_back(templat);

            if (++count >= BACNETIPREADSIZE)
            {
                updateLength(tmp);
                nextFrame nextf(tmp, pair_frame(device, v_templat));
                R_Vector.emplace_back(nextf);

                tmp = headFrame();
                count = 0;
                v_templat.clear();

                LOG_INFO("BacnetipFrame ++ %d", (int)R_Vector.size());
            }
        }
    }
    if(count != 0)
    {
        updateLength(tmp);
        nextFrame nextf(tmp, pair_frame(tmp_device, v_templat));
        R_Vector.emplace_back(nextf);
        
        LOG_INFO("BacnetipFrame ++ %d", (int)R_Vector.size());
    }
}

void BacnetipFrame::PropertyReferences(const iot_template& templat, frame& data, int& count)
{
    data.emplace_back(openingContextTag_);
    propertyIdentifity(std::atoi(templat.correct_mode.c_str()), data);
    data.emplace_back(closingContextTag_);
}

void BacnetipFrame::propertyIdentifity(const int& readValue, frame& data)
{
    if(readValue > 0 && readValue <= 0xFF)
    {
        data.emplace_back(0x09);
        data.emplace_back(readValue);
    }
    else if(readValue > 0xFF && readValue <= 0xFFFF)
    {
        data.emplace_back(0x0A);
        data.emplace_back(readValue >> 8);
        data.emplace_back(readValue & 0xFF);
    }
    else if(readValue > 0xFFFF && readValue <= 0xFFFFFF)
    {
        data.emplace_back(0x0B);
        data.emplace_back(readValue >> 16);
        data.emplace_back((readValue >> 8) & 0xFF);
        data.emplace_back(readValue & 0xFF);
    }
}

void BacnetipFrame::updateLength(frame& tmp)
{
    u_int16_t length = tmp.size();
    tmp[2] = length >> 8;
    tmp[3] = length & 0xFF;
}

