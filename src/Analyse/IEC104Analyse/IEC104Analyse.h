#pragma once

#include "Analyse/Analyse.h"
#include "myHelper/myHelper.h"

class IEC104Analyse : public Analyse
{
public:
    IEC104Analyse();
    ~IEC104Analyse();

    void AnalyseFunc(const std::string &msg, const nextFrame &nextframe);

    bool FrameListCount(frame &data, std::vector<frame> &framelist);
    IEC104FrameType FrameType(const char type);
    void Analyse_U_Frame(const char ctr);
    void Analyse_S_Frame();

    void AnalyseTypeIdentify(const frame& data, int& index); // I帧 类型标识
    void AnalyseVSQ(const frame& data, IEC104TypeIdentity typeIdentity, int& index); // 可变结构限定词,最高位表示是否连续，其余位表示信息体个数
    void AnalyseCOT(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index); // 传送原因2个字节
    void AnalyseRTU(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index); // rtu地址
    void AnalyseBlock(const frame& data, IEC104TypeIdentity typeIdentity, bool cont, int objCount, int& index);
    void AnalyseAddr(const frame& data,  int& index, int& addr); // 信息体地址

    void AnalyseData_09(const frame &data, int &index, int &value);
    // 标度化值
    void AnalyseData_0D(const frame &data, int &index, float& value); // 解析数据
    // 遥脉 十六机制数据
    void AnalyseData_0F(const frame &data, int &index, int& value);
    //
    void AnalyseQDS(const frame& data, int& index); // 品质描述符

    // 遥信
    void AnalyseItem_01(const frame& data, bool cont, int objCount, int& index);
    // 遥测 归一化值
    void AnalyseItem_09(const frame& data, bool cont, int objCount, int& index);
    // 遥测 标度化值
    void AnalyseItem_0B(const frame& data, bool cont, int objCount, int& index);
    // 遥测 短浮点值
    void AnalyseItem_0D(const frame& data, bool cont, int objCount, int& index);
    // 遥脉 
    void AnalyseItem_0F(const frame &data, bool cont, int objCount, int &index);

private:
    // IEC104Type frameType_;    // 当前帧类型
    AnalyseResult result_;    // 解析完成下一步需要进行的操作
};
