#pragma once

#include "/usr/include/mymuduo/Helper.h"
#include "/usr/include/mymuduo/Logger.h"
#include "/usr/include/json-arm/json.h"

#include <iostream>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <functional>
#include <utility>
#include <unordered_map>
#include <queue>
#include <list>

class Frame;
class Database;
class Analyse;
class Factory;
class SerialPort;
class TcpClient;
class UdpClient;
class NetSerial;
class Mediator;
class Observer;

using SerialPortPtr = std::shared_ptr<SerialPort>;
using TcpClientPtr = std::shared_ptr<TcpClient>;
using UdpClientPtr = std::shared_ptr<UdpClient>;

using FramePtr = std::shared_ptr<Frame>;
using DatabasePtr = std::shared_ptr<Database>;
using AnalysePtr = std::shared_ptr<Analyse>;
using FactoryPtr = std::shared_ptr<Factory>;
using NetSerialPtr = std::shared_ptr<NetSerial>;
using MediatorPtr = std::shared_ptr<Mediator>;
using ObserverPtr = std::shared_ptr<Observer>;


using frame = std::vector<char>;
using vector_frame = std::vector<frame>;
using ObserverRecvCallabck = std::function<void(const std::string&, const std::string&)>;
using ObserverSendCallback = std::function<void(const std::string&)>;
// using ControlFrameCallback = std::function<void()>;

typedef enum{
    enum_big_endian,
    enum_little_endian
} enun_endian;

typedef enum{
    enum_observer_type_tcp = 1,
    enum_observer_type_mqtt,
    enum_observer_type_rabbitmq
}enum_observer_type;

typedef enum{
    enum_netserial_serialport=1,
    enum_netserial_udp,
    enum_netserial_tcp
}enum_netserial_type;

typedef union{
    uint16_t u16_data;
    u_char uchar_data[2]={0};
} union_uint16TOuchar;

typedef union{
    float float_data;
    u_char uchar_data[4] = {0};
} union_uchar4TOfloat;

typedef union{
    u_int32_t u32_data;
    u_char uchar_data[4] = {0};
} union_uchar4TOuint32;

typedef struct{
    // 控制和上传
    int                                                                                                                                                                                                                                                                                                                   gateway_id;
    int device_id;
    int param_id;
    std::string value;
    
    // 上传
    std::string param_name;
    // std::string device_addr;
    // int templat_id;
} iot_data_item;


void endianMode();
bool stringSplit(const std::string& s, std::vector<std::string>& elems, char delim = ' ');
std::string floatToString(const float& f);
void printFrame(const std::string& str, const frame& data);
std::string itemToJson(const std::string& title, const iot_data_item& item);
std::string itemToJson(const std::string& title, const std::vector<iot_data_item>& v_item);
void addIotDataTD(const iot_data_item& item);

/* "1234" --> 4660 十六进制字符串转uint32
    "12"  --> 18   十六进制字符串转uint16 */
template <typename T>
void char2or4Hex_To_uint16or32(const frame& src, T& dest)
{
    dest = 0;
	for (int i = 0; i < src.size(); i++)
	{
        dest <<= 8;
        dest += (u_char)src.at(i);
	}
}

/*  "1234"--> 1234 字符串转uint32
    "12"  --> 12   字符串转uint16 */
template <typename T>
void char2or4Str_To_Uint16or32(const frame& src, T& dest)
{
    for (int i = 0; i < src.size(); i++)
	{
		dest = dest*10 + (((u_char)src.at(i)) - '0');
	}
}

template<typename T>
std::string handleData_correct_mode(T& src, const std::string& correct_mode)
{
    float value = 0;
    if (correct_mode == "1")
    {
        value = src * 1.0;
    }
    else if(correct_mode.at(0) == '/')
    {
        int param = std::atoi(correct_mode.substr(1).c_str());
        value = src * 1.0 / param;
    }
    else if(correct_mode.at(0) == '*')
    {
        int param = std::atoi(correct_mode.substr(1).c_str());
        value = src * 1.0 * param;
    }
    else if(correct_mode.at(0) == '+')
    {
        int param = std::atoi(correct_mode.substr(1).c_str());
        value = src * 1.0 + param;
    }
    else if(correct_mode.at(0) == '-')
    {
        int param = std::atoi(correct_mode.substr(1).c_str());
        value = src * 1.0 - param;
    }
    std::string tmp = floatToString(value);
    return tmp;
}

