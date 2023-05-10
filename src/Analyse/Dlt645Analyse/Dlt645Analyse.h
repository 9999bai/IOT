#include "Analyse/Analyse.h"
#include <vector>

class Dlt645Analyse : public Analyse
{
public:
    Dlt645Analyse();
    ~Dlt645Analyse();

    void AnalyseFunc(const std::string &msg, const nextFrame &nextframe);


private:
    bool DLTCheckHead(const frame& src, const int& index);
    bool DLT645DataCheck(const frame& src, const char& check);
    bool DLTCheckLength(const frame &src, const char &length);
    bool DLT645MeteridCheck(const frame& hexMeter, const std::string& strMeter);

    std::string DLTAnalyseData(const frame& src, const iot_template& templat);

    std::vector<char> v_data;
};
