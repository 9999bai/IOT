#include "Analyse.h"

Analyse::Analyse()
{
    // LOG_INFO("Analyse::Analyse() ctor ...");    
}

Analyse::~Analyse()
{

}

void Analyse::HandleByte_order(const frame &v_data, const enum_byte_order& type)
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
            const_cast<frame&>(v_data) = char2_BA(v_data);
            break;
        case enum_byte_order_AB_CD:
            break;
        case enum_byte_order_CD_AB:
            const_cast<frame&>(v_data) = char4_CD_AB(v_data);
            break;
        case enum_byte_order_BA_DC:
            const_cast<frame&>(v_data) = char4_BA_DC(v_data);
            break;
        case enum_byte_order_DC_BA:
            const_cast<frame&>(v_data) = char4_DC_BA(v_data);
            break;
        case enum_byte_order_AB_CD_EF_GH:
            break;
        case enum_byte_order_GH_EF_CD_AB:
            const_cast<frame&>(v_data) = char8_GH_EF_CD_AB(v_data);
            break;
        case enum_byte_order_BA_DC_FE_HG:
            const_cast<frame&>(v_data) = char8_BA_DC_FE_HG(v_data);
            break;
        case enum_byte_order_HG_FE_DC_BA:
            const_cast<frame&>(v_data) = char8_HG_FE_DC_BA(v_data);
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
            break;
    }
    return res;
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