iot_data_item setItem(const int &gatewid, const int &device_id, const std::string &deviceaddr, const int &param_id, const std::string &param_name, const std::string &value);
void QueueData(int send_type, const iot_data_item &item);

u_int32_t LittleEndianStrToU32Bit(const frame &str);
u_int32_t BigEndianStrToU32Bit(const frame &str);

void IEEE754_To_float(const u_int32_t &src, float &dest);
void float_To_IEEE754(const float& src, u_int32_t& dest);

void IEEE754_To_double();
void double_To_IEEE754();

void uint16_To_char2(const u_int16_t&src, frame& dest);
void uint32_To_char4(const u_int32_t&src, frame& dest);

//和HexStrToByteArray结合使用
char ConvertHexChar(char ch);
//16进制字符串转字节数组
frame HexStrToByteArray(const std::string& str);

uint16_t usMBCRC16(const frame& frame, uint16_t len);
bool CheckCRC(const frame& datablock, const frame& crcblock);

//dlt645
// 11 22 33 44 ---> 44 33 22 11
void ReverseOrder(const frame &src, frame &dest);
// 累加和
char DLTCheck(const frame& src);


//------------------------------MYSQL CONNECTION---
// mysql 连接信息
typedef struct
{
    int mysqlServer_port = 3306;
    std::string mysqlServer_ip;
    std::string db_name;
    std::string user_name;
    std::string user_passwd;
} sqlServer_info;

typedef struct
{
    mymuduo::enum_data_res valid = mymuduo::enum_data_res_disvalid;
    std::string ip;
    int port;
} struct_net;

typedef struct{
    std::string hostname;
    int port;
    std::string username;
    std::string passwd;
    std::string subscribeTopic;
    std::string publishTopic;
} mqtt_info;

typedef enum{
    enum_data_type_normal = 0,
    enum_data_type_bool,
    enum_data_type_int8_str,
    enum_data_type_int8_hex,
    enum_data_type_int16_str,
    enum_data_type_int16_hex,
    enum_data_type_int32_str,
    enum_data_type_int32_hex,
    enum_data_type_float,
    enum_data_type_double,
    enum_data_type_string,
    enum_data_type_BCD,
    enum_data_type_BA_bool,         // 应用标记编码 1
    enum_data_type_BA_uint,         // 无符号整型
    enum_data_type_BA_int,          // 有符号整型
    enum_data_type_BA_float,        // IEEE754-float
    enum_data_type_BA_double,       // IEEE754-double
    enum_data_type_BA_octet_string, // 字符串
    enum_data_type_BA_character_string,// 字符串
    enum_data_type_BA_bit_string,       // 比特位串
    enum_data_type_BA_enum,             // 枚举
    enum_data_type_BA_date,             // 日期
    enum_data_type_BA_time,             // 时间
    enum_data_type_BA_ObjectIdentifity  // 对象标识符
} enum_data_type;

typedef enum{
    enum_byte_order_normal = 0,
    enum_byte_order_A = 1,
    enum_byte_order_AB,
    enum_byte_order_BA,
    enum_byte_order_AB_CD,
    enum_byte_order_CD_AB,
    enum_byte_order_BA_DC,
    enum_byte_order_DC_BA,
    enum_byte_order_AB_CD_EF_GH,
    enum_byte_order_GH_EF_CD_AB,
    enum_byte_order_BA_DC_FE_HG,
    enum_byte_order_HG_FE_DC_BA,
} enum_byte_order;

typedef enum
{
    enum_priority_read = 1,
    enum_priority_write,
    enum_priority_readwrite
} enum_priority;

typedef enum{
    enum_w_func_normal = 0,
    enum_w_func_0x05 = 0x05,
    enum_w_func_0x06 = 0x06,
    enum_w_func_0x0F = 0x0F,
    enum_w_func_0x10 = 0x10
} enum_w_func_code;

