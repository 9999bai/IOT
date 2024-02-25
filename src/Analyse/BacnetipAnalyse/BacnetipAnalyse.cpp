#include "BacnetipAnalyse.h"

BacnetipAnalyse::BacnetipAnalyse()
{

}

BacnetipAnalyse::~BacnetipAnalyse()
{

}

void BacnetipAnalyse::AnalyseFunc(const std::string &msg, const nextFrame &nextframe)
{
    LOG_INFO("BacnetipAnalyse::AnalyseFunc --start--");
    v_data.insert(v_data.end(), msg.begin(), msg.end());

    if(BACNETIPAnalyeFrame_Minsize > v_data.size())
    {
        return;
    }

    bool resOK = false; // 解析是否成功
    enum_RW resRW;
    int index = 0;
    iot_device device = nextframe.second.first;
    std::vector<iot_template> v_templat = nextframe.second.second;
    if(v_templat.size() < 0)
    {
        LOG_FATAL("BacnetipAnalyse::AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);

    if(templat.rw == enum_read) // 读语句返回 解析
    {
        resRW = enum_read;
        try
        {
            while(index < v_data.size())
            {
                // BVLC
                u_int16_t len = 0;
                if(v_data.at(index) == 0x81 && v_data.at(index+1) == 0x0A || v_data.at(index+1) == 0x0B)
                {
                    index += 2;
                    char2or4Hex_To_uint16or32(frame(v_data.begin() + index, v_data.begin() + index + 2), len);
                    index += 2;
                    if(len > (v_data.size()- index + 4))
                    {
                        LOG_ERROR("BacnetIP 接收不完整...len=%d, data.size=%d", (int)len, (int)v_data.size());
                        return;
                    }

                    // NPDU
                    if(v_data.at(index) == 0x01) // 版本号
                    {
                        index++;
                        if(v_data.at(index) == 0x00)
                        {
                            index++;
                            AnalyseAPDU(v_data, index, len, device, v_templat);
                            resOK = true;
                        }
                        else
                        {
                            AnalyseNPDU_Control(v_data, index);
                            AnalyseAPDU(v_data, index, len, device, v_templat);
                            resOK = true;
                        }
                    }else{
                        LOG_ERROR("版本号error... %d", (int)v_data.at(index));
                    }
                }
                else
                {
                    LOG_ERROR("BacnetIP BVLC error...");
                    v_data.clear();
                    resOK = false;
                }
            }
            v_data.clear();
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("BacnetipAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("BacnetipAnalyse::Analyse  error other...");
            v_data.clear();
            resOK = false;
        }
    }
    else if(templat.rw == enum_write)
    {
        resRW = enum_write;
        try
        {
            while(index < v_data.size())
            {
                // BVLC
                u_int16_t len = 0;
                if(v_data.at(index) == 0x81 && v_data.at(index+1) == 0x0A || v_data.at(index+1) == 0x0B)
                {
                    index += 2;
                    char2or4Hex_To_uint16or32(frame(v_data.begin() + index, v_data.begin() + index + 2), len);
                    index += 2;
                    if(len > (v_data.size()- index + 4))
                    {
                        LOG_ERROR("BacnetIP 接收不完整...len=%d, data.size=%d", (int)len, (int)v_data.size());
                        return;
                    }

                    // NPDU
                    if(v_data.at(index) == 0x01) // 版本号
                    {
                        index++;
                        if(v_data.at(index) == 0x00)
                        {
                            index++;
                            AnalyseAPDU(v_data, index, len, device, v_templat);
                            resOK = true;
                        }
                        else
                        {
                            AnalyseNPDU_Control(v_data, index);
                            AnalyseAPDU(v_data, index, len, device, v_templat);
                            resOK = true;
                        }
                    }else{
                        LOG_ERROR("版本号error... %d", (int)v_data.at(index));
                    }
                }
                else
                {
                    LOG_ERROR("BacnetIP BVLC error...");
                    v_data.clear();
                    resOK = false;
                }
            }
            v_data.clear();
        }
        catch(std::out_of_range)//数组越界
        {
            LOG_ERROR("BacnetipAnalyse::Analyse  error out_of_range...");
            v_data.clear();
            resOK = false;
        }
        catch(...)
        {
            LOG_ERROR("BacnetipAnalyse::Analyse  error ...");
            v_data.clear();
            resOK = false;
        }
        v_data.clear();
    }
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame/*std::pair<int, IEC104FrameType>(0, ENUM_Normal_Frame)*/);
    }
}

void BacnetipAnalyse::AnalyseNPDU_Control(const frame& data, int& index)
{
    bool messageTypeBool = false;
    bool hopCountBool = false;
    u_int8_t control = data.at(index);
    index++;

    if ((control & 0x80) == 0x80)
    {
        messageTypeBool = true;
    }
    if((control & 0x20) == 0x20)
    {
        hopCountBool = true;
        u_int16_t dnet = 0;
        char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 2), dnet);
        index += 2;
        if (dnet == 0xFFFF)
        {
            LOG_INFO("目标网络号DNET=FFFF, 广播消息...");
        }
        else
        {
            LOG_INFO("目标网络号DNET=%X, 广播消息...", dnet);
        }
        u_int8_t dlen = v_data.at(index);
        LOG_INFO("目标网络长度=%d...", dlen);
        frame dadr(data.begin() + index, data.begin() + index + dlen);
        index += dlen;
        if(dlen != 0)
        {
            printFrame("目标地址DADR", dadr);
        }
    }
    if((control & 0x08) == 0x08)
    {
        u_int16_t SNET = 0;
        char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 2), SNET);
        index += 2;
        LOG_INFO("原网络号SNET=%X", SNET);
        u_int8_t slen = data.at(index);
        index++;
        LOG_INFO("原网络长度SLEN=%X", slen);
        frame SADR(frame(data.begin() + index, data.begin() + index + slen));
        index += slen;
        if (slen != 0)
        {
            printFrame("原地址SADR=", SADR);
        }
    }
    if(hopCountBool)
    {
        u_int8_t hopCount = data.at(index);
        index++;
        LOG_INFO("HopCount=%d", (int)hopCount);
    }
    if(messageTypeBool)
    {
        u_int8_t messageType = data.at(index);
        LOG_INFO("MessageType=%d", (int)messageType);
        if(messageType >= 0x80 && messageType <= 0xFF)
        {
            u_int16_t VendorID = 0;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 2), VendorID);
            index += 2;
            LOG_INFO("VendorID=%X", VendorID);
        }
    }
}

