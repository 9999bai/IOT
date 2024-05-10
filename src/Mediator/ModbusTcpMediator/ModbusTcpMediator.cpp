#include "ModbusTcpMediator.h"
#include "/usr/include/mymuduo/Buffer.h"

ModbusTcpMediator::ModbusTcpMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& modbusTcpFactory)
                : Mediator(loop, gateway, poolPtr)
                , modbustcpAnalysePtr_(modbusTcpFactory->createAnalyse())
                , tcpclientPtr_(modbusTcpFactory->createNetSerial(enum_netserial_tcp, gateway))
                , modbustcpFramePtr_(modbusTcpFactory->createFrame(gateway))
{
    modbustcpAnalysePtr_->setAnalyseFinishCallback(std::bind(&ModbusTcpMediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    tcpclientPtr_->setNextFrameCallback(std::bind(&ModbusTcpMediator::onNextFrame, this));
    tcpclientPtr_->setMessageCallback(std::bind(&ModbusTcpMediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

ModbusTcpMediator::~ModbusTcpMediator()
{

}

void ModbusTcpMediator::secTimer()
{

}

void ModbusTcpMediator::start()
{
    modbustcpFramePtr_->start(); // 组合数据帧
    tcpclientPtr_->start(ModbusTcp_Freq);      // tcp
}

void ModbusTcpMediator::addControlFrame(const nextFrame& controlFrame)
{
    modbustcpFramePtr_->addControlFrame(controlFrame);
}

void ModbusTcpMediator::onNextFrame()
{
    nextFrame next;
    if(modbustcpFramePtr_->getNextReadFrame(next))
    {
        sendFrame_ = next;
        std::string buf(next.first.begin(), next.first.end());
        tcpclientPtr_->SendData(buf);
        printFrame("TX", frame(next.first.begin(), next.first.end()));
    }
    else
    {
        LOG_ERROR("ModbusTcpMediator::sendNextFrame  error........");
    }
}

void ModbusTcpMediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, modbustcpAnalysePtr_, msg, sendFrame_, nullptr));
}

void ModbusTcpMediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(ok, rw, result, count, type);
    }
}  