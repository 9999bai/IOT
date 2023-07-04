#pragma once

#include "Frame/Frame.h"
#include "myHelper/myHelper.h"

class BacnetipFrame : public Frame
{
public:
    BacnetipFrame(const iot_gateway& gatewayConf);
    ~BacnetipFrame();

    void start();
    
private:
    frame headFrame();
    void updateLength(frame &tmp);

    void PropertyReferences(const iot_template &templat, frame &data, int& count);
    void propertyIdentifity(const int& readValue, frame &data);

    const frame BVLC_;
    const frame NPDU_;  // 01 04 版本号，控制字04：需确认的请求数据单元
    const int pdutype_; // pdutype
    const int maxAPDUlength_; // 0x04: 1024
    const int invodeID_;    // 调用者ID
    const int readMultiple_;

    const int openingContextTag_; // 0x1E
    const int closingContextTag_; // 0x1F
};
