#pragma once

#include "Analyse/Analyse.h"
#include "myHelper/myHelper.h"
#include <vector>

class Pro188Analyse : public Analyse
{
public:
    Pro188Analyse();
    ~Pro188Analyse();
    
    void AnalyseFunc(const std::string &msg, const nextFrame &nextframe, void* pending);


private:
    bool CJT188CheckHead(const frame& data, const int& index);
    bool CJT188CheckLength(const frame& data, const u_int8_t& lengthChar);
    bool CJT188DataCheck(const frame &data, const char &check);
    bool CJT188MeteridCheck(const frame& data, const std::string& strMeter);

    std::string CJT188AnalyseData(const frame& data, const iot_template& templat);

    // std::vector<char> v_data;
};