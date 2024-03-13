#include "MysqlDatabase.h"
#include "/usr/include/mymuduo/Logger.h"
#include "/usr/include/mymuduo/Helper.h"
// #include "myHelper/myHelper.h"

#include <string.h>

MysqlDatabase::MysqlDatabase(sqlServer_info& info)
        : Database(info)
        , connection_(false)
{
    LOG_INFO("MysqlDatabase  ctor...");
}

MysqlDatabase::~MysqlDatabase()
{
    LOG_INFO("MysqlDatabase  dtor...");
	///< 关闭数据库连接
	mysql_close(&mysql_);
}

void MysqlDatabase::start()//mysql 字符集-- utf8
{
    if(nullptr != mysql_init(&mysql_))
    {
        LOG_INFO("mysql init suc...\n");
    }else{
        LOG_ERROR("mysql init failed... errno = %d\n", mysql_errno());
    }

    if(0 == mysql_options(&mysql_, MYSQL_SET_CHARSET_NAME, Character_.c_str()))
	{
		LOG_INFO("mysql character set %s suc...\n", Character_.c_str());
	}else{
        LOG_ERROR("mysql character set %s failed...errno = %d\n", Character_.c_str(), mysql_errno());
    }

    if(mysql_connect(info_))
    {
        connection_ = true;
        LOG_INFO("mysql connect to %s:%d suc...\n", info_.mysqlServer_ip.c_str(), info_.mysqlServer_port);
    }else{
        LOG_FATAL("mysql connect to %s:%d failed  errno = %d...\n", info_.mysqlServer_ip.c_str(), info_.mysqlServer_port, mysql_errno());
    }
    //连接成功后，查询数据
    getGatewayConfigure();
}

bool MysqlDatabase::mysql_connect(const sqlServer_info& info)
{
    ///< 连接的数据库（句柄、主机名、用户名、密码、数据库名、端口号、socket指针、标记）
	if (nullptr == mysql_real_connect(&mysql_, info.mysqlServer_ip.c_str(),\
                                                info.user_name.c_str(),\
                                                info.user_passwd.c_str(),\
                                                info.db_name.c_str(),\
                                                (unsigned int)info.mysqlServer_port,\
                                                nullptr, 0))
	{
        return false;
    }
    return true;
}

int MysqlDatabase::mysql_errno()
{
    return (int)::mysql_errno(&mysql_);
}

mymuduo::struct_serial MysqlDatabase::proserial(std::string param)
{
    mymuduo::struct_serial serial;
    std::vector<std::string> v_serialParam;
    if (!stringSplit(param, v_serialParam, ':'))
    {
        LOG_ERROR("proserial stringSplit-1 error");
        serial.valid = mymuduo::enum_data_res_error;
        abort();
    }
    try {
        serial.serialName = v_serialParam.at(0);
        std::vector<std::string> v_serialConf;
        if(!stringSplit(v_serialParam.at(1),v_serialConf, ','))
        {
            serial.valid = mymuduo::enum_data_res_error;
            LOG_ERROR("proserial stringSplit-2 error");
            abort();
        }
        serial.baudRate = std::atoi(v_serialConf.at(0).c_str());
        serial.dataBits = std::atoi(v_serialConf.at(1).c_str());
        std::string parity = v_serialConf.at(2);
        switch(*parity.data())
        {
            case 'N':
            case 'n':
                serial.parityBit = mymuduo::enum_data_parityBit_N;break;
            case 'E':
            case 'e':
                serial.parityBit = mymuduo::enum_data_parityBit_E;break;
            case 'O':
            case 'o':
                serial.parityBit = mymuduo::enum_data_parityBit_O;break;
            case 'S':
            case 's':
                serial.parityBit = mymuduo::enum_data_parityBit_S;break;
            default:
                serial.parityBit = mymuduo::enum_data_parityBit_N;break;
        }
        serial.stopBits = std::atoi(v_serialConf.at(3).c_str());
        serial.valid = mymuduo::enum_data_res_normal;
    }
    catch (const std::out_of_range &e)
    {
        serial.valid = mymuduo::enum_data_res_error;
        LOG_ERROR("proserial error std::out_of_range--");
        abort();
    }
    catch(...)
    {
        LOG_ERROR("proserial error --");
        abort();
    }
    LOG_INFO("proserial --  %s:%d,%d,%d,%d", serial.serialName.c_str(),\
            serial.baudRate,serial.dataBits,(int)serial.parityBit,serial.stopBits);
    return serial;
}

