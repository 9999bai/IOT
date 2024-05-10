#include "OpcuaMediator.h"

OpcUAMeiator::OpcUAMeiator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& opcuaFactory)
            : Mediator(loop, gateway, poolPtr)
            , opcuaFramePtr_(opcuaFactory->createFrame(gateway))
            , opcuaAnalysePtr_(opcuaFactory->createAnalyse())
{
    opcuaAnalysePtr_->setAnalyseFinishCallback(std::bind(&OpcUAMeiator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
}

OpcUAMeiator::~OpcUAMeiator()
{

}

void OpcUAMeiator::addControlFrame(const nextFrame& controlFrame)
{
    opcuaFramePtr_->addControlFrame(controlFrame);
}

void OpcUAMeiator::readValueArrayInit(UA_ReadValueId *src, int size)
{
    for (int j = 0; j < size; ++j)
    {
        UA_ReadValueId_init(&src[j]);
        src[j].attributeId = UA_ATTRIBUTEID_VALUE;
    }
}

void OpcUAMeiator::start()
{
    opcuaFramePtr_->start();

    client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    std::string loginfo("opc.tcp://");
    loginfo = loginfo + gateway_.net.ip + ":" + std::to_string(gateway_.net.port);

    UA_StatusCode retval = UA_Client_connect(client, loginfo.c_str());
    if (retval != UA_STATUSCODE_GOOD) 
    {
        LOG_INFO("OPCUA start failed...%d", (int)retval);
		UA_Client_delete(client);
        return;
    }else{
        LOG_INFO("OPCUA start suc...%s", loginfo.c_str());
    }

    onNextFrame(client);
}

void OpcUAMeiator::onNextFrame(UA_Client *client)
{
    structNextFrame next;
    if (opcuaFramePtr_->getNextReadFrame(next))
    {
        sendFrame_ = next;
        iot_device device = next.nextframe.second.first;
        std::vector<iot_template> v_templat = next.nextframe.second.second;

        if(next.rw == enum_read) // 读
        {
            int arraySize = v_templat.size();
            UA_ReadValueId itemArray[arraySize];
            for (int i = 0; i < arraySize; ++i)
            {
                UA_ReadValueId_init(&itemArray[i]);
                itemArray[i].attributeId = UA_ATTRIBUTEID_VALUE;

                std::string str(v_templat.at(i).register_addr);
                itemArray[i].nodeId = UA_NODEID_STRING(std::atoi(device.device_addr.c_str()), const_cast<char *>(str.c_str()));
                // printNodeId(&itemArray[i].nodeId);
            }

            UA_ReadRequest request;
            UA_ReadRequest_init(&request);
            request.nodesToRead = &itemArray[0];
            request.nodesToReadSize = arraySize;
            UA_ReadResponse response = UA_Client_Service_read(client, request);
            
            poolPtr_->run(std::bind(&Analyse::OPCMultiRead, opcuaAnalysePtr_, client, response, &itemArray[0], arraySize, sendFrame_));
        }
        else if(next.rw == enum_write) // 写
        {
            // LOG_INFO("OpcUAMeiator::onNextFrame %d", (int)v_templat.size());
            if (v_templat.size() > 0)
            {
                iot_template templat = v_templat.at(0);
                std::string tmp(next.nextframe.first.begin(), next.nextframe.first.end());
                // 写
                bool res = OPCWriteValue(device, templat, tmp);
                HandleAnalyseFinishCallback(res, enum_write);
            }
        }
    }
    else
    {
        LOG_ERROR("opc getNextReadFrame error...");
    }
}

bool OpcUAMeiator::OPCWriteValue(const iot_device& device, const iot_template& templat, const std::string& value)
{
    UA_StatusCode res;
    UA_Variant myVariant;
    UA_Variant_init(&myVariant);

    switch(templat.data_type)
    {
        case enum_data_type_OPC_bool:
        {
            UA_Boolean boolvalue = (value == "true" || value == "1") ? true : false;
            UA_Variant_setScalarCopy(&myVariant, &boolvalue, &UA_TYPES[UA_TYPES_BOOLEAN]);
            break;
        }
        case enum_data_type_OPC_sbyte:
        {
            UA_SByte sbytevalue = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &sbytevalue, &UA_TYPES[UA_TYPES_SBYTE]);
            break;
        }
        case enum_data_type_OPC_byte:
        {
            UA_Byte bytevalue = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &bytevalue, &UA_TYPES[UA_TYPES_BYTE]);
            break;
        }
        case enum_data_type_OPC_int16:
        {
            UA_Int16 int16value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &int16value, &UA_TYPES[UA_TYPES_INT16]);
            break;
        }
        case enum_data_type_OPC_uint16:
        {
            UA_UInt16 uint16value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &uint16value, &UA_TYPES[UA_TYPES_UINT16]);
            break;
        }
        case enum_data_type_OPC_int32:
        {
            UA_Int32 int32value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &int32value, &UA_TYPES[UA_TYPES_INT32]);
            break;
        }
        case enum_data_type_OPC_uint32:
        {
            UA_UInt32 uint32value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &uint32value, &UA_TYPES[UA_TYPES_UINT32]);
            break;
        }
        case enum_data_type_OPC_int64:
        {
            UA_Int64 int64value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &int64value, &UA_TYPES[UA_TYPES_INT64]);
            break;
        }
        case enum_data_type_OPC_uint64:
        {
            UA_UInt64 uint64value = std::atoi(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &uint64value, &UA_TYPES[UA_TYPES_UINT64]);
            break;
        }
        case enum_data_type_OPC_float:
        {
            UA_Float floatvalue = std::atof(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &floatvalue, &UA_TYPES[UA_TYPES_FLOAT]);
            break;
        }
        case enum_data_type_OPC_double:
        {
            UA_Double doublevalue = std::atof(value.c_str());
            UA_Variant_setScalarCopy(&myVariant, &doublevalue, &UA_TYPES[UA_TYPES_DOUBLE]);
            break;
        }
        case enum_data_type_OPC_string:
        {
            UA_String stringvalue = UA_STRING(const_cast<char*>(value.c_str()));
            UA_Variant_setScalarCopy(&myVariant, &stringvalue, &UA_TYPES[UA_TYPES_STRING]);
            break;
        }
        // case enum_data_type_OPC_datetime:
        // case enum_data_type_OPC_guid:
        case enum_data_type_OPC_bytestring:
        {
            UA_ByteString bytestringvalue = UA_STRING(const_cast<char*>(value.c_str()));
            UA_Variant_setScalarCopy(&myVariant, &bytestringvalue, &UA_TYPES[UA_TYPES_BYTESTRING]);
            break;
        }
        case enum_data_type_OPC_localizedtext:
        {
            UA_LocalizedText textvalue = UA_LOCALIZEDTEXT("en-US", const_cast<char*>(value.c_str()));
            UA_Variant_setScalarCopy(&myVariant, &textvalue, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            break;
        }
        default:{
            LOG_ERROR("OpcUAAnalyse::AnalyseFunc default...%d", (int)templat.data_type);
            // boolresult = false;
            // resOK = false;
            break;
        }
        // case enum_data_type_OPC_xmlelement:
        // case enum_data_type_OPC_nodeid:
        // case enum_data_type_OPC_expandednodeid:
        // case enum_data_type_OPC_statuscode:
        // case enum_data_type_OPC_qualifiedname:
        // case enum_data_type_OPC_extensionobject:
        // case enum_data_type_OPC_datavalue:
        // case enum_data_type_OPC_variant:
        // case enum_data_type_OPC_diagnosticinfo:
    }
    res = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(std::atoi(device.device_addr.c_str()),\
     const_cast<char*>(templat.register_addr.c_str())), &myVariant);

    return res == UA_STATUSCODE_GOOD;
}