void BacnetipAnalyse::AnalyseAPDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    u_int8_t pdu = data.at(index) >> 4;
    switch(pdu)
    {
    case 0:
        LOG_INFO("有证实服务 Confirmed_Request_PDU...");
        BA_Confirmed_Request_PDU(data,index, len, device, v_templat);
        break;
    case 1:
        LOG_INFO("无证实服务 Unconfirmed_Request_PDU...");
        BA_Unconfirmed_Request_PDU(data, index, len, device, v_templat);
        break;
    case 2:
        LOG_INFO("简单确认 SimpleACK_PDU...");
        BA_SimpleACK_PDU(data,index, len, device, v_templat);
        break;
    case 3:
        LOG_INFO("复杂确认 ComplexACK_PDU...");
        BA_ComplexACK_PDU(data,index,len, device, v_templat);
        break;
    case 4:
        LOG_INFO("分段确认 SegmentACK_PDU...");
        BA_SegmentACK_PDU(data,index, len, device, v_templat);
        break;
    case 5:
        LOG_INFO("差错 Error_PDU...");
        BA_Error_PDU(data,index, len, device, v_templat);
        break;
    case 6:
        LOG_INFO("拒绝pdu Reject_PDU...");
        BA_Reject_PDU(data,index, len, device, v_templat);
        break;
    case 7:
        LOG_INFO("中止pdu Abort_PDU...");
        BA_Abort_PDU(data,index, len, device, v_templat);
        break;
    default:
        LOG_INFO("default pdu = %d...", (int)pdu);
        break;
    }
}

