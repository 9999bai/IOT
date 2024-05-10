#include "Analyse.h"

Analyse::Analyse()
{
    // LOG_INFO("Analyse::Analyse() ctor ...");    
}

Analyse::~Analyse()
{

}

// opc
UA_StatusCode Analyse::OPCMultiRead(UA_Client *client, UA_ReadResponse& response, UA_ReadValueId* arrayItem, const int& arraySize, const nextFrame& sendFrame)
{
    // UA_ReadRequest request;
    // UA_ReadRequest_init(&request);
	// request.nodesToRead = &arrayItem[0];
	// request.nodesToReadSize = arraySize;

	// UA_ReadResponse response = UA_Client_Service_read(client, request);
	UA_StatusCode retval = response.responseHeader.serviceResult;
	if (retval == UA_STATUSCODE_GOOD)
	{
		if (response.resultsSize == arraySize)
		{
			for (int i = 0; i < arraySize; ++i)
			{
                if(response.results[i].status == UA_STATUSCODE_GOOD)
                {
                    if(response.results[i].hasValue)
                    {
                        // LOG_INFO("opc...%d", i);
                        std::string strtmp(std::to_string(i));

                        UA_Variant out;
                        memcpy(&out, &response.results[i].value, sizeof(UA_Variant));
                        UA_Variant_init(&response.results[i].value);
                        OPCAnalyse(i, sendFrame, out);
                        // AnalyseFunc(i, send, out);
                        // poolPtr_->run(std::bind(&Analyse::AnalyseFunc, opcuaAnalysePtr_, strtmp, sendFrame_, (void *)&out));
                    }else
                    {
                        LOG_ERROR("response.results[i].hasValue errorr...");
                    }
                }
                else
                {
                    LOG_ERROR("retStatusArray errorr=%d",(int)response.results[i].status);
                }
            }
        }
		else
		{
			UA_ReadResponse_clear(&response);
            LOG_ERROR("opc analyse response.resultsSize != arraySize...%d=%d",(int)response.resultsSize, (int)arraySize);
            return UA_STATUSCODE_BADUNEXPECTEDERROR;
        }
	}else{
        printf("UA_Client_Service_read response errorcode = %X\n", (int)retval);
    }
    UA_ReadResponse_clear(&response);
    return UA_STATUSCODE_GOOD;
}

