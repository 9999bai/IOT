#include "myHelper.h"
#include <stdio.h>
#include <sstream>

std::string localip = "192.168.2.103";
enun_endian ENDIAN = enum_little_endian; //默认小端
std::mutex TD_mutex_;
std::mutex RD_mutex_;
std::list<iot_data_item> list_iotdataTD_;			//定时数据
std::queue<iot_data_item> queue_iotdataRD_;      	//实时数据


void endianMode()
{
	union_uint16TOuchar data;
	data.u16_data = 0x1234;
	if(data.uchar_data[0] == 0x34)
	{
		ENDIAN = enum_little_endian;
		LOG_INFO("ENDIAN = enum_little_endian");
	}
	else{
		ENDIAN = enum_big_endian;
		LOG_INFO("ENDIAN = enum_big_endian");
	}
}

// template <typename T>
// void char2or4Hex_To_uint16or32(const frame& src, T& dest)
// {
// 	T tmp = 0;
// 	for (int i = src.size() - 1; i >= 0; i++)
// 	{
// 		tmp = 0;
// 		tmp = src.at(i) & 0x0f;
// 		tmp += (src.at(i) >> 4) * 16;
// 		dest += (dest << 8) + tmp;
// 	}
// }

// template <typename T>
// void char2or4Str_To_Uint16or32(const frame& src, T& dest)
// {
// 	for (int i = 0; i < src.size(); i++)
// 	{
// 		dest = dest*10 + (((u_char)src.at(i)) - '0');
// 	}
// }

u_int32_t LittleEndianStrToU32Bit(const frame& str)
{
    int len = str.size();
    u_int32_t value=0;
    u_int32_t tmp=0;
    for(int i=len-1; i>=0; i--)
    {
        tmp=0;
        tmp = str.at(i)&0x0f ;
        tmp +=((((u_int8_t)str.at(i))>>4))*16;
        value = (value<<8) + tmp;
    }
    return value;
}

u_int32_t BigEndianStrToU32Bit(const frame& str)
{
    int len = str.size();
    u_int32_t value=0;
    u_int32_t tmp=0;
    for(int i=0; i<len; i++)
    {
        tmp=0;
        tmp = str.at(i)&0x0f ;
        //        quint8 a=(str.at(i)>>4);
        tmp +=((((u_int8_t)str.at(i))>>4))*16;
        value = (value<<8) + tmp;
    }
    return value;
}

void IEEE754_To_float(const u_int32_t& src, float& dest)
{
	union_uchar4TOfloat value;
	for (int i = 0; i < 4; i++)
	{
		value.uchar_data[i] = (u_int8_t)(src >> (i * 8));
	}
	dest = value.float_data;
}

void float_To_IEEE754(const float& src, u_int32_t& dest)
{
	union_uchar4TOfloat value;
	value.float_data = src;
	// switch(ENDIAN)
	// {
	// 	case enum_little_endian:
	// 	for (int i = 3; i >= 0; i--)
	// 	{
	// 		dest |= (u_int32_t)value.uchar_data[i] << (i * 8);
	// 	}
	// 	break;
	// 	case enum_big_endian:
	// 	for (int i = 0; i < 4; i++)
	// 	{
	// 		dest |= (u_int32_t)value.uchar_data[i] << (i * 8);
	// 	}
	// 	break;
	// }
	for (int i = 0; i < 4; i++)
	{
		dest |= (u_int32_t)value.uchar_data[i] << (i * 8);
	}
}

void IEEE754_To_double(){}
void double_To_IEEE754(){}

void uint16_To_char2(const u_int16_t& src, frame& dest)
{
	union_uint16TOuchar data;
	data.u16_data = src;
	switch(ENDIAN)
	{
		case enum_little_endian:
			dest.emplace_back(data.uchar_data[1]);//高地址
			dest.emplace_back(data.uchar_data[0]);
			break;
		case enum_big_endian:
			dest.emplace_back(data.uchar_data[0]);//低地址
			dest.emplace_back(data.uchar_data[1]);
			break;
	}
}

