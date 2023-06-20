#pragma once

#include "Analyse/Analyse.h"
#include "myHelper/myHelper.h"
#include <strings.h>
#include <vector>
#include <math.h>

class ModbusRtuAnalyse : public Analyse
{
public:
    ModbusRtuAnalyse();
    ~ModbusRtuAnalyse();

    // friend class ModbusTcpAnalyse;

    void AnalyseFunc(const std::string &msg, const nextFrame &nextframe);

private:
    bool compairAddr(frame& v_data, int index, const std::string &s2);
    bool compairFuncCode(frame& v_data, int index, const enum_r_func_code& funcCode);

    // bool HandleData(const frame& v_data, const nextFrame& nextframe);


    // iot_data_item setItem(const int& gatewid, const int& device_id,const std::string& deviceaddr, const int& param_id,const std::string& param_name, const std::string& value);
    // void HandleByte_order(const frame &v_data, const enum_byte_order& type);
    // std::string HandleData_typeBit(const char& data, const int& bit);

    // void QueueData(int send_type, const iot_data_item& item);

    // std::vector<char> v_data;
};