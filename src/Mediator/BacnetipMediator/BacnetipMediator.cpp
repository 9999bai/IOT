#include "BacnetipMediator.h"


BacnetipMediator::BacnetipMediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& bacnetipFactory)
                : Mediator(loop, gateway, poolPtr)
                , bacnetipAnalysePtr_(bacnetipFactory->createAnalyse())
                , udpClientPtr_(bacnetipFactory->createNetSerial(enum_netserial_udp, gateway))
                , bacnetipFramePtr_(bacnetipFactory->createFrame(gateway))
{
    bacnetipAnalysePtr_->setAnalyseFinishCallback(std::bind(&BacnetipMediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    udpClientPtr_->setNextFrameCallback(std::bind(&BacnetipMediator::onNextFrame, this));
    udpClientPtr_->setMessageCallback(std::bind(&BacnetipMediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

BacnetipMediator::~BacnetipMediator()
{

}

void BacnetipMediator::start()
{
    bacnetipFramePtr_->start();
    udpClientPtr_->start(BACNETIP_Freq);
}

void BacnetipMediator::addControlFrame(const nextFrame& controlFrame)
{
    bacnetipFramePtr_->addControlFrame(controlFrame);
}

void BacnetipMediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(analyseFinishCallback_)
    {
        analyseFinishCallback_(ok, rw, result, count, type);
    }
}

void BacnetipMediator::onNextFrame()
{
    nextFrame next;
    if(bacnetipFramePtr_->getNextReadFrame(next))
    {
        sendFrame_ = next;
        std::string buf(next.first.begin(), next.first.end());
        udpClientPtr_->SendData(buf);
        printFrame("TX", frame(next.first.begin(), next.first.end()));
    }
    else
    {
        LOG_ERROR("BacnetipMediator::sendNextFrame  error........");
    }
}

void BacnetipMediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, bacnetipAnalysePtr_, msg, sendFrame_, nullptr));
}