void BacnetipAnalyse::BA_Confirmed_Request_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    int param = data.at(index) & 0x0F;
    index++;
    int seg = param & 0x08;
    int mor = param & 0x04;
    int sa = param & 0x02;

    int maxResp = data.at(index) & 0x0F;
    index++;
    int InvokeID = data.at(index);
    index++;

    if(seg == 1)
    {
        int sequenceNumber = data.at(index);
        index++;
        int proposedWindowSize = data.at(index);
        index++;
    }
    int serviceChoice = data.at(index);
    index++;
    AnalyseServiceChioce(data, index, serviceChoice, len, device, v_templat);
}

void BacnetipAnalyse::BA_Unconfirmed_Request_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    index++;
    int serviceChoice = data.at(index);
    index++;
    AnalyseServiceChioce(data, index, serviceChoice, len, device, v_templat);
}

void BacnetipAnalyse::BA_SimpleACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    index++;
    int InokeID = data.at(index);
    index++;
    int serviceChoice = data.at(index);
    index++;
    AnalyseServiceChioce(data, index, serviceChoice, len, device, v_templat);
}

void BacnetipAnalyse::BA_ComplexACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    int param = data.at(index) & 0x0F;
    index++;
    int seg = param & 0x08;
    int mor = param & 0x04;

    int invokeID = data.at(index);
    index++;
    if (seg == 1)
    {
        int seqNumber = data.at(index);
        index++;
        int windowSize = data.at(index);
        index++;
    }

    int serviceChoice = data.at(index);
    index++;
    AnalyseServiceChioce(data, index, serviceChoice, len, device, v_templat);
}

void BacnetipAnalyse::BA_SegmentACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    int param = data.at(index);
    int srv = param & 0x01;
    int nak = param & 0x02;
    index++;

    int InvokeID = data.at(index);
    index++;

    int seqNumber = data.at(index);
    index++;

    int windowSize = data.at(index);
    index++;
}

void BacnetipAnalyse::BA_Error_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    index++;
    int invokeID = data.at(index);
    // int serviceChoice = data.at(index);
    // index++;
    // AnalyseServiceChioce(data, index, serviceChoice, len, device, templat);
}

void BacnetipAnalyse::BA_Reject_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    index++;
    int InvokeID = data.at(index);
    index++;

    int RejectReason = data.at(index);
    index++;

    switch(RejectReason)
    {
        case 0:
            LOG_INFO("rejectReason: other");
            break;
        case 1:
            LOG_INFO("rejectReason: buffer-overflow ");
            break;
        case 2:
            LOG_INFO("rejectReason: inconsistent-Parameters ");
            break;
        case 3:
            LOG_INFO("rejectReason: invalid-Parameter-datatype");
            break;
        case 4:
            LOG_INFO("rejectReason: invalid-tag ");
            break;
        case 5:
            LOG_INFO("rejectReason: missing-required-parameter");
            break;
        case 6:
            LOG_INFO("rejectReason: parameter-out-of range 	");
            break;
        case 7:
            LOG_INFO("rejectReason: too-man-arguments ");
            break;
        case 8:
            LOG_INFO("rejectReason: undefined-enumeration ");
            break;
        case 9:
            LOG_INFO("rejectReason: unrecognized-service ");
            break;
    }
}

void BacnetipAnalyse::BA_Abort_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    int srv = data.at(index) & 0x01;
    index++;

    int InvokeID = data.at(index);
    index++;

    int AbortReason = data.at(index);
    index++;

    switch (AbortReason)
    {
        case 0:
            LOG_INFO("AbstractReason: Other");
            break;
        case 1:
            LOG_INFO("AbstractReason: buffer-overflow ");
            break;
        case 2:
            LOG_INFO("AbstractReason: invalid-apdu-in-this-state");
            break;
        case 3:
            LOG_INFO("AbstractReason: preempted-by-higher-Priority-task");
            break;
        case 4:
            LOG_INFO("AbstractReason: segmentation-not-supported");
            break;
        default:
            LOG_INFO("AbstractReason: default = %d", AbortReason);
            break;
    }
}