void OpcUAMeiator::secTimer()
{
    static int count = 0;
    if(++count == 2)
    {
        onNextFrame(client);//获取下一帧读
        count = 0;
    }
}

void OpcUAMeiator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(ok, rw, result, count, type);
    }
}

void OpcUAMeiator::AnalyseFunc(const int& index, const nextFrame &nextframe, const UA_Variant& out)
{
    // bool resOK = true; // 解析是否成功
    // enum_RW resRW;

    // iot_device device = nextframe.second.first;
    // std::vector<iot_template> templats = nextframe.second.second;
    // if(index >= templats.size())
    // {
    //     LOG_INFO("eoor OpcUAAnalyse::AnalyseFunc %d >= templats.size()...", index);
    //     return;
    // }
    // iot_template templat = templats.at(index);
    // LOG_INFO("\t\t\t\t%s", templat.register_addr.c_str());
    // if (templat.rw == enum_read)
    // {
    //     resRW = enum_read;
    //     bool boolresult = true;
    //     std::string value("");

    //     switch(templat.data_type)
    //     {
    //         case enum_data_type_OPC_bool:
    //         {
    //             value = ((*(UA_Boolean *)out.data)==true?("true"):("false"));
    //             LOG_INFO("UA_Boolean=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_sbyte:
    //         {
    //             value = std::to_string(*(UA_SByte *)out.data);
    //             LOG_INFO("UA_SByte=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_byte:
    //         {
    //             value = std::to_string(*(UA_Byte *)out.data);
    //             LOG_INFO("UA_Byte=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_int16:
    //         {
    //             value = std::to_string(*(UA_Int16 *)out.data);
    //             LOG_INFO("UA_Int16=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_uint16:
    //         {
    //             value = std::to_string(*(UA_UInt16 *)out.data);
    //             LOG_INFO("UA_UInt16=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_int32:
    //         {
    //             value = std::to_string(*(UA_Int32 *)out.data);
    //             LOG_INFO("UA_Int32=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_uint32:
    //         {
    //             value = std::to_string(*(UA_UInt32 *)out.data);
    //             LOG_INFO("UA_UInt32=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_int64:
    //         {
    //             value = std::to_string(*(UA_Int64 *)out.data);
    //             LOG_INFO("UA_Int64=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_uint64:
    //         {
    //             value = std::to_string(*(UA_UInt64 *)out.data);
    //             LOG_INFO("UA_UInt64=%s",value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_float:
    //         {
    //             float tmp = (float)*(UA_Float *)out.data;
    //             // char tmp[300];
    //             // sprintf(tmp, "%.2f", (float)*(UA_Float *)out.data);
    //             std::string str = std::to_string(tmp);
    //             value = str;
    //             LOG_INFO("UA_Float=%s", value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_double:
    //         {
    //             double tmp = (double)*(UA_Double *)out.data;
    //             // char tmp[300];
    //             // sprintf(tmp, "%.2f", (double)*(UA_Double *)out.data);
    //             std::string str = std::to_string(tmp);
    //             value = str;
    //             // value = std::to_string(tmp);
    //             LOG_INFO("UA_Double=%s", value.c_str());
    //             break;
    //         }
    //         case enum_data_type_OPC_string:
    //         {
    //             UA_String *ptr = (UA_String *)out.data;
    //             value = std::string((char*)ptr->data, ptr->length);
    //             LOG_INFO("UA_String=%s", value.c_str());
    //             break;
    //         }
    //         // case enum_data_type_OPC_datetime:
    //         // case enum_data_type_OPC_guid:
    //         case enum_data_type_OPC_bytestring:
    //         {
    //             UA_ByteString * ptr = (UA_ByteString *)out.data;
    //             value = std::string((char*)ptr->data,ptr->length);
    //             LOG_INFO("UA_ByteString=%s",value.c_str());
    //             break;
    //         }
    //         // case enum_data_type_OPC_xmlelement:
    //         // case enum_data_type_OPC_nodeid:
    //         // case enum_data_type_OPC_expandednodeid:
    //         // case enum_data_type_OPC_statuscode:
    //         // case enum_data_type_OPC_qualifiedname:
    //         // case enum_data_type_OPC_localizedtext:
    //         // {

    //             // break;
    //         // }
    //         default:{
    //             LOG_ERROR("OpcUAAnalyse::AnalyseFunc default...%d", (int)templat.data_type);
    //             boolresult = false;
    //             resOK = false;
    //             break;
    //         }
    //             // case enum_data_type_OPC_extensionobject:
    //             // case enum_data_type_OPC_datavalue:
    //             // case enum_data_type_OPC_variant:
    //             // case enum_data_type_OPC_diagnosticinfo:
    //     }
    //     if(boolresult)
    //     {
    //         QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, value));
    //     }
    // }else if(templat.rw == enum_write)
    // {
    //     resRW = enum_write;
    // }
    // // if(analyseFinishCallback_)
    // // {
    // //     analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame);
    // // }
}
