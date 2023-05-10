#pragma once

#include <vector>
#include "myHelper/myHelper.h"

class Analyse
{
public:
    Analyse();
    virtual ~Analyse();

    void setAnalyseFinishCallback(const AnalyseFinishCallback &cb) { analyseFinishCallback_ = cb; }

    // virtual void setFrameConf(const nextFrame& frameConfig) = 0;
    virtual void AnalyseFunc(const std::string& msg, const nextFrame& nextframe) = 0;
    // void ObserverRecvAnalyse(const std::string& topic,const std::string& msg);

protected:
    void HandleByte_order(const frame &v_data, const enum_byte_order &type);
    std::string HandleData_type(const frame &v_data, const enum_data_type& data_type, const std::string& correct_mode);


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
    nextFrame frameConfig_;
};