void BacnetipAnalyse::AnalyseServiceChioce(const frame& data, int& index, int& serviceChoice, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    switch(serviceChoice)
    {
        case 0:     // I-AM
            LOG_INFO("I-AM Request...");
            Analyse_I_AM_Request(data, index, len, device, v_templat);
            break;
        case 12:    // readProperty
            LOG_INFO("ReadProperty_Request...");
            AnalyseReadProperty_Request(data, index, len, device, v_templat);
            break;
        case 14:    // readPropertyMutiple
            LOG_INFO("ReadPropertyMutiple_Request...");
            AnalyseReadPropertyMutiple_Request(data, index, len, device, v_templat);
            break;
        case 15:    // writeProperty
            LOG_INFO("WriteProperty_Request...");
            AnalyseWriteProperty_Request(data, index, len, device, v_templat);
            break;
        case 16:    // writePropertyMutiple
            LOG_INFO("WritePropertyMutiple_Request...");
            AnalyseWritePropertyMutiple_Request(data, index, len, device, v_templat);
            break;
        default:
            LOG_INFO("default serviceChoice = %d", serviceChoice);
            break;
    }
}

void BacnetipAnalyse::Analyse_I_AM_Request(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    // 对象类型-实例号
    BacnetIP_ObjectIdentifity object = AnalyseObject(data, index);

    BacnetIP_Tag apduTag = AnalyseTag(data, index);
    if(apduTag.classType == enum_ApplicationTag)
    {
        // APDU 最大长度
        int apduMaxlength = 0;
        char2or4Hex_To_uint16or32(frame(data.begin()+index, data.begin()+index+apduTag.tvlValue), apduMaxlength);
        index += apduTag.tvlValue;
        LOG_INFO("apduMaxLength = %d", apduMaxlength);
    }
    BacnetIP_Tag segTag = AnalyseTag(data, index);
    if(segTag.classType == enum_ApplicationTag)
    {
        // 分段支持
        int seg = data.at(index);
        index++;
        LOG_INFO("分段=%d (0=都支持, 1=发送支持, 2=接收支持, 3=都不支持)", seg);
    }
    BacnetIP_Tag idTag = AnalyseTag(data, index);
    // index++;
    if(idTag.classType == enum_ApplicationTag)
    {
        // vendorID
        int vendorID = 0;
        char2or4Hex_To_uint16or32(frame(data.begin()+index, data.begin()+index+idTag.tvlValue), vendorID);
        index += idTag.tvlValue;
        LOG_INFO("VendorID=%d", vendorID);
    }
}

void BacnetipAnalyse::AnalyseReadProperty_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    BacnetIP_ObjectIdentifity object = AnalyseObject(data, index);
    LOG_INFO("object=%d, instance=%d", object.objectType, object.InstanceNumber);

    BacnetIP_Tag Tag = AnalyseTag(data, index);
    if(Tag.classType == enum_ContextTag && Tag.tvlType == enum_PrimitiveData)
    {
        int prepertyValue = AnalysePropertyIdentifier(data, index, Tag);
        AnalysePropertyValue(data, index, object, prepertyValue, device, v_templat);
    }
}

void BacnetipAnalyse::AnalyseReadPropertyMutiple_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    int count = 0;
    while(index < len)
    {
        BacnetIP_ObjectIdentifity object = AnalyseObject(data, index);
        LOG_INFO("object=%d, instance=%d", object.objectType, object.InstanceNumber);

        BacnetIP_Tag Tag = AnalyseTag(data, index);
        if(isOpening(Tag)) // opening
        {
            AnalyseListofResults(data, index, object, device, v_templat);
        }
    }
}

