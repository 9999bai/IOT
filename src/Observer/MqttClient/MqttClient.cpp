#include "MqttClient.h"
#include "/usr/include/mymuduo/EventLoop.h"

const int MqttClient::kMaxRetryDelayMs;
const int MqttClient::kInitRetryDelayMs;

MqttClient::MqttClient(EventLoop *loop, const mqtt_info& mqttconf) 
            : Observer(loop, mqttconf)
            , status_(false)
            , retryDelayMs_(kInitRetryDelayMs)
{

}

MqttClient::~MqttClient()
{
    // status_ = false;
    mosqpp::lib_cleanup();
}

void MqttClient::start()
{
    if(!status_)
    {
        LOG_INFO(" MQTT [%s] start...", mqttconf_.hostname.c_str());
        connect();
    }else {
        LOG_INFO(" MqttClient::start  error (status_ = true) ");
    }
}

// void MqttClient::restart()
// {
    // status_ = false;
    // if(MOSQ_ERR_SUCCESS == mosqpp::lib_cleanup())
    // {
    //     start();
    // }
// }

void MqttClient::connect()
{
    if(MOSQ_ERR_SUCCESS != mosqpp::lib_init())
    {
        LOG_ERROR("MQTT lib_init errno...");
        return;
    }
    int res = username_pw_set(mqttconf_.username.c_str(), mqttconf_.passwd.c_str());
    if(res == MOSQ_ERR_SUCCESS)
    {
        int ret = connect_async(mqttconf_.hostname.c_str(), mqttconf_.port, 20);
        if (MOSQ_ERR_SUCCESS == ret)
        {
            int loopret = loop_start();
            if(loopret != MOSQ_ERR_SUCCESS)
            {
                LOG_ERROR("MQTT loop_start errno=%d...",loopret);
            }
        }
        else
        {
            LOG_ERROR("MQTT [%s] connected failed  errno=%d... ", mqttconf_.hostname.c_str(), ret);
            // retry();
        }
    }
    else{
        LOG_ERROR("MQTT [%s] username_pw_set errno=%d... ",mqttconf_.hostname.c_str(), res);
    }
}

// void MqttClient::retry()
// {
//     LOG_INFO(" MQTT [%s] retry ... ", mqttconf_.hostname.c_str());
//     mosqpp::lib_cleanup();
//     loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&MqttClient::start, this));
//     retryDelayMs_ = std::min(retryDelayMs_*2, kMaxRetryDelayMs);
//     // LOG_INFO("MQTT [%s] retry 2... ", mqttconf_.hostname.c_str());
// }

void MqttClient::subscribeTopic()
{
    int ret = subscribe(NULL, mqttconf_.subscribeTopic.c_str());
    if (ret != MOSQ_ERR_SUCCESS)
    {
        LOG_ERROR("MQTT [%s] subscribeTopic [ %s ] errno=%d...", mqttconf_.hostname.c_str(), mqttconf_.subscribeTopic.c_str(),ret);
    }else{
        LOG_INFO("MQTT [%s] subscribeTopic [ %s ] suc", mqttconf_.hostname.c_str(), mqttconf_.subscribeTopic.c_str());
    }
}

void MqttClient::publicTopic(const std::string& msg)
{
    // LOG_INFO("public msg = %s,  msg.size = %d", msg.c_str(), msg.size());
    if(status_)
    {
        int ret = publish(NULL, mqttconf_.publishTopic.c_str(), msg.size(), (const void *)msg.c_str());
        if (ret != MOSQ_ERR_SUCCESS)
        {
            LOG_ERROR("MQTT publish errno=%d", ret);
        }
    }
}

void MqttClient::on_connect(int rc)
{
    status_ = true;
    LOG_INFO("MQTT [%s] 已连接成功, threadid=%d", mqttconf_.hostname.c_str(), CurrentThread::tid());
    //连接成功--订阅消息
    subscribeTopic();
}

void MqttClient::on_disconnect()
{
    status_ = false;
    LOG_INFO("MQTT [%s] 已断开, threadid=%d", mqttconf_.hostname.c_str(), CurrentThread::tid());
}

void MqttClient::on_publish(int mid)
{
    // LOG_INFO("MQTT [%s] 发布成功, threadid=%d", mqttconf_.hostname.c_str(), CurrentThread::tid());
}

void MqttClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    LOG_INFO("MQTT [%s] 订阅成功, threadid=%d", mqttconf_.hostname.c_str(), CurrentThread::tid());
}

void MqttClient::on_unsubscribe(int mid)//订阅回调函数
{
    LOG_INFO("MQTT [%s] 取消订阅成功, threadid=%d", mqttconf_.hostname.c_str(), CurrentThread::tid());
}

void MqttClient::on_message(const struct mosquitto_message *message)
{
    bool res = false;
    mosqpp::topic_matches_sub(mqttconf_.subscribeTopic.c_str(), message->topic, &res);
    if (res)
    {
        std::string topic = (char*)message->topic;
        std::string strMsg = (char *)message->payload;
        // std::cout << "mqtt recv thread id = " << this_thread::get_id() << endl;
        // cout << "来自<" << message->topic << ">的消息：" << strRcv << std::endl;

        if(observerRecvCallabck_)
        {
            observerRecvCallabck_(topic, strMsg);
        }
    }
    else
    {
        LOG_INFO("MqttClient::on_message error...");
    }
}
