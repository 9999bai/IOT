#pragma once

#include "Analyse/Analyse.h"
// #include <strings.h>
// #include <vector>
// #include <math.h>

class ModbusRtuAnalyse;
class ModbusTcpAnalyse : public Analyse
{
public:
    ModbusTcpAnalyse();
    ~ModbusTcpAnalyse();

    void AnalyseFunc(const std::string& msg, const nextFrame& nextframe);
    bool ModbustTcpMBAP(const frame& src, const iot_device& device, const iot_template& templat);

private:
    std::vector<char> v_data;
    ModbusRtuAnalyse *rtuAnalyse_;
};