struct_net MysqlDatabase::pronet(std::string param)
{
    struct_net net;
    std::vector<std::string> v_netParam;
    if (!stringSplit(param, v_netParam, ':'))
    {
        LOG_ERROR("pronet stringSplit-1 error");
        net.valid = mymuduo::enum_data_res_error;
        abort();
    }
    try {
        net.ip = v_netParam.at(0);
        net.port = std::atoi(v_netParam.at(1).c_str());
        net.valid = mymuduo::enum_data_res_normal;
    }
    catch (const std::out_of_range &e)
    {
        net.valid = mymuduo::enum_data_res_error;
        LOG_ERROR("pronet error std::out_of_range--");
        abort();
    }
    catch(...)
    {
        LOG_ERROR("proserial error --");
        abort();
    }
    LOG_INFO("pronet %s:%d", net.ip.c_str(), net.port);
    return net;
}

void MysqlDatabase::getGatewayConfigure()
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char sql[200] = {0};
    snprintf(sql, 200, "select gateway_id,gateway_name,description,pro_mode+0,pro_name+0,net_serial,project_id,status+0 from iot_gateway where project_id = '%d' and status = 2;", Project_ID);
    // LOG_INFO("getGatewayConfigure sql: %s\n", sql);

    if (0 == mysql_real_query(&mysql_, sql, sizeof sql))
    {
        res = mysql_store_result(&mysql_);//装载结果集
        if(res == nullptr)
        {
            LOG_ERROR("MysqlDatabase::getGatewayConfigure 装载数据失败...errno = %d\n", mysql_errno());
        }
        else
        {
            while (row = mysql_fetch_row(res)) //取出结果集中内容
            {
                iot_gateway gateway;
                gateway.gateway_id = std::atoi(row[0]);
                gateway.gateway_name = std::string(row[1]);
                gateway.description = std::string(row[2] == nullptr ? ("") : (row[2]));
                int promode = std::atoi(row[3]);
                gateway.pro_mode = (enum_pro_mode)promode;

                int proname = std::atoi(row[4]);
                gateway.pro_name = (enum_pro_name)proname;

                if (gateway.pro_mode == enum_pro_mode_serial)
                {
                    gateway.serial = proserial(std::string(row[5]));
                }
                else if(gateway.pro_mode == enum_pro_mode_net)
                {
                    gateway.net = pronet(std::string(row[5]));
                }else{
                    LOG_FATAL("MysqlDatabase::getGatewayConfigure error.. gateway.pro_mode=%d\n", gateway.pro_mode);
                }
                gateway.project_id = std::atoi(row[6]);
                int status = std::atoi(row[7]);
                gateway.status = (enum_status)status;

                LOG_INFO("MysqlDatabase::getGatewayConfigure() %d--%s--%s--%d--%d--%d--%d\n", gateway.gateway_id,\
                                                    gateway.gateway_name.c_str(), gateway.description.c_str(),\
                                                    (int)gateway.pro_mode, (int)gateway.pro_name,\
                                                    gateway.project_id, (int)gateway.status);

                getDeviceConfigure(gateway.gateway_id, gateway.v_device);
                gatewayConfList_.emplace_back(gateway); //++
            }
        }
    }
    else
    {
        LOG_ERROR("MysqlDatabase::getGatewayConfigure 查询失败...errnr = %d\n", mysql_errno());
    }
    ///< 释放结果集
	mysql_free_result(res);
}