void BacnetipAnalyse::AnalyseListofResults(const frame &data, int &index, BacnetIP_ObjectIdentifity object, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    BacnetIP_Tag nextTag = AnalyseTag(data, index);
    if(isClosing(nextTag))
    {
        LOG_INFO("当前object 解析完成............\n");
        return;
    }
    else if(nextTag.classType == enum_ContextTag && nextTag.tvlType == enum_PrimitiveData)
    {
        int prepertyValue = AnalysePropertyIdentifier(data, index, nextTag);            // propertyIdentifity 
        AnalysePropertyValue(data, index, object, prepertyValue, device, v_templat);    // propertyValue

        AnalyseListofResults(data, index, object, device, v_templat);
    }
}

void BacnetipAnalyse::AnalyseWriteProperty_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    LOG_INFO("writeProperty suc...........%d %d", data.size(), index);
}

void BacnetipAnalyse::AnalyseWritePropertyMutiple_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat)
{

}

BacnetIP_Tag BacnetipAnalyse::AnalyseTag(const frame &data, int &index)
{
    BacnetIP_Tag tag;
    u_int8_t u8 = data.at(index);
    index++;
    if ((u8 & 0x08) == 0x08)
    {
        tag.classType = enum_ContextTag;
        u_int8_t tagNumber = u8 >> 4;
        AnalyseTagNumber(data, index, tagNumber, tag.tagNumber);
        u_int8_t tvl = u8 & 0x07;
        AnalyseTVL(data, index, tvl, tag);
    }
    else
    {
        tag.classType = enum_ApplicationTag;
        u_int8_t tagNumber = u8 >> 4;
        AnalyseTagNumber(data, index, tagNumber, tag.tagNumber);
        u_int8_t tvl = u8 & 0x07;
        AnalyseTVL(data, index, tvl, tag);
    }
    return tag;
}

void BacnetipAnalyse::AnalyseTagNumber(const frame&data, int& index, u_int8_t& src, u_int8_t& dest)
{
    if(src >= 0 && src <= 14) // 应用类型 0-12
    {
        dest = src;
    }
    else if(src == 0x0F) // 有扩展标记编码
    {
        dest = data.at(index);
        index++;
    }
}

void BacnetipAnalyse::AnalyseTVL(const frame& data, int& index, u_int8_t tvl, BacnetIP_Tag& tag)
{
    tag.tvlType = enum_PrimitiveData;
    if (tvl >= 0 && tvl <= 4)
    {
        tag.tvlValue = tvl;
    }
    else if(tvl == 0x05)
    {
        if(data.at(index) < 0xFE) // 5-253
        {
            tag.tvlValue = data.at(index);
            index++;
        }
        else if(data.at(index) == 0xFE) // 254-65535
        {
            index++;
            u_int16_t tmpValue = 0;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 2), tmpValue);
            tag.tvlValue = tmpValue;
            index += 2;
        }
        else if(data.at(index) == 0xFF) // 65536 - (2^23 -1)
        {
            index++;
            u_int32_t tmpValue = 0;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 4), tmpValue);
            tag.tvlValue = tmpValue;
            index += 4;
        }else {
            LOG_ERROR("BacnetIP AnalyseTag error...");
        }
    }
    else if(tvl == 0x06)
    {
        tag.tvlType = enum_ConstructedData_opening;
    }
    else if(tvl == 0x07)
    {
        tag.tvlType = enum_ConstructedData_closing;
    }
}

BacnetIP_ObjectIdentifity BacnetipAnalyse::AnalyseObject(const frame& data, int& index)
{
    BacnetIP_Tag nextTag = AnalyseTag(data, index);
    BacnetIP_ObjectIdentifity object;

    if(nextTag.classType == enum_ContextTag && nextTag.tvlType == enum_PrimitiveData) // 0x0C
    {
        u_int32_t u32 = 0;
        char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + nextTag.tvlValue), u32);
        index += nextTag.tvlValue;

        object.objectType = u32 >> 22;
        object.InstanceNumber = u32 & 0x3FFFFF;
    }
    else if(nextTag.classType == enum_ApplicationTag && nextTag.tagNumber == 0x09) // 0x91 error-class
    {
        int errorClass = data.at(index);
        prientErrorClass(errorClass);
        AnalyseErrorCode(data, index);  // error-code
    }

    return object;
}

