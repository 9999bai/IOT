#include "ModbusRtuMediator.h"
#include "/usr/include/mymuduo/TcpConnection.h"
#include "/usr/include/mymuduo/EventLoop.h"
#include "/usr/include/mymuduo/ThreadPool.h"


// 有新连接 --> 定时器开启（发送数据定时器, 获取数据定时器）
//                              --> 有数据返回 --> 存入buff --> 解析数据
//                              --> 没有数据返回 --> 定时器定时继续发送下一帧数据

ModbusRtuMediator::ModbusRtuMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& modbusRtuFactory)
                    : Mediator(loop, gateway, poolPtr)
                    , modbusrtuAnalysePtr_(modbusRtuFactory->createAnalyse())
                    , serialPortPtr_(modbusRtuFactory->createNetSerial(enum_netserial_serialport, gateway))
                    , modbusrtuFramePtr_(modbusRtuFactory->createFrame(gateway))
{
    //解析完数据的回调函数  立即发送下一帧数据  (速度太快)
    modbusrtuAnalysePtr_->setAnalyseFinishCallback(std::bind(&ModbusRtuMediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    // observerPtr_->setObserverRecvCallback(std::bind(&ModbusRtuMediator::onObserverRecv, this));
    serialPortPtr_->setNextFrameCallback(std::bind(&ModbusRtuMediator::onNextFrame, this));
    serialPortPtr_->setMessageCallback(std::bind(&ModbusRtuMediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

ModbusRtuMediator::~ModbusRtuMediator()
{

}

void ModbusRtuMediator::secTimer()
{

}

void ModbusRtuMediator::start()
{
    modbusrtuFramePtr_->start();              // 组合数据帧
    serialPortPtr_->start(ModbusRtu_Freq);    // 打开串口,定时发送请求帧频率
}

//modbusRTU 控制帧加入待发送队列
void ModbusRtuMediator::addControlFrame(const nextFrame& controlFrame)
{
    modbusrtuFramePtr_->addControlFrame(controlFrame);
}

//serial 接收串口数据-->解析
void ModbusRtuMediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, modbusrtuAnalysePtr_, msg, sendedFrame_));
}

void ModbusRtuMediator::onNextFrame()
{
    nextFrame next;
    if(modbusrtuFramePtr_->getNextReadFrame(next))
    {
        sendedFrame_ = next;
        std::string buf(next.first.begin(), next.first.end());
        serialPortPtr_->SendData(buf);
        printFrame("TX", frame(next.first.begin(), next.first.end()));
    }
    else
    {
        LOG_ERROR("ModbusRtuMediator::sendNextFrame  error........");
    }
}

void ModbusRtuMediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(ok, rw, result, count, type);
    }
}