typedef enum{
    enum_r_func_normal = 0,
    enum_r_func_0x01,
    enum_r_func_0x02,
    enum_r_func_0x03,
    enum_r_func_0x04
}enum_r_func_code;

typedef enum{
    enum_error_func_0x81,
    enum_error_func_0x82,
    enum_error_func_0x83,
    enum_error_func_0x84,
    enum_error_func_0x85,
    enum_error_func_0x86,
    enum_error_func_0x8F,
    enum_error_func_0x90
}enum_error_func_code; // modbus错误码

typedef enum{
    enum_pro_name_ModbusRTU = 1,
    enum_pro_name_ModbusTCP,
    enum_pro_name_DLT645,
    enum_pro_name_CJT188,
    enum_pro_name_IEC104,
    enum_pro_name_BAcnetIP,
    enum_pro_name_OPCUA
} enum_pro_name;

typedef enum{
    enum_pro_normal = 0,
    enum_pro_mode_serial,
    enum_pro_mode_net
} enum_pro_mode;

typedef enum{
    enum_false = 1,
    enum_true
}enum_status;

typedef enum{
    enum_read = 1,
    enum_write
}enum_RW;

typedef struct{
    int sub_template_id;
    std::string param_name;
    int param_id;
    int s_addr;
    int data_quantity;
    int bit;
    std::string register_addr;
    enum_w_func_code w_func;
    enum_data_type data_type;
    enum_byte_order byte_order;
    std::string correct_mode;
    std::string data_unit;
    int send_type;
    enum_priority priority;
} iot_sub_template;

typedef struct{
    int template_id;
    std::string register_addr;
    int register_quantity;
    enum_r_func_code r_func;
    enum_w_func_code w_func;
    std::string param_name;
    enum_data_type data_type;
    enum_byte_order byte_order;
    std::string correct_mode;
    int param_id;
    std::string data_unit;
    int send_type;
    int sub_template_id;
    enum_priority priority;
    enum_RW rw;
    std::string other;
    std::vector<iot_sub_template> v_sub_template;
} iot_template;

typedef struct{
    int gateway_id;
    int device_id;
    std::string device_addr;
    std::string device_name;
    int template_id;
    int project_id;
    enum_status status;
    std::vector<iot_template> v_template;
} iot_device;

typedef struct{
    int gateway_id;
    std::string gateway_name;
    std::string description;
    enum_pro_name pro_name;
    enum_pro_mode pro_mode;
    mymuduo::struct_serial serial;
    struct_net net;
    int project_id;
    enum_status status;
    std::vector<iot_device> v_device;
} iot_gateway;


// --------IEC104-------

typedef enum{
    ENUM_Normal_Frame = 0,
    ENUM_U_Frame,
    ENUM_I_Frame,
    ENUM_S_Frame
} IEC104FrameType;

typedef enum{
    // 遥信
        ENUM_I_TypeIdentity_01 = 0x01, // 不带时标的单点遥信， 每个遥信占一个字节
        ENUM_I_TypeIdentity_03 = 0x03, // 不带时标的双点遥信， 每个遥信占一个字节
        ENUM_I_TypeIdentity_14 = 0x14, // 具有状态变位检出的成组单点遥信，每个字节8个遥信
    //遥测
        ENUM_I_TypeIdentity_09 = 0x09, // 带品质描述的测量值， 每个遥测值占3个字节  !!!!! 测量值，归一化值（遥测）
        ENUM_I_TypeIdentity_0A = 0x0A, // 带3个字节时标的且具有品质描述的测量值，每个遥测值占6个字节  !!!!!测量值，标度化值（遥测）
        ENUM_I_TypeIdentity_0B = 0x0B, // 不带时标的标准化值，每个遥测值占3个字节   !!!!!! 测量值，短浮点数（遥测）
        ENUM_I_TypeIdentity_0C = 0x0C, // 带3个时标的标准化值，每个遥测值占6个字节
        ENUM_I_TypeIdentity_0D = 0x0D, // 带品质描述的浮点值，每个遥测值占5个字节
        ENUM_I_TypeIdentity_0E = 0x0E, // 带三个字节时标且具有品质描述的浮点值，每个遥测值占8个字节
        ENUM_I_TypeIdentity_15 = 0x15, // 不带品质描述的遥测值，每个遥测值占一个字节
    // 遥脉
        ENUM_I_TypeIdentity_0F = 0x0F, // 不带时标的电能量，每个电能量占5个字节
        ENUM_I_TypeIdentity_10 = 0x10, // 带三个字节短时标的电能量，每个电能量占8个字节
        ENUM_I_TypeIdentity_25 = 0x25, // 带七个字节短时标的电能量，每个电能量占12个字节
    // SOE
        ENUM_I_TypeIdentity_02 = 0x02, // 带3个字节短时标的单点遥信
        ENUM_I_TypeIdentity_04 = 0x04, // 带3个字节短时标的双点遥信
        ENUM_I_TypeIdentity_1E = 0x1E, // 带7个字节短时标的单点遥信
        ENUM_I_TypeIdentity_1F = 0x1F, // 带7个字节短时标的双点遥信
    // 其他
        ENUM_I_TypeIdentity_2E = 0x2E, // 双点遥控
        ENUM_I_TypeIdentity_2F = 0x2F, // 双点遥调
        ENUM_I_TypeIdentity_64 = 0x64, // 召唤全数据
        ENUM_I_TypeIdentity_65 = 0x65, // 召唤全电度
        ENUM_I_TypeIdentity_67 = 0x67  // 时钟同步
} IEC104TypeIdentity;

