#pragma once 

#include "Mediator.h"
#include "Factory/OpcuaFactory/OpcuaFactory.h"
#include "open62541/open62541_gm.h"
#include "myHelper/myHelper.h"


class OpcUAMeiator : public Mediator
{
public:
    OpcUAMeiator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& opcuaFactory);
    ~OpcUAMeiator();

    void addControlFrame(const nextFrame& controlFrame);
    void start();

    void readValueArrayInit(UA_ReadValueId *src, int size);

    void secTimer(); // 定时器发送下一帧读

private:
    void onNextFrame(UA_Client *client);
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result = ENUM_Normal, int count=0, IEC104FrameType type=ENUM_Normal_Frame);

    // UA_StatusCode MultiRead(UA_Client *client, UA_ReadValueId *arrayItem, size_t arraySize);
    void AnalyseFunc(const int& index, const nextFrame &nextframe, const UA_Variant& out);

    bool OPCWriteValue(const iot_device& device, const iot_template &templat, const std::string &value);

    UA_Client *client;
    FramePtr opcuaFramePtr_;
    AnalysePtr opcuaAnalysePtr_;
};
