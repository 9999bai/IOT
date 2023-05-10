#pragma once

#include "Database.h"
#include "mysql.h"
#include <memory>
#include <vector>

class MysqlDatabase : public Database
{
public:
    MysqlDatabase(sqlServer_info& info);
    ~MysqlDatabase();
    //设置字符集  连接mysl server参数
    void start();

private:
    //连接mysql server
    bool mysql_connect(const sqlServer_info& info);
    //mysql 错误码
    int mysql_errno();
    void getGatewayConfigure();
    void getDeviceConfigure(int gateway_id, std::vector<iot_device>& deviceConfList);
    void getTemplateConfigure(int template_id, std::vector<iot_template>& templateConfList);
    void getSubTemplateConfigure(int& subTemplate_id, std::vector<iot_sub_template>& subtemplateConfList);

    mymuduo::struct_serial proserial(std::string param);    
    struct_net pronet(std::string param);

    MYSQL mysql_;
    bool connection_;
};