typedef enum{
    ENUM_I_COT_0x01 = 0x01, // 周期、循环
    ENUM_I_COT_0x02 = 0x02, // 双点遥控
    ENUM_I_COT_0x03 = 0x03, // 突变
    ENUM_I_COT_0x04 = 0x04, // 初始化
    ENUM_I_COT_0x05 = 0x05, // 请求或被请求
    ENUM_I_COT_0x06 = 0x06, // 激活
    ENUM_I_COT_0x07 = 0x07, // 激活确认
    ENUM_I_COT_0x08 = 0x08, // 停止激活
    ENUM_I_COT_0x09 = 0x09, // 停止激活确认
    ENUM_I_COT_0x0A = 0x0A, // 激活结束
    ENUM_I_COT_0x14 = 0x14, // 响应总召唤
    ENUM_I_COT_0x25 = 0x25  // 遥脉
} IEC104COT;

// 解析完成下一步操作
typedef enum{
    ENUM_Normal = 1,        // 不需要发送
    ENUM_RebootSocket,      // 重启socket
    ENUM_Send_S_Frame,      // 需发送S确认帧
    // ENUM_Send_U_testFrame,      // 需发送U链路测试帧
    ENUM_Send_U_testRespFrame,  // 需发送U帧链路测试确认帧
    ENUM_SendFirst_I_Frame, // 第一次/定时 发送总召唤
    ENUM_SendNext_I_Frame   // YM 遥脉召唤帧（如果存在YM）
} AnalyseResult;

typedef enum{
    ENUM_Nothing =0,
    ENUM_YC,
    ENUM_YM
} IEC104Type;

// BacnetIP-----

typedef struct{
    int objectType;     // 对象类型 -- 前10位
    int InstanceNumber; // 实例号   -- 后22位
}BacnetIP_ObjectIdentifity;

typedef enum{
    enum_ApplicationTag = 0, // 应用标记
    enum_ContextTag          // 上下文标记
} BacnetIP_ClassType;

typedef enum{
    enum_PrimitiveData = 1, // 原语数据
    enum_ConstructedData_opening,    // 构件数据-open
    enum_ConstructedData_closing     // 构件数据-close
} BacnetIP_TVL;

typedef struct{
    BacnetIP_ClassType classType;   // 类别
    u_int8_t tagNumber;  // 标记编号
    BacnetIP_TVL tvlType;
    int tvlValue; // tvlType为原语数据时，tvl域值
} BacnetIP_Tag;

void ObjectIdentifier(const iot_template &templat, frame &data);
BacnetIP_ObjectIdentifity strToObject(const std::string &strObject);



