#pragma once

#include "Frame/Frame.h"
#include "myHelper/myHelper.h"


class Dlt645Frame : public Frame
{
public:
    Dlt645Frame(const iot_gateway& gatewayConf);
    ~Dlt645Frame();

    void start();

private:
    // // 字符串 "01 02 03 04 05 06" ---> hex 01 02 03 04 05 06
    // frame DLT645StrToHex(const std::string& meterid);

    // // hex 01 02 03 04 05 06 ---> 06 05 04 03 02 01
    // void ReverseOrder(const frame& src, frame& dest);



};