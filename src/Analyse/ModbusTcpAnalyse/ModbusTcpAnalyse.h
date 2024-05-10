#pragma once

#include "Analyse/Analyse.h"
// #include "ModbusRtuAnalyse.h"

class ModbusTcpAnalyse : public Analyse
{
public:
    ModbusTcpAnalyse();
    ~ModbusTcpAnalyse();

    void AnalyseFunc(const std::string& msg, const nextFrame& nextframe, void* pending);
    bool ModbustTcpMBAP(const frame& src, const iot_device& device, const iot_template& templat);

private:
    // std::vector<char> v_data;
};