// using AnalyseFinishCallback = std::function<void(const std::vector<iot_data_item>&)>;
// using AnalyseFinishCallback = std::function<void(bool, enum_RW, AnalyseResult, std::pair<int, IEC104FrameType>)>;
using AnalyseFinishCallback = std::function<void(bool, enum_RW, AnalyseResult, int, IEC104FrameType)>;
using NextFrameCallback = std::function<void()>;
using NewConnectionCallback = std::function<void()>;

using pair_frame = std::pair<iot_device, std::vector<iot_template>>;
using map_frame = std::unordered_map<int, pair_frame>;

using nextFrame = std::pair<frame, pair_frame>; //下一帧数据



/* 时间间隔以50毫秒为单位 */
#define Project_ID 2                        // 项目id
#define Origin_Timer 0.05                   // 定时间隔 一个单位 50ms

// #define TCP_GETMAG_TIMER_INTERVAL 1         // tcp缓存区buff 获取数据间隔 1个时间间隔 50ms
// #define UDP_GETMAG_TIMER_INTERVAL 2         // udp缓冲区buff 获取数据间隔 2个时间间隔 100ms
// #define Serial_GETMAG_TIMER_INTERVAL 0.2    // serial缓冲区buff 获取数据间隔


//threadpool
#define THREADPOOL_MAX_QUEUE_SIZE 200       // threadpool 装载函数最大数量
#define THREADPOOL_MAX_THREAD_SIZE 25       // threadpool 最大线程数量
#define THREADPOOL_MIN_THREAD_SIZE 10       // threadpool 最小线程数量

// modbusrtu
#define ModbusRtu_Freq 16                   // modbusRtu 定时发送频率 ----表示16个50ms=0.8s
#define ModbusRtuAnalyseFrame_MinSize 5     // modbusRtu 数据帧最小长度

// modbustcp
#define ModbusTcp_Freq 50                   // modbustcp 定时发送频率 ----表示200个50ms=10s
#define ModbusTcpIdentity 0x00              // modbustcp标识符
#define ModbusTcpReadLength 0x06            // modbustcp 读数据帧长度
#define ModbusTcpAnalyseFrame_Minsize 0x09  // modbustcp返回帧最小长度

//dlt645
#define DLT645_Freq 16                      // dlt645 定时发送频率 ----表示16个50ms=0.8s
#define DLT645ControlCode 0x11              // dlt645-2007 控制码
#define DLT645AnalyseFrame_Minsize 0x0C     // dlt645 数据帧最小长度

// CJT188
#define CJT188_Freq 16                      // CJT188 定时发送频率 ----表示16个50ms=0.8s
#define CJT188AnalyseFrame_Minsize  0x16    // CJT188 数据帧最小长度


// IEC104
#define IEC104_T0  30  // tcp连接的超时
#define IEC104_T1  15  // 发送方发送一个I格式报文或U格式报文后，必须在t1的时间内得到接收方的认可，否则发送方认为TCP连接出现问题并应重新建立连接。
#define IEC104_T2  10  // 接收方接收到I帧后，t2时间内未收到I帧，发送S确认帧    (t2<t1)
#define IEC104_T3  20  // 未收到 I/U/S帧，发送测试链路帧  (U帧)
#define IEC104_Freq 20*60*15 // IEC104 定时总召唤 15分钟
#define IEC104AnalyseFrame_Minsize 0x06    // IEC104 最小帧长度
 
//Bacnetip
#define BACNETIP_RX_MAXLENGTH 1024
#define BACNETIPAnalyeFrame_Minsize 0x09    // Bacnetip最小长度
#define BACNETIP_Freq 20    // 20*50ms=1s

#define SendPeriodTimer 40                  //定时发送数据  周期 单位：秒
#define TD_Data_Title "TD"
#define RD_Data_Title "RD"
#define TD_DataSize  10     // 定时发送数据个数  个数/次

extern std::string localip;

extern enun_endian ENDIAN;


extern std::mutex TD_mutex_;
extern std::mutex RD_mutex_;
extern std::list<iot_data_item> list_iotdataTD_;   // 定时数据
extern std::queue<iot_data_item> queue_iotdataRD_; // 实时数据








































static const u_char aucCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};
/***********************************************************************************
 * Summary: CRC校验码检索表（低位）
 **********************************************************************************/
static const u_char aucCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};