void MysqlDatabase::getDeviceConfigure(int gateway_id, std::vector<iot_device>& deviceConfList)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char sql[400] = {0};
    snprintf(sql, 400, "select gateway_id,device_id,device_addr,device_name,template_id,project_id,status+0 from iot_device where project_id = '%d' and status = 2 and gateway_id = '%d';", Project_ID, gateway_id);
    // LOG_INFO("getDeviceConfigure sql: %s\n", sql);

    if (0 == mysql_real_query(&mysql_, sql, (unsigned int)strlen(sql)))
    {
        res = mysql_store_result(&mysql_); //装载结果集
        if(res == nullptr)
        {
            LOG_ERROR("MysqlDatabase::getDeviceConfigure 装载数据失败...errno = %d\n", mysql_errno());
        }
        else
        {
			while (row = mysql_fetch_row(res))//取出结果集中内容
			{
                iot_device device;
                device.gateway_id = std::atoi(row[0]);
                device.device_id = std::atoi(row[1]);
                device.device_addr = std::string(row[2]);
                device.device_name = std::string(row[3]);
                device.template_id = std::atoi(row[4]);
                device.project_id = std::atoi(row[5]);
                int status = std::atoi(row[6]);
                device.status = (enum_status)status;
                
                LOG_INFO("MysqlDatabase::getDeviceConfigure() %d--%d--%s--%s--%d--%d--%d\n",\
                                                            device.gateway_id,device.device_id,\
                                                            device.device_addr.c_str(),device.device_name.c_str(),\
                                                            device.template_id,device.project_id,\
                                                            (int)device.status);
                if(device.template_id != 0)
                {
                    getTemplateConfigure(device.template_id, device.v_template);
                }
                deviceConfList.emplace_back(device);
            }
        }
    }
    else
    {
        LOG_ERROR("MysqlDatabase::getDeviceConfigure 查询失败...errnr = %d\n", mysql_errno());
    }
    ///< 释放结果集
	mysql_free_result(res);
}

void MysqlDatabase::getTemplateConfigure(int template_id, std::vector<iot_template>& templateConfList)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char sql[400] = {0};
    snprintf(sql, 400, "select template_id, register_addr,register_quantity,r_func_code+0,w_func_code+0,param_name,data_type+0,byte_order+0,correct_mode,param_id,data_unit,send_type,sub_template_id,priority+0 from iot_template where template_id = '%d';", template_id);
    // LOG_INFO("getTemplateConfigure sql: %s\n", sql);
    
    if (0 == mysql_real_query(&mysql_, sql, (unsigned int)strlen(sql)))
    {
        res = mysql_store_result(&mysql_);//装载结果集
        if(res == nullptr)
        {
            LOG_ERROR("MysqlDatabase::getTemplateConfigure 装载数据失败...errno = %d\n", mysql_errno());
        }
        else
        {
			while (row = mysql_fetch_row(res))//取出结果集中内容
			{
                iot_template templat;
                templat.template_id = (row[0] == nullptr) ? (0) : std::atoi(row[0]);
                templat.register_addr = (row[1] == nullptr) ? ("NULL") : (std::string(row[1]));
                templat.register_quantity = (row[2] == nullptr) ? (0) : ((int)std::atoi(row[2]));
                
                int r_func_code = (row[3] == nullptr) ? (0) : (std::atoi(row[3]));
                templat.r_func = (enum_r_func_code)r_func_code;

                int w_func_code = (row[4] == nullptr) ? (0) : (std::atoi(row[4]));
                if(w_func_code == 7)
                    w_func_code = 15;
                else if(w_func_code == 8)
                    w_func_code = 16;
                templat.w_func = (enum_w_func_code)w_func_code;

                templat.param_name = (row[5] == nullptr) ? ("NULL") : (std::string(row[5]));
                int data_type  = (row[6] == nullptr) ? (0) : (std::atoi(row[6]));
                templat.data_type = (enum_data_type)data_type;
                int byte_order = (row[7] == nullptr) ? (0) : (std::atoi(row[7]));
                templat.byte_order = (enum_byte_order)byte_order;

                templat.correct_mode = (row[8] == nullptr) ? ("NULL") : (std::string(row[8]));
                templat.param_id = (row[9] == nullptr) ? (0) : (std::atoi(row[9]));
                templat.data_unit = (row[10] == nullptr) ? ("NULL") : (std::string(row[10]));
                templat.send_type = (row[11] == nullptr) ? (0) : (std::atoi(row[11]));
                templat.sub_template_id = (row[12] == nullptr) ? (0) : (std::atoi(row[12]));
                int priority = (row[13] == nullptr) ? (1) : (std::atoi(row[13]));
                templat.priority = (enum_priority)priority;

                LOG_INFO("MysqlDatabase::getTemplateConfigure() --%d--%s--%d--%d--%d--%s--%d--%d--%s--%d--%s--%d--%d--%d--\n",\
                        (int)templat.template_id, templat.register_addr.c_str(), (int)templat.register_quantity,\
                        (int)templat.r_func, (int)templat.w_func, templat.param_name.c_str(),\
                        (int)templat.data_type, (int)templat.byte_order, templat.correct_mode.c_str(),\
                        templat.param_id, templat.data_unit.c_str(), templat.send_type,\
                        templat.sub_template_id, (int)templat.priority);

                if(0 != templat.sub_template_id)
                {
                    getSubTemplateConfigure(templat.sub_template_id, templat.v_sub_template);
                }
                templateConfList.emplace_back(templat);
            }
        }
    }
    else
    {
        LOG_ERROR("MysqlDatabase::getTemplateConfigure 查询失败...errnr = %d\n", mysql_errno());
    }
    ///< 释放结果集
	mysql_free_result(res);
}

