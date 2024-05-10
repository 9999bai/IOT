#include "OpcuaAnalyse.h"

OpcUAAnalyse::OpcUAAnalyse()
            : Analyse()
{
    
}

OpcUAAnalyse::~OpcUAAnalyse()
{
    
}

void OpcUAAnalyse::AnalyseFunc(const std::string& msg, const nextFrame& nextframe, void* pending)
{
    bool resOK = true; // 解析是否成功
    enum_RW resRW;
    int index = std::atoi(msg.c_str());

    iot_device device = nextframe.second.first;
    std::vector<iot_template> templats = nextframe.second.second;
    if(index >= templats.size())
    {
        LOG_INFO("eoor OpcUAAnalyse::AnalyseFunc %d >= templats.size()...", index);
        return;
    }
    iot_template templat = templats.at(index);
    LOG_INFO("\t\t\t\t%s", templat.register_addr.c_str());
    if (templat.rw == enum_read)
    {
        resRW = enum_read;
        bool boolresult = true;
        std::string value("");
        UA_Variant out = *(UA_Variant*)pending;

        switch(templat.data_type)
        {
            case enum_data_type_OPC_bool:
            {
                value = ((*(UA_Boolean *)out.data)==true?("true"):("false"));
                LOG_INFO("UA_Boolean=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_sbyte:
            {
                value = std::to_string(*(UA_SByte *)out.data);
                LOG_INFO("UA_SByte=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_byte:
            {
                value = std::to_string(*(UA_Byte *)out.data);
                LOG_INFO("UA_Byte=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int16:
            {
                value = std::to_string(*(UA_Int16 *)out.data);
                LOG_INFO("UA_Int16=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint16:
            {
                value = std::to_string(*(UA_UInt16 *)out.data);
                LOG_INFO("UA_UInt16=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int32:
            {
                value = std::to_string(*(UA_Int32 *)out.data);
                LOG_INFO("UA_Int32=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint32:
            {
                value = std::to_string(*(UA_UInt32 *)out.data);
                LOG_INFO("UA_UInt32=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_int64:
            {
                value = std::to_string(*(UA_Int64 *)out.data);
                LOG_INFO("UA_Int64=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_uint64:
            {
                value = std::to_string(*(UA_UInt64 *)out.data);
                LOG_INFO("UA_UInt64=%s",value.c_str());
                break;
            }
            case enum_data_type_OPC_float:
            {
                float tmp = (float)*(UA_Float *)out.data;
                // char tmp[300];
                // sprintf(tmp, "%.2f", (float)*(UA_Float *)out.data);
                std::string str = std::to_string(tmp);
                value = str;
                LOG_INFO("UA_Float=%s", value.c_str());
                break;
            }
            case enum_data_type_OPC_double:
            {
                double tmp = (double)*(UA_Double *)out.data;
                // char tmp[300];
                // sprintf(tmp, "%.2f", (double)*(UA_Double *)out.data);
                std::string str = std::to_string(tmp);
                value = str;
                // value = std::to_string(tmp);
                LOG_INFO("UA_Double=%s", value.c_str());
                break;
            }
            case enum_data_type_OPC_string:
            {
                UA_String *ptr = (UA_String *)out.data;
                value = std::string((char*)ptr->data, ptr->length);
                LOG_INFO("UA_String=%s", value.c_str());
                break;
            }
            // case enum_data_type_OPC_datetime:
            // case enum_data_type_OPC_guid:
            case enum_data_type_OPC_bytestring:
            {
                UA_ByteString * ptr = (UA_ByteString *)out.data;
                value = std::string((char*)ptr->data,ptr->length);
                LOG_INFO("UA_ByteString=%s",value.c_str());
                break;
            }
            // case enum_data_type_OPC_xmlelement:
            // case enum_data_type_OPC_nodeid:
            // case enum_data_type_OPC_expandednodeid:
            // case enum_data_type_OPC_statuscode:
            // case enum_data_type_OPC_qualifiedname:
            // case enum_data_type_OPC_localizedtext:
            // {

                // break;
            // }
            default:{
                LOG_ERROR("OpcUAAnalyse::AnalyseFunc default...%d", (int)templat.data_type);
                boolresult = false;
                resOK = false;
                break;
            }
                // case enum_data_type_OPC_extensionobject:
                // case enum_data_type_OPC_datavalue:
                // case enum_data_type_OPC_variant:
                // case enum_data_type_OPC_diagnosticinfo:
        }
        if(boolresult)
        {
            QueueData(templat.send_type, setItem(device.gateway_id, device.device_id, device.device_addr, templat.param_id, templat.param_name, value));
        }
    }else if(templat.rw == enum_write)
    {
        resRW = enum_write;
        bool resOK = true; // 解析是否成功
        
    }
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(resOK, resRW, ENUM_Normal, 0, ENUM_Normal_Frame);
    }


        // if (out.type == &UA_TYPES[UA_TYPES_BOOLEAN])
        // {
        //     UA_Boolean *ptr = (UA_Boolean *)out.data;
        //     value = (*ptr==true ? ("true"):("false"));
        //     printf("UA_Boolean: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_SBYTE])
        // {
        //     UA_SByte *ptr = (UA_SByte *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_SByte: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_BYTE])
        // {
        //     UA_Byte *ptr = (UA_Byte *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Byte: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_INT16])
        // {
        //     UA_Int16 *ptr = (UA_Int16 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Int16: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_UINT16])
        // {
        //     UA_UInt16 *ptr = (UA_UInt16 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_UInt16: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_INT32])
        // {
        //     UA_Int32 *ptr = (UA_Int32 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Int32: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_UINT32])
        // {
        //     UA_UInt32 * ptr = (UA_UInt32 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_UInt32: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_INT64])
        // {
        //     UA_Int64 * ptr = (UA_Int64 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Int64: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_UINT64])
        // {
        //     UA_UInt64 * ptr = (UA_UInt64 *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_UInt64: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_FLOAT])
        // {
        //     UA_Float * ptr = (UA_Float *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Float: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_DOUBLE])
        // {
        //     UA_Double * ptr = (UA_Double *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_Double: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_STRING])
        // {
        //     UA_String *ptr = (UA_String *)out.data;
        //     value = std::string((char*)ptr->data,ptr->length);
        //     // printf("UA_String: %*.s\n", ptr->length, ptr->data);
        //     printf("UA_String: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_DATETIME])
        // {
        //     UA_DateTime * ptr = (UA_DateTime *)out.data;
        //     value = std::to_string(*ptr);
        //     printf("UA_DateTime: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_GUID])
        // {
        //     // UA_Guid * ptr = (UA_Guid *)out.data;
        //     // printf("UA_Guid: %u\n", *ptr);
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_BYTESTRING])
        // {
        //     UA_ByteString * ptr = (UA_ByteString *)out.data;
        //     value = std::string((char*)ptr->data,ptr->length);
        //     // printf("UA_ByteString: %*.s\n", ptr->length, ptr->data);
        //     printf("UA_ByteString: %s\n", value.c_str());
        // }
        // else if(out.type == &UA_TYPES[UA_TYPES_LOCALIZEDTEXT])
        // {
        //     UA_LocalizedText * ptr = (UA_LocalizedText *)out.data;
        //     value = std::string((char*)ptr->text.data, ptr->text.length);
        //     // printf("Text: %.*s\n", ptr->text.length, ptr->text.data);
        //     printf("Text: %s\n", value.c_str());
        // }else {
            // printf("AnalyseType error-----------\n");
            // boolresult = false;
            // resOK = false;
        // }
}

bool OpcUAAnalyse::HandleData(const frame& v_data, const nextFrame& nextframe)
{
    
}
