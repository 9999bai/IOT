#include "Pro188Mediator.h"

Pro188Mediator::Pro188Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& pro188Factory)
                : Mediator(loop, gateway, poolPtr)
                , pro188FramePtr_(pro188Factory->createFrame(gateway))
                , pro188AnalysePtr_(pro188Factory->createAnalyse())
                , serialPortPtr_(pro188Factory->createNetSerial(enum_netserial_serialport, gateway))
{
    pro188AnalysePtr_->setAnalyseFinishCallback(std::bind(&Pro188Mediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    serialPortPtr_->setNextFrameCallback(std::bind(&Pro188Mediator::onNextFrame, this));
    serialPortPtr_->setMessageCallback(std::bind(&Pro188Mediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

Pro188Mediator::~Pro188Mediator()
{
    
}

void Pro188Mediator::secTimer()
{

}

void Pro188Mediator::start()
{
    pro188FramePtr_->start();   // 组合数据帧
    serialPortPtr_->start(CJT188_Freq);    // 打开串口
}

void Pro188Mediator::addControlFrame(const nextFrame& controlFrame)
{
    pro188FramePtr_->addControlFrame(controlFrame);
}

//serial 接收串口数据-->解析
void Pro188Mediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, pro188AnalysePtr_, msg, sendedFrame_));
}

void Pro188Mediator::onNextFrame()
{
    nextFrame next;
    if(pro188FramePtr_->getNextReadFrame(next))
    {
        sendedFrame_ = next;
        std::string buf(next.first.begin(), next.first.end());
        serialPortPtr_->SendData(buf);
        printFrame("TX", frame(next.first.begin(), next.first.end()));
    }
    else
    {
        LOG_ERROR("Pro188Mediator::sendNextFrame  error........");
    }
}

void Pro188Mediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, int count, IEC104FrameType type)
{
    if(rw == enum_write)
    {
        if(ok)
        {
            // 写 成功
            LOG_INFO("Pro188Mediator 写成功...");
        }
        else
        {
            // 写 失败
            LOG_INFO("Pro188Mediator 写失败...");
        }
    }
    else if(rw == enum_read)
    {
        if(ok)
        {
            // 读 成功
            LOG_INFO("Pro188Mediator 读成功...");
        }
        else
        {
            // 读 失败
            LOG_INFO("Pro188Mediator 读失败...");
        }
    }
}