void Analyse::OPCAnalyse(const int& index, const nextFrame& sendFrame, const UA_Variant& out)
{
    bool resOK = true; // 解析是否成功
    enum_RW resRW;

    iot_device device = sendFrame.second.first;
    std::vector<iot_template> templats = sendFrame.second.second;
    if(index >= templats.size())
    {
        LOG_INFO("eoor OpcUAAnalyse::AnalyseFunc %d >= templats.size()...", index);
        return;
    }
    iot_template templat = templats.at(index);
    LOG_INFO("\t\t\t\t%s", templat.register_addr.c_str());
    if (templat.rw == enum_read)
    {
        resRW = enum_read;
        bool boolresult = true;
        std::string value("");

        switch(templat.data_type)
        {
            case enum_data_type_OPC_bool:
            {
                value = ((*(UA_Boolean *)out.data)==true?("true"):("false"));
                LOG_INFO("UA_Boolean=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_sbyte:
            {
                value = std::to_string(*(UA_SByte *)out.data);
                LOG_INFO("UA_SByte=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_byte:
            {
                value = std::to_string(*(UA_Byte *)out.data);
                LOG_INFO("UA_Byte=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int16:
            {
                value = std::to_string(*(UA_Int16 *)out.data);
                LOG_INFO("UA_Int16=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint16:
            {
                value = std::to_string(*(UA_UInt16 *)out.data);
                LOG_INFO("UA_UInt16=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int32:
            {
                value = std::to_string(*(UA_Int32 *)out.data);
                LOG_INFO("UA_Int32=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint32:
            {
                value = std::to_string(*(UA_UInt32 *)out.data);
                LOG_INFO("UA_UInt32=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int64:
            {
                value = std::to_string(*(UA_Int64 *)out.data);
                LOG_INFO("UA_Int64=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint64:
            {
                value = std::to_string(*(UA_UInt64 *)out.data);
                LOG_INFO("UA_UInt64=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_float:
            {
                float tmp = (float)*(UA_Float *)out.data;
                // char tmp[300];
                // sprintf(tmp, "%.2f", (float)*(UA_Float *)out.data);
                std::string str = std::to_string(tmp);
                value = str;
                LOG_INFO("UA_Float=%s", value.c_str());
                break;
            }
            case enum_data_type_OPC_double:
            {
                double tmp = (double)*(UA_Double *)out.data;
                // char tmp[300];
                // sprintf(tmp, "%.2f", (double)*(UA_Double *)out.data);
                std::string str = std::to_string(tmp);
                value = str;
                // value = std::to_string(tmp);
                LOG_INFO("UA_Double=%s", value.c_str());
                break;
            }
            case enum_data_type_OPC_string:
            {
                UA_String *ptr = (UA_String *)out.data;
                value = std::string((char*)ptr->data, ptr->length);
                LOG_INFO("UA_String=%s", value.c_str());
                break;
            }
            // case enum_data_type_OPC_datetime:
            // case enum_data_type_OPC_guid:
            case enum_data_type_OPC_bytestring:
            {
                UA_ByteString * ptr = (UA_ByteString *)out.data;
                value = std::string((char*)ptr->data,ptr->length);
                LOG_INFO("UA_ByteString=%s",value.c_str());
                break;
            }
            // case enum_data_type_OPC_xmlelement:
            // case enum_data_type_OPC_nodeid:
            // case enum_data_type_OPC_expandednodeid:
            // case enum_data_type_OPC_statuscode:
            // case enum_data_type_OPC_qualifiedname:
            // case enum_data_type_OPC_localizedtext:
            // {

                // break;
            // }
            default:{
                LOG_ERROR("OpcUAAnalyse::AnalyseFunc default...%d", (int)templat.data_type);
                boolresult = false;
                resOK = false;
                break;
            }
                // case enum_data_type_OPC_extensionobject:
                // case enum_data_type_OPC_datavalue:
                // case enum_data_type_OPC_variant:
                // case enum_data_type_OPC_diagnosticinfo:
        }
        if(boolresult)
        {
            QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, value));
        }
    }else if(templat.rw == enum_write)
    {
        resRW = enum_write;
    }
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame);
    }
}

bool Analyse::HandleData(const frame& v_data, const nextFrame& nextframe)
{
    // LOG_INFO("**********data.size=%d",v_data.size());
    if (v_data.empty())
    {
        return false;
    }
    iot_device device = nextframe.second.first;
    std::vector<iot_template> v_templat = nextframe.second.second;
    
    if(v_templat.size() < 0)
    {
        LOG_FATAL("AnalyseFunc v_templat.size < 0");
    }
    iot_template templat = v_templat.at(0);

    if(0 == templat.sub_template_id)
    {
        try{
            frame dest;
            HandleByte_order(v_data, dest, templat.byte_order);
            std::string res = HandleData_type(dest, templat.data_type, templat.correct_mode);
            LOG_INFO("解析 %s = %s", templat.param_name.c_str(), res.c_str());
            QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, res));
        }
        catch(std::out_of_range)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData sub_template_id=0 --out_of_range...");
            return false;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData sub_template_id=0 error %s", e.what());
            return false;
        }
        catch(...)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData sub_template_id=0 error...");
            return false;
        }
        return true;
    }
    else
    {
        try
        {
            for (auto it = templat.v_sub_template.begin(); it != templat.v_sub_template.end(); it++)
            {
                std::string res;
                if (it->bit == 0)
                {
                    auto d_it = v_data.begin();
                    frame tmp(d_it + it->s_addr, d_it + it->s_addr + it->data_quantity);

                    frame dest;
                    HandleByte_order(tmp, dest, it->byte_order);
                    res = HandleData_type(dest, it->data_type, it->correct_mode);
                }
                else
                {
                    res = HandleData_typeBit(v_data.at(it->s_addr), it->bit);
                }
                LOG_INFO("%s = %s", it->param_name.c_str(), res.c_str());
                QueueData(it->send_type, setItem(device.gateway_id, device.device_id, device.device_addr, it->param_id, it->param_name, res));
            }
        }
        catch(const std::out_of_range)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData--out_of_range...");
            return false;
        }
        catch(std::exception& e)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData error %s", e.what());
            return false;
        }
        catch(...)
        {
            LOG_ERROR("ModbusRtuAnalyse::HandleData error ... %s");
            return false;
        }
        return true;
    }
}

void Analyse::HandleByte_order(const frame &v_data, frame& dest, const enum_byte_order& type)
{
    switch (type)
    {
        case enum_byte_order_normal:
            LOG_ERROR("HandleData enum_byte_order_normal...");
            // abort();
            break;
        case enum_byte_order_A:
            break;
        case enum_byte_order_AB:
            break;
        case enum_byte_order_BA:
            dest = char2_BA(v_data);
            break;
        case enum_byte_order_ABC:
            break;
        case enum_byte_order_CBA:
            dest = char3_CBA(v_data);
            break;
        case enum_byte_order_AB_CD:
            break;
        case enum_byte_order_CD_AB:
            dest = char4_CD_AB(v_data);
            break;
        case enum_byte_order_BA_DC:
            dest = char4_BA_DC(v_data);
            break;
        case enum_byte_order_DC_BA:
            dest = char4_DC_BA(v_data);
            break;
        case enum_byte_order_AB_CD_EF_GH:
            break;
        case enum_byte_order_GH_EF_CD_AB:
            dest = char8_GH_EF_CD_AB(v_data);
            break;
        case enum_byte_order_BA_DC_FE_HG:
            dest = char8_BA_DC_FE_HG(v_data);
            break;
        case enum_byte_order_HG_FE_DC_BA:
            dest = char8_HG_FE_DC_BA(v_data);
            break;
        default:
            LOG_ERROR("HandleByte_order -- %d", (int)type);
            break;
    }
}

std::string Analyse::HandleData_type(const frame &v_data, const enum_data_type& data_type, const std::string& correct_mode)
{
    std::string res;
    switch (data_type)
    {
        case enum_data_type_normal:
            LOG_FATAL("ModbusRtuAnalyse::HandleData_type----enum_data_type_normal...");
            break;
        case enum_data_type_bool:
            res = std::to_string(v_data.at(0)-'0');
            break;
        case enum_data_type_int8_str:
            res = std::to_string(v_data.at(0)-'0');
            break;
        case enum_data_type_int8_hex:
            res = std::to_string(v_data.at(0) & 0xFF);
            break;
        case enum_data_type_int16_str:
        {
            // LOG_INFO("HandleData  enum_data_type_int16_str...");
            //解析数据
            u_int16_t value = handleData_16bitStr(v_data);
            //数据校准
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_int16_hex:
        {
            // LOG_INFO("HandleData  enum_data_type_int16_hex...");
            u_int16_t value = handleData_16bitHex(v_data);
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_int32_str:
        {
            // LOG_INFO("HandleData  enum_data_type_int32_str...");
            u_int32_t value = handleData_32bitStr(v_data);
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_int32_hex:
        {
            // LOG_INFO("HandleData  enum_data_type_int32_hex...");
            u_int32_t value = handleData_32bitHex(v_data);  
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_float:
        {
            // LOG_INFO("HandleData  enum_data_type_float...");
            float value = handleData_float(v_data);
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_double:
        {
            // LOG_INFO("HandleData  enum_data_type_double...");
            double value = handleData_double(v_data);
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        case enum_data_type_BCD:
        {
            // LOG_INFO("HandleData  enum_data_type_BCD...");
            u_int32_t value = handleData_BCD(const_cast<frame&>(v_data));
            res = handleData_correct_mode(value, correct_mode);
            break;
        }
        default:
            LOG_ERROR("HandleData_type error %d", data_type);
            break;
    }
    return res;
}

std::string Analyse::HandleData_typeBit(const char& data, const int& bit)
{
	// printf("[INFO]0000/00/00 00:00:00 : v_data : %d", data);
    // uint8_t tmp = data & (uint8_t)pow((int)2, (int)(bit - 1));
    uint8_t tmp = 0;
    switch(bit)
    {
    case 1:
        tmp = data & 0x01;
        break;
    case 2:
        tmp = data & 0x02;
        break;
    case 3:
        tmp = data & 0x04;
        break;
    case 4:
        tmp = data & 0x08;
        break;
    case 5:
        tmp = data & 0x10;
        break;
    case 6:
        tmp = data & 0x20;
        break;
    case 7:
        tmp = data & 0x40;
        break;
    case 8:
        tmp = data & 0x80;
        break;
    }
    return std::to_string(tmp > 0 ?(1) : (0));
}

u_int16_t Analyse::handleData_16bitStr(const frame& v_data)
{
    // LOG_INFO("handleData_16bitStr");
    u_int16_t res = 0;
    char2or4Str_To_Uint16or32(v_data, res);
    // LOG_INFO("ModbusRtuAnalyse::handleData_16bitStr name=%s, res=%d\n", templat.param_name.c_str(), (int)res);
    return res;
}

u_int16_t Analyse::handleData_16bitHex(const frame& v_data)
{
    // LOG_INFO("handleData_16bitHex");
    u_int16_t res = 0;
    char2or4Hex_To_uint16or32(v_data, res);
    // LOG_INFO("ModbusRtuAnalyse::handleData_16bitHex name=%s, res=%d\n", templat.param_name.c_str(), (int)res);
    return res;
}

u_int32_t Analyse::handleData_32bitStr(const frame& v_data)
{
    // LOG_INFO("handleData_32bitStr");
    u_int32_t res = 0;
    char2or4Str_To_Uint16or32(v_data, res);
    // LOG_INFO("ModbusRtuAnalyse::handleData_32bitStr name=%s, res=%d\n", templat.param_name.c_str(), (int)res);
    return res;
}

u_int32_t Analyse::handleData_32bitHex(const frame& v_data)
{
    // LOG_INFO("handleData_32bitHex");
    u_int32_t res = 0;
    char2or4Hex_To_uint16or32(v_data, res);
    // LOG_INFO("ModbusRtuAnalyse::handleData_32bitHex name=%s, res=%d", templat.param_name.c_str(), (int)res);
    return res;
}

float Analyse::handleData_float(const frame& v_data)
{
    // LOG_INFO("handleData_float");
    float res = 0;
    u_int32_t u32_data;
    char2or4Hex_To_uint16or32(v_data, u32_data);
    IEEE754_To_float(u32_data, res);
    // LOG_INFO("ModbusRtuAnalyse::handleData_float name=%s, res=%f\n", templat.param_name.c_str(), res);
    return res;
}

double Analyse::handleData_double(const frame& v_data)
{
    // LOG_INFO("handleData_double");
    double res = 0;
    //coding
    return res;
}

u_int32_t Analyse::power(int base, int times)
{
    int i;
    u_int32_t rslt = 1;
    for (i = 0; i < times; i++)
        rslt *= base;
    return rslt;
}

u_int32_t Analyse::handleData_BCD(frame& bcd)
{
    int i, tmp;
    u_int32_t dec = 0;
    for (i = 0; i < bcd.size(); i++)
    {
        tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);
        dec += tmp * power(100, bcd.size() - 1 - i);
    }
    return dec;
}

frame Analyse::char2_BA(const frame& src)
{
	if(src.size() == 2)
	{
		frame tmp;
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		return tmp;
	}else{
		LOG_ERROR("error char2_BA size = %d", src.size());
	}
}

frame Analyse::char3_CBA(const frame& src)
{
    if(src.size() == 3)
	{
		frame tmp;
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		return tmp;
	}else{
		LOG_ERROR("error char3_CBA size = %d", src.size());
	}
}

frame Analyse::char4_CD_AB(const frame& src)
{
	if(src.size() == 4)
	{
		frame tmp;
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[0]);
		tmp.emplace_back(src[1]);
		return tmp;
	}else{
		LOG_ERROR("error char4_CD_AB size = %d", src.size());
	}

}

frame Analyse::char4_BA_DC(const frame& src)
{
	if(src.size() == 4)
	{
		frame tmp;
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[2]);
		return tmp;
	}else{
		LOG_ERROR("error char4_BA_DC size = %d", src.size());
	}
}

frame Analyse::char4_DC_BA(const frame& src)
{
	if(src.size() == 4)
	{
		frame tmp;
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		return tmp;
	}else{
		LOG_ERROR("error char4_DC_BA size = %d", src.size());
	}
}

frame Analyse::char8_GH_EF_CD_AB(const frame& src)
{
	if(src.size() == 8)
	{
		frame tmp;
		tmp.emplace_back(src[6]);
		tmp.emplace_back(src[7]);
		tmp.emplace_back(src[4]);
		tmp.emplace_back(src[5]);
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[0]);
		tmp.emplace_back(src[1]);
		return tmp;
	}else{
		LOG_ERROR("error char8_GH_EF_CD_AB size = %d", src.size());
	}
}

frame Analyse::char8_BA_DC_FE_HG(const frame& src)
{
	if(src.size() == 8)
	{
		frame tmp;
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[5]);
		tmp.emplace_back(src[4]);
		tmp.emplace_back(src[7]);
		tmp.emplace_back(src[6]);
		return tmp;
	}else{
		LOG_ERROR("error char8_BA_DC_FE_HG size = %d", src.size());
	}
}

frame Analyse::char8_HG_FE_DC_BA(const frame &src)
{
	if(src.size() == 8)
	{
		frame tmp;
		tmp.emplace_back(src[7]);
		tmp.emplace_back(src[6]);
		tmp.emplace_back(src[5]);
		tmp.emplace_back(src[4]);
		tmp.emplace_back(src[3]);
		tmp.emplace_back(src[2]);
		tmp.emplace_back(src[1]);
		tmp.emplace_back(src[0]);
		return tmp;
	}else{
		LOG_ERROR("error char8_HG_FE_DC_BA size = %d", src.size());
	}
}
