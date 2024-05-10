#pragma once


#include "Frame/Frame.h"
#include "myHelper/myHelper.h"
#include "open62541/open62541_gm.h"

class OpcUAFrame : public Frame
{
public:
    OpcUAFrame(const iot_gateway& gatewayConf);
    ~OpcUAFrame();

    void readValueArrayInit(UA_ReadValueId *src1, int size1, UA_ReadValueId* src2, int size2);

    void start();

    /*
        arrsize:    数组大小
        index:      当前数组是否满
    */
    void addQueue(const int &arrsize, int &index, const iot_device &device, std::vector<iot_template>& v_templat);


};
