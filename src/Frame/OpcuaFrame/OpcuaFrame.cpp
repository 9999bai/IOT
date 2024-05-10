#include "OpcuaFrame.h"
#include "open62541/open62541_gm.h"


OpcUAFrame::OpcUAFrame(const iot_gateway& gatewayConf)
            : Frame(gatewayConf, 0, "opcua")
{

}

OpcUAFrame::~OpcUAFrame()
{

}

void OpcUAFrame::readValueArrayInit(UA_ReadValueId *src1, int size1, UA_ReadValueId* src2, int size2)
{
    for (int j = 0; j < size1; ++j)
    {
        UA_ReadValueId_init(&src1[j]);
        src1[j].attributeId = UA_ATTRIBUTEID_VALUE;
    }

    for (int j = 0; j < size2; ++j)
    {
        UA_ReadValueId_init(&src1[j]);
        src2[j].attributeId = UA_ATTRIBUTEID_VALUE;
    }
}

void OpcUAFrame::start()
{
    std::vector<iot_template> v_templat;
    for(iot_device& device : gatewayConf_.v_device)
    {
        int index1 = 0;
        int index2 = 0;
        int count = 0;
        int arrsize = device.v_template.size() % OPCREADSIZE;
        for (int i = 0; i < device.v_template.size(); i++)
        {
            iot_template templat = device.v_template.at(i);
            templat.rw = enum_read;
            v_templat.emplace_back(templat);

            if(OPCREADSIZE > device.v_template.size()) // 节点总数小于opcsize
            {
                index2++; // 标记itemArray2是否已满
                addQueue(arrsize, index2, device, v_templat);
            }
            else if((device.v_template.size() % OPCREADSIZE) == 0) // 节点总数等于opcsize的倍数
            {
                index1++;
                addQueue(OPCREADSIZE, index1, device, v_templat);
            }
            else  // 节点总数大于opcsize
            {
                if ((device.v_template.size() - count) <= arrsize)
                {
                    index2++;
                    addQueue(arrsize, index2, device, v_templat);
                }
                else
                {
                    index1++;
                    addQueue(OPCREADSIZE, index1, device, v_templat);
                }
                count++;
            }
        }
    }
}

void OpcUAFrame::addQueue(const int &arrsize, int &index, const iot_device &device, std::vector<iot_template>& v_templat)
{
    if(arrsize == index)
    {
        nextFrame nextf(frame(), pair_frame(device, v_templat));
        R_Vector.emplace_back(nextf);
        index = 0; // 初始化=0 数组下标
        v_templat.clear();

        LOG_INFO("OPC ITEM ++ ");
    }
}

// void OpcUAFrame::start()
// {
//     std::vector<iot_template> v_templat;
//     for(iot_device& device : gatewayConf_.v_device)
//     {
//         int index1 = 0;
//         int index2 = 0;
//         int count = 0;
//         int arrsize = device.v_template.size() % OPCREADSIZE;
//         UA_ReadValueId itemArray1[OPCREADSIZE];
//         UA_ReadValueId itemArray2[arrsize];
//         readValueArrayInit(itemArray1, OPCREADSIZE, itemArray2, arrsize);
//         for (int i = 0; i < device.v_template.size(); i++)
//         {
//             iot_template templat = device.v_template.at(i);
//             templat.rw = enum_read;
//             v_templat.emplace_back(templat);

//             if(OPCREADSIZE > device.v_template.size()) // 节点总数小于opcsize
//             {
//                 itemArray2[index2].attributeId = UA_ATTRIBUTEID_VALUE;
//                 itemArray2[index2].nodeId = UA_NODEID_STRING(std::atoi(device.device_addr.c_str()), const_cast<char*>(templat.register_addr.c_str()));
                
//                 printNodeId(&itemArray2[index2].nodeId);
//                 index2++; // 标记itemArray2是否已满
//                 addQueue(itemArray2, arrsize, index2, device, v_templat);
//             }
//             else if((device.v_template.size() % OPCREADSIZE) == 0) // 节点总数等于opcsize的倍数
//             {
//                 itemArray1[index1].attributeId = UA_ATTRIBUTEID_VALUE;
//                 itemArray1[index1].nodeId = UA_NODEID_STRING(std::atoi(device.device_addr.c_str()), const_cast<char*>(templat.register_addr.c_str()));
                
//                 printNodeId(&itemArray1[index1].nodeId);
//                 index1++;
//                 addQueue(itemArray1, OPCREADSIZE, index1, device, v_templat);
//             }
//             else  // 节点总数大于opcsize
//             {
//                 if ((device.v_template.size() - count) <= arrsize)
//                 {
//                     itemArray2[index2].attributeId = UA_ATTRIBUTEID_VALUE;
//                     itemArray2[index2].nodeId = UA_NODEID_STRING(std::atoi(device.device_addr.c_str()), const_cast<char*>(templat.register_addr.c_str()));
                    
//                     printNodeId(&itemArray2[index2].nodeId);
//                     index2++;
//                     addQueue(itemArray2, arrsize, index2, device, v_templat);
//                 }
//                 else
//                 {
//                     itemArray1[index1].attributeId = UA_ATTRIBUTEID_VALUE;
//                     itemArray1[index1].nodeId = UA_NODEID_STRING(std::atoi(device.device_addr.c_str()), const_cast<char*>(templat.register_addr.c_str()));
                    
//                     printNodeId(&itemArray1[index1].nodeId);
//                     index1++;
//                     addQueue(itemArray1, OPCREADSIZE, index1, device, v_templat);
//                 }
//                 count++;
//             }
//         }
//     }
// }