void uint32_To_char4(const u_int32_t&src, frame& dest)
{
	union_uchar4TOuint32 data;
	data.u32_data = src;
	switch(ENDIAN)
	{
		case enum_little_endian:
			dest.emplace_back(data.uchar_data[3]); //高地址
			dest.emplace_back(data.uchar_data[2]);
			dest.emplace_back(data.uchar_data[1]);
			dest.emplace_back(data.uchar_data[0]);
			break;
		case enum_big_endian:
			dest.emplace_back(data.uchar_data[0]);//低地址
			dest.emplace_back(data.uchar_data[1]);
			dest.emplace_back(data.uchar_data[2]);
			dest.emplace_back(data.uchar_data[3]);
		break;
	}
}

char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

frame HexStrToByteArray(const std::string& str)
{
    frame senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();         //len = 2;
    senddata.resize(len/2);        //
    char lstr,hstr;
    for(int i=0; i<len; )
    {
		hstr = str.at(i);
		if (hstr == ' ')
		{
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str.at(i);
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
    return senddata;
}


// template <typename T>
// void uchar2_To_u16(const frame& v_data, T& dest)
// {
// 	for (int i = 1; i >= 0; i++)
// 	{
// 		dest <<= 8;
// 		dest += (u_char)v_data.at(i);
// 	}
// }

uint16_t usMBCRC16(const frame& frame, uint16_t len)
{
	u_char ucCRCHi = 0xFF;
	u_char ucCRCLo = 0xFF;
	int index;
	for (int i = 0; i < len; i++)
	{
		index = ucCRCLo ^ (u_char)(frame.at(i));
		ucCRCLo = (u_char)(ucCRCHi ^ aucCRCHi[index]);
		ucCRCHi = aucCRCLo[index];
	}
	return (uint16_t)(ucCRCHi << 8 | ucCRCLo);
}

bool CheckCRC(const frame& datablock, const frame& crcblock)
{
	u_int16_t crc_tmp = usMBCRC16(datablock, (uint16_t)datablock.size());
	u_int16_t crc_get = 0;
	frame tmp;
	tmp.emplace_back(crcblock.at(1));
	tmp.emplace_back(crcblock.at(0));
	char2or4Hex_To_uint16or32(tmp, crc_get);
	// char2or4Hex_To_uint16or32(char2_BA(crcblock), crc_get);
	return crc_get == crc_tmp;
}

bool stringSplit(const std::string& src, std::vector<std::string>& dst, char delim)
{
	if(0 == src.size())
	{
		return false;
	}
	std::stringstream srcS(src);
	std::string item;
	while (std::getline(srcS, item, delim))
	{
		dst.emplace_back(item);
	}
	return true;
}

std::string floatToString(const float& f)
{
	char buf[100] = {0};
	sprintf(buf, "%.2f", f);
	std::string tmp(buf);
	return tmp;
}

iot_data_item setItem(const int& gatewid, const int& device_id,const std::string& deviceaddr, const int& param_id, const std::string& param_name, const std::string& value)
{
    iot_data_item item;
    item.gateway_id = gatewid;
    item.device_id = device_id;
    // item.device_addr = deviceaddr;
    item.param_id = param_id;
    item.param_name = param_name;
    item.value = value;
    return item;
}

void printFrame(const std::string& str, const frame& data)
{
	printf("[INFO]----/--/-- --:--:-- : %d %s : ", data.size(), str.c_str());
	for (auto it = data.begin(); it != data.end(); it++)
	{
		printf("%02x ", *it);
	}
	printf("\n");
}

std::string itemToJson(const std::string& title, const iot_data_item& item)
{
	Json::Value root;
	Json::FastWriter writer;
	Json::Value content;
	Json::Value node;

	node["gateway_id"] = item.gateway_id;
	node["device_id"] = item.device_id;
	// node["device_addr"] = item.device_addr;
	node["param_id"] = item.param_id;
	node["param_name"] = item.param_name;
	node["value"] = item.value;
	content[0] = node;

	root[title] = content;
	std::string jsonStr(writer.write(root).c_str());
	// LOG_INFO("json : %s --- json.size = %d ", jsonStr.c_str() ,jsonStr.size());
	return jsonStr;
}

std::string itemToJson(const std::string& title, const std::vector<iot_data_item>& v_item)
{
	Json::Value root;
	Json::FastWriter writer;
	Json::Value content;
	Json::Value node;
	for (int i = 0; i < v_item.size(); i++)
	{
		iot_data_item item = v_item.at(i);
		node["gateway_id"] = item.gateway_id;
		node["device_id"] = item.device_id;
		// node["device_addr"] = item.device_addr;
		node["param_id"] = item.param_id;
		node["param_name"] = item.param_name;
		node["value"] = item.value;
		content[i] = node;
	}
	root[title] = content;
	std::string jsonStr = writer.write(root);
	return jsonStr;
}

void addIotDataTD(const iot_data_item& item)
{
	for(iot_data_item& tmp : list_iotdataTD_)
	{
		if(tmp.gateway_id == item.gateway_id && tmp.device_id == item.device_id && tmp.param_id == item.param_id)
		{
			if(tmp.value != item.value)
			{
				// LOG_INFO("--------tmp = %s", tmp.value.c_str());
				std::unique_lock<std::mutex> lock(TD_mutex_);
				tmp.value = item.value;
				// LOG_INFO("--------tmp = %s", tmp.value.c_str());
			}
			return;
		}
	}
	{
		std::unique_lock<std::mutex> lock(TD_mutex_);
		list_iotdataTD_.emplace_back(item);
	}
	LOG_INFO("list_iotdataTD_++++++++");
}

void QueueData(int send_type, const iot_data_item& item)
{
	// LOG_INFO("************************************%d", send_type);
	if (send_type == 1) // 立即发送
	{
        std::unique_lock<std::mutex> lock(RD_mutex_);
        queue_iotdataRD_.push(item);
		LOG_INFO("实时表 +++++++++++++++");
	}
	else
    {
        addIotDataTD(item);
    }
}

// frame DLT645StrToHex(const std::string& meterid)
// {
//     if(meterid.size() > 2)
//     {
//         frame res;
//         auto it = meterid.begin();
//         for (int i = 0; i <= meterid.size()-2; i+=2)
//         {
//             int tmp = std::atoi(std::string(it + i, it + i + 2).c_str());
//             res.emplace_back(tmp & 0xF0 * 16 + tmp & 0x0F);
//         }
//         return res;
//     }
//     else
//     {
//         LOG_ERROR("DLT645MeteridStrToHex meterid.length() != 12");
//     }
// }

void ReverseOrder(const frame& src, frame& dest)
{
    if(src.size() <= 0)
    {
		LOG_ERROR("ReverseOrder src.size()<= 0");
		return;
	}

	for (int i = src.size() - 1; i >= 0; i--)
	{
		dest.emplace_back(src.at(i));
	}
}

char DLTCheck(const frame& src)
{
    int res = 0;
    for (auto it = src.begin(); it != src.end(); it++)
    {
        res += (u_char)*it;
    }
    return res &= 0xFF;
}

void ObjectIdentifier(const iot_template& templat, frame& data)
{
    data.emplace_back(0x0C);
    BacnetIP_ObjectIdentifity object = strToObject(templat.register_addr);

    union_uchar4TOuint32 uc32;
    uc32.u32_data = (object.objectType << 22) | (object.InstanceNumber & 0x3FFFFF);

    LOG_INFO("objectType=%d, instance=%d", object.objectType, object.InstanceNumber);
    // LOG_INFO("res= %d", uc32.u32_data);

    data.emplace_back(uc32.uchar_data[3]);
    data.emplace_back(uc32.uchar_data[2]);
    data.emplace_back(uc32.uchar_data[1]);
    data.emplace_back(uc32.uchar_data[0]);
}

BacnetIP_ObjectIdentifity strToObject(const std::string& strObject)
{
    BacnetIP_ObjectIdentifity object;
    std::vector<std::string> res;
    if(stringSplit(strObject, res, '-'))
    {
        try
        {
            object.objectType = std::atoi(res.at(0).c_str());
            object.InstanceNumber = std::atoi(res.at(1).c_str());
        }
        catch(std::out_of_range)
        {
            LOG_FATAL("BacnetipFrame::strToObject out_of_range...%s", strObject.c_str());
        }
        catch(...)
        {
            LOG_FATAL("BacnetipFrame::strToObject other error...%s", strObject.c_str());
        }
    }else{
        LOG_FATAL("BacnetipFrame::strToObject error..%s", strObject.c_str());
    }
    return object;
}


