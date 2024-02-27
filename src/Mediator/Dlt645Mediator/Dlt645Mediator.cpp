#include "Dlt645Mediator.h"
#include "/usr/include/mymuduo/EventLoop.h"

Dlt645Mediator::Dlt645Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& dlt645Factory)
                : Mediator(loop, gateway, poolPtr)
                , dlt645AnalysePtr_(dlt645Factory->createAnalyse())
                , dlt645FramePtr_(dlt645Factory->createFrame(gateway))
                , serialPortPtr_(dlt645Factory->createNetSerial(enum_netserial_serialport, gateway))
{
    dlt645AnalysePtr_->setAnalyseFinishCallback(std::bind(&Dlt645Mediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    serialPortPtr_->setNextFrameCallback(std::bind(&Dlt645Mediator::onNextFrame, this));
    serialPortPtr_->setMessageCallback(std::bind(&Dlt645Mediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

Dlt645Mediator::~Dlt645Mediator()
{
    
}

void Dlt645Mediator::secTimer()
{
    
}

void Dlt645Mediator::start()
{
    dlt645FramePtr_->start();
    serialPortPtr_->start(DLT645_Freq);
}

void Dlt645Mediator::addControlFrame(const nextFrame& controlFrame)
{
    dlt645FramePtr_->addControlFrame(controlFrame);
}

void Dlt645Mediator::onNextFrame()
{
    nextFrame next;
    if(dlt645FramePtr_->getNextReadFrame(next))
    {
        sendedFrame_ = next;
        std::string buf(next.first.begin(), next.first.end());
        serialPortPtr_->SendData(buf);
        printFrame("TX", frame(next.first.begin(), next.first.end()));
    }
    else
    {
        LOG_ERROR("Dlt645Mediator::sendNextFrame  error........");
    }
}

//解析完成后
void Dlt645Mediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, dlt645AnalysePtr_, msg, sendedFrame_));
}

void Dlt645Mediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(ok, rw, result, count, type);
    }
}   
