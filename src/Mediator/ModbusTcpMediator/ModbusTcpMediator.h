#include "Mediator.h"
#include "Factory/ModbusTcpFactory/ModbusTcpFactory.h"
#include "myHelper/myHelper.h"
// #include <mutex>
// #include <atomic>

class ModbusTcpMediator : public Mediator
{
public:
    ModbusTcpMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& modbusTcpFactory);
    ~ModbusTcpMediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);
    void secTimer();

private:
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type);     //解析完成后

    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息

    // nextFrame sendedFrame_;     //当前发送的数据及解析参数

    NetSerialPtr tcpclientPtr_;
    AnalysePtr modbustcpAnalysePtr_;
    FramePtr modbustcpFramePtr_;
};
