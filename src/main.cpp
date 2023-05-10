#include <iostream>

#include "GatewayManage.h"
#include "Database/MysqlDatabase/MysqlDatabase.h"
#include "myHelper/myHelper.h"

int main()
{
    LOG_INFO("main threadID = %d",CurrentThread::tid());

    //---------MYSQL------------
    sqlServer_info info;
    info.mysqlServer_ip = "192.168.2.105";
    info.db_name = "test";
    info.user_name = "root";
    info.user_passwd = "123456";

    DatabasePtr mysqlDatabasePtr = std::make_shared<MysqlDatabase>(info);
    mysqlDatabasePtr->start();
    std::vector<iot_gateway> v_gateway = mysqlDatabasePtr->getSqlConfigData();

    //---------MQTT------------
    mqtt_info mqttconf;
    mqttconf.hostname = "192.168.2.102";
    mqttconf.port = 1883;
    mqttconf.username = "admin";
    mqttconf.passwd = "bai123456";
    mqttconf.publishTopic = "/pub/test";
    mqttconf.subscribeTopic = "/sub/test";

    EventLoop loop;
    GatewayManage manage(&loop, v_gateway, mqttconf);
    // manage.setMQTTSendCallback(std::bind(&MQTTClient::publicTopic, &mqttclient, std::placeholders::_1));
    manage.start();
    loop.loop();

    return 0;
}