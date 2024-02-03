#pragma once

#include <vector>
#include "myHelper/myHelper.h"

class Analyse
{
public:
    Analyse();
    virtual ~Analyse();

    void setAnalyseFinishCallback(AnalyseFinishCallback cb) { analyseFinishCallback_ = cb; }

    // virtual void setFrameConf(const nextFrame& frameConfig) = 0;
    virtual void AnalyseFunc(const std::string& msg, const nextFrame& nextframe) = 0;
    // void ObserverRecvAnalyse(const std::string& topic,const std::string& msg);
    virtual bool HandleData(const frame& v_data, const nextFrame& nextframe);//modbus解析，其他协议重写此函数


    // IEC104
    u_int16_t getTX_SN() { return TX_SN_; }
    u_int16_t getRX_SN() { return RX_SN_; }
    void IncreaseTX()
    {
        std::unique_lock<std::mutex> lock(TX_Mutex_);
        if(++TX_SN_ > 0x7FFF)
        {
            TX_SN_ = 0;
        }
    }

    void IncreaseRX() 
    {
        std::unique_lock<std::mutex> lock(RX_Mutex_);
        if(++RX_SN_ > 0x7FFF)
        {
            RX_SN_ = 0;
        }
    }

protected:

    // IEC104

    std::mutex TX_Mutex_;
    u_int16_t TX_SN_; // 发送序号

    std::mutex RX_Mutex_;
    u_int16_t RX_SN_; // 接收序号
    
    // Modbus
    void HandleByte_order(const frame &v_data, const enum_byte_order &type);
    std::string HandleData_type(const frame &v_data, const enum_data_type& data_type, const std::string& correct_mode);
    std::string HandleData_typeBit(const char& data, const int& bit);

    u_int16_t handleData_16bitStr(const frame& v_data);
    u_int16_t handleData_16bitHex(const frame& v_data);
    u_int32_t handleData_32bitStr(const frame& v_data);
    u_int32_t handleData_32bitHex(const frame& v_data);
    float handleData_float(const frame& v_data);
    double handleData_double(const frame& v_data);

    // BCD转10进制
    u_int32_t power(int base, int times);
    u_int32_t handleData_BCD(frame& bcd);

    frame char2_BA(const frame& src);
    frame char4_CD_AB(const frame &src);
    frame char4_BA_DC(const frame& src);
    frame char4_DC_BA(const frame& src);
    frame char8_GH_EF_CD_AB(const frame& src);
    frame char8_BA_DC_FE_HG(const frame& src);
    frame char8_HG_FE_DC_BA(const frame &src);

    AnalyseFinishCallback analyseFinishCallback_;
    // nextFrame frameConfig_;

    frame v_data;
    // std::vector<char> v_data;
};