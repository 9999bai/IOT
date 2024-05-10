#pragma once

#include "/usr/include/mosquitto-arm/mosquitto.h"
#include "/usr/include/mosquitto-arm/mosquittopp.h"
#include "/usr/include/mymuduo/Logger.h"
#include "/usr/include/mymuduo/CurrentThread.h"
#include "Observer/Observer.h"
// #include "Helper/myHelper.h"
// #include "mymuduo/CurrentThread.h"

class MqttClient : mosqpp::mosquittopp , public Observer
{
public:
    MqttClient(EventLoop *loop, const mqtt_info &mqttconf);
    ~MqttClient();

    // void restart();
    void start();
    void publicTopic(const std::string& msg);

    //回调函数
    void on_connect(int rc);
    void on_disconnect();
    void on_publish(int mid);
    void on_subscribe(int mid, int qos_count, const int *granted_qos); //订阅回调函数
    void on_unsubscribe(int mid);
    void on_message(const struct mosquitto_message *message);                             //订阅主题接收到消息

private:
    void connect();
    void subscribeTopic();
    // void retry();

    bool status_;

    int retryDelayMs_;
    static const int kMaxRetryDelayMs = 30 * 1000; // 30s
    static const int kInitRetryDelayMs = 3 * 1000; // 3s
    // mqtt_info mqttconf_;
};