int BacnetipAnalyse::AnalysePropertyIdentifier(const frame& data, int& index, const BacnetIP_Tag& tag)
{
    int propertyIdentifier = 0;
    if(tag.tvlValue == 1)
    {
        propertyIdentifier = data.at(index);
        index++;
    }
    else if(tag.tvlValue == 2)
    {
        char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + 2), propertyIdentifier);
        index += 2;
    }
    LOG_INFO("propertyValue=%d", propertyIdentifier);
    return propertyIdentifier;
}

void BacnetipAnalyse::AnalysePropertyValue(const frame& data, int& index, BacnetIP_ObjectIdentifity object,int& prepertyValue, const iot_device& device, const std::vector<iot_template>& v_templat)
{
    BacnetIP_Tag openTag = AnalyseTag(data, index);\
    if(isOpening(openTag) && openTag.tagNumber == 5) // error
    {
        int errorClass = AnalyseErrorClass(data, index);
        int errorCode = AnalyseErrorCode(data, index);
    }
    else if(isOpening(openTag)) // 4E  opening
    {
        BacnetIP_Tag applicationTag = AnalyseTag(data, index);
        if(applicationTag.classType == enum_ApplicationTag)
        {
            std::string value = AnalyseValue(data, index, applicationTag);
            iot_template res;
            if(findParam(object, v_templat, res)) // 匹配templat.sub_templat中的参数用于传输
            {
                // 入队列
                QueueData(res.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, res.param_id, res.param_name, value));
            }else{
                LOG_ERROR("findParam = false...");
            }
        }
    }

    BacnetIP_Tag closeTag = AnalyseTag(data, index);
    if(isClosing(closeTag)) // 4F
    {
        // LOG_INFO("BacnetIP AnalyseProperty_value closing...\n");
    }
}

std::string BacnetipAnalyse::AnalyseValue(const frame& data, int& index, BacnetIP_Tag& applicationTag)
{
    std::string resValue;
    switch (applicationTag.tagNumber)
    {
        case 1:     // bool
        {
            bool value;
            applicationTag.tvlValue == 0 ? (value = false) : (value = true);
            resValue = (value == true) ? (true) : (false);
            LOG_INFO("BacnetIP AnalyseValue bool value=%d", (int)value);
            break;
        }
        case 2:     // uint
        {
            u_int32_t value;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + applicationTag.tvlValue), value);
            index += applicationTag.tvlValue;
            resValue = std::to_string(value);
            LOG_INFO("BacnetIP AnalyseValue uint value=%d", (int)value);
            break;
        }
        case 3:     // int
        {
            int value;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + applicationTag.tvlValue), value);
            index += applicationTag.tvlValue;
            resValue = std::to_string(value);
            LOG_INFO("BacnetIP AnalyseValue int value=%d", value);
            break;
        }
        case 4:     // IEEE754-float
        {
            float value;
            u_int32_t u32_data;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + applicationTag.tvlValue), u32_data);
            IEEE754_To_float(u32_data, value);
            index += applicationTag.tvlValue;
            resValue = floatToString(value);
            LOG_INFO("BacnetIP AnalyseValue float value=%f", value);
            break;
        }
        case 5:     // IEEE754-double
        {
            break;
        }
        case 6:     // 字节串
        {
            std::string value;
            value.append(data.begin() + index, data.begin() + index + applicationTag.tvlValue);
            index += applicationTag.tvlValue;
            resValue = value;
            LOG_INFO("BacnetIP AnalyseValue 字节串 value=%s", value.c_str());
            break;
        }
        case 7:     // 字符串
        {
            int length = applicationTag.tvlValue; // 长度
            LOG_INFO("字符串长度=%d", length);

            int characterSet = data.at(index); // 字符集 
            LOG_INFO("字符集=%d", characterSet);
            
            std::string value;
            for (int i = 1; i < length; i++)
            {
                value.push_back(*(data.begin() + index + i));
            }
            index += length;
            resValue = value;
            LOG_INFO("BacnetIP AnalyseValue 字符串 value.length=%d, value=%s", value.size(), value.c_str());
            break;
        }
        case 8:     // 比特位串
            break;
        case 9:     // 枚举
        {
            int value;
            value = data.at(index);
            index++;
            resValue = std::to_string(value);
            LOG_INFO("BacnetIP AnalyseValue 枚举 value=%d", value);
            break;
        }
        case 10:    // 日期
            break;
        case 11:    // 时间
            break;
        case 12:    // 对象标识
        {
            BacnetIP_ObjectIdentifity object;
            u_int32_t u32 = 0;
            char2or4Hex_To_uint16or32(frame(data.begin() + index, data.begin() + index + applicationTag.tvlValue), u32);
            index += applicationTag.tvlValue;

            object.objectType = u32 >> 22;
            object.InstanceNumber = u32 & 0x3FFFFF;
            break;
        }
        default:
            LOG_ERROR("BacnetIP applicationTag.tagNumber error...=%d", (int)applicationTag.tagNumber);
            break;
    }
    return resValue;
}

bool BacnetipAnalyse::findParam(const BacnetIP_ObjectIdentifity& object, const std::vector<iot_template>& v_templat, iot_template& res)
{
    char buf[10] = {0};
    sprintf(buf, "%d-%d", object.objectType, object.InstanceNumber);

    for (iot_template templat : v_templat)
    {
        if(std::string(buf) == templat.register_addr)
        {
            res = templat;
            return true;
        }
    }
    return false;
}

bool BacnetipAnalyse::isOpening(const BacnetIP_Tag& tag)
{
    if(tag.classType == enum_ContextTag && tag.tvlType == enum_ConstructedData_opening)
    {
        return true;
    }
    return false;
}

bool BacnetipAnalyse::isClosing(const BacnetIP_Tag& tag)
{
    if(tag.classType == enum_ContextTag && tag.tvlType == enum_ConstructedData_closing)
    {
        return true;
    }
    return false;
}

int BacnetipAnalyse::AnalyseErrorClass(const frame& data, int& index)
{
    int errorClass = -1;
    BacnetIP_Tag errorTag = AnalyseTag(data, index);
    if(errorTag.classType == enum_ApplicationTag)
    {
        errorClass = data.at(index);
        index++;
        prientErrorClass(errorClass);
    }
    return errorClass;
}

int BacnetipAnalyse::AnalyseErrorCode(const frame& data, int& index)
{
    int errorCode = -1;
    BacnetIP_Tag errorTag = AnalyseTag(data, index);
    if(errorTag.classType == enum_ApplicationTag)
    {
        errorCode = data.at(index);
        index++;
        prientErrorCode(errorCode);
    }
    return errorCode;
}

void BacnetipAnalyse::prientErrorClass(int& errorClass)
{
    switch(errorClass)
        {
            case 0:
                LOG_INFO("errorClass=%d : device", errorClass);
                break;
            case 1:
                LOG_INFO("errorClass=%d : object", errorClass);
                break;
            case 2:
                LOG_INFO("errorClass=%d : property", errorClass);
                break;
            case 3:
                LOG_INFO("errorClass=%d : resources", errorClass);
                break;
            case 4:
                LOG_INFO("errorClass=%d : security", errorClass);
                break;
            case 5:
                LOG_INFO("errorClass=%d : service", errorClass);
                break;
            case 6:
                LOG_INFO("errorClass=%d : vt", errorClass);
                break;
            default:
                LOG_INFO("errorClass : defaul=%d", errorClass);
                break;
            }
}

void BacnetipAnalyse::prientErrorCode(int& errorCode)
{
    switch(errorCode)
    {
        case 31:
            LOG_INFO("errorCode=%d : unknown - object", errorCode);
            break;
        case 32:
            LOG_INFO("errorCode=%d : unknown - property", errorCode);
            break;
        default:
            LOG_INFO("errorCode=%d : default", errorCode);
            break;
    }
}