void MysqlDatabase::getSubTemplateConfigure(int& subTemplate_id, std::vector<iot_sub_template>& subtemplateConfList)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char sql[400] = {0};
    snprintf(sql, 400, "select sub_template_id, param_name, param_id, s_addr,data_quantity,bit,register_addr,w_func_code+0,data_type+0,byte_type+0,correct_mode,data_unit,send_type,priority+0 from iot_sub_template where sub_template_id = '%d';", subTemplate_id);
    // LOG_INFO("getSubTemplateConfigure sql: %s\n", sql);
    if (0 == mysql_real_query(&mysql_, sql, (unsigned int)strlen(sql)))
    {
        res = mysql_store_result(&mysql_);//装载结果集
        if(res == nullptr)
        {
            LOG_ERROR("MysqlDatabase::getSubTemplateConfigure 装载数据失败...errno = %d\n", mysql_errno());
        }
        else
        {
			while (row = mysql_fetch_row(res))//取出结果集中内容
			{
                iot_sub_template sub_templat;
                sub_templat.sub_template_id = (row[0] == nullptr) ? (0) : std::atoi(row[0]);
                sub_templat.param_name = (row[1] == nullptr) ? ("NULL") : std::string(row[1]);
                sub_templat.param_id = (row[2] == nullptr) ? (0) : std::atoi(row[2]);
                sub_templat.s_addr = (row[3] == nullptr) ? (0) : std::atoi(row[3]);
                sub_templat.data_quantity = (row[4] == nullptr) ? (0) : std::atoi(row[4]);

                int bit = (row[5] == nullptr) ?(0) : (std::atoi(row[5]));
                sub_templat.bit = bit;
                sub_templat.register_addr = (row[6] == nullptr) ? ("NULL") : (std::string(row[6]));

                int w_func = (row[7] == nullptr) ? (0) : (std::atoi(row[7]));
                if(w_func > 6) // mysql 读enum超过9结果为1
                {
                    w_func+=8;
                }
                sub_templat.w_func = (enum_w_func_code)w_func;
                int data_type = (row[8] == nullptr) ? (0) : (std::atoi(row[8]));
                sub_templat.data_type = (enum_data_type)data_type;
                int byte_order = (row[9] == nullptr) ? (0) : (std::atoi(row[9]));
                sub_templat.byte_order = (enum_byte_order)byte_order;
                sub_templat.correct_mode = (row[10] == nullptr) ? ("NULL") : std::string(row[10]);
                sub_templat.data_unit = (row[11] == nullptr) ? ("NULL") : std::string(row[11]);
                sub_templat.send_type = (row[12] == nullptr) ? (0) : std::atoi(row[12]);
                int priority = (row[13] == nullptr) ? (1) : (std::atoi(row[13]));
                sub_templat.priority = (enum_priority)priority;
               
                subtemplateConfList.emplace_back(sub_templat);
                LOG_INFO("MysqlDatabase::getSubTemplateConfigure %d--%s--%d--%d--%d--%d--wfunc=%d--%d--%d--%s--%s--%d--%d",\
                        (int)sub_templat.sub_template_id, sub_templat.param_name.c_str(), (int)sub_templat.param_id,\
                        (int)sub_templat.s_addr, (int)sub_templat.data_quantity, (int)sub_templat.bit,(int)sub_templat.w_func,\
                        (int)sub_templat.data_type, (int)sub_templat.byte_order, sub_templat.correct_mode.c_str(),\
                        sub_templat.data_unit.c_str(), (int)sub_templat.send_type,(int)sub_templat.priority);
            }
        }
    }
    else
    {
        LOG_ERROR("MysqlDatabase::getSubTemplateConfigure 查询失败...errnr = %d\n", mysql_errno());
    }
    ///< 释放结果集
	mysql_free_result(res);
}