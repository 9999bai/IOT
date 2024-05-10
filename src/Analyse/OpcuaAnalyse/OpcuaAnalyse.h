#pragma once

#include "Analyse/Analyse.h"
#include "myHelper/myHelper.h"
#include "open62541/open62541_gm.h"

class OpcUAAnalyse : public Analyse
{
public:
    OpcUAAnalyse();
    ~OpcUAAnalyse();

    void AnalyseFunc(const std::string& msg, const nextFrame& nextframe, void* pending);
    bool HandleData(const frame& v_data, const nextFrame& nextframe);

    // void MultiRead(UA_Client *client, UA_ReadValueId *itemArray, const int &arraySize);

private:
    // void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type);

};
