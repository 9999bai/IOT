#include "ModbusTcpMediator.h"
#include "/usr/include/mymuduo/Buffer.h"

ModbusTcpMediator::ModbusTcpMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& modbusTcpFactory)
                : Mediator(loop, gateway, poolPtr)
                , modbustcpAnalysePtr_(modbusTcpFactory->createAnalyse())
                , tcpclientPtr_(modbusTcpFactory->createNetSerial(enum_netserial_tcp, gateway))
                , modbustcpFramePtr_(modbusTcpFactory->createFrame(gateway))
{
    modbustcpAnalysePtr_->setAnalyseFinishCallback(std::bind(&ModbusTcpMediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
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
        sendedFrame_ = next;
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
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, modbustcpAnalysePtr_, msg, sendedFrame_));
}

void ModbusTcpMediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType)
{
    if(rw == enum_write)
    {
        if(ok)
        {
            // 写 成功
            LOG_INFO("ModbusTcpMediator 写成功...");
        }
        else
        {
            // 写 失败
            LOG_INFO("ModbusTcpMediator 写失败...");
        }
    }
    else if(rw == enum_read)
    {
        if(ok)
        {
            // 读 成功
            LOG_INFO("ModbusTcpMediator 读成功...");
        }
        else
        {
            // 读 失败
            LOG_INFO("ModbusTcpMediator 读失败...");
        }
    }
}  