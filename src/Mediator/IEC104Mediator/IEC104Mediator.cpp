#include "IEC104Mediator.h"


IEC104Mediator::IEC104Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& iec104Factory)
                : Mediator(loop, gateway, poolPtr)
                , T1_(0), T2_(0), T3_(0)
                , W_(6)
                , BoolT1_(true), BoolT2_(true), BoolT3_(true) 
                , iec104AnalysePtr_(iec104Factory->createAnalyse())
                , tcpClientPtr_(iec104Factory->createNetSerial(enum_netserial_tcp, gateway))
                , iec104FramePtr_(iec104Factory->createFrame(gateway))
                , u_startFrame_(HexStrToByteArray("68 04 07 00 00 00"))
                , u_stopFrame_(HexStrToByteArray("68 04 13 00 00 00"))
                , u_stopRespFrame_(HexStrToByteArray("68 04 23 00 00 00"))
                , u_testFrame_(HexStrToByteArray("68 04 43 00 00 00"))
                , u_testRespFrame_(HexStrToByteArray("68 04 83 00 00 00"))
                , s_frame_(HexStrToByteArray("68 04 01 00"))
{
    iec104AnalysePtr_->setAnalyseFinishCallback(std::bind(&IEC104Mediator::HandleAnalyseFinishCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    tcpClientPtr_->setNextFrameCallback(std::bind(&IEC104Mediator::onNextFrame, this));
    tcpClientPtr_->setMessageCallback(std::bind(&IEC104Mediator::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    tcpClientPtr_->setNewConnectionCallback(std::bind(&IEC104Mediator::onConnection, this));
    tcpClientPtr_->setCloseCallback(std::bind(&IEC104Mediator::onClose, this, std::placeholders::_1));
}

IEC104Mediator::~IEC104Mediator()
{

}

void IEC104Mediator::secTimer()
{
    // if (!BoolT0_ && (++T0_ >= 30))
    // {
    //     T0_ = 0;
    //     // tcp连接超时， 重启tcp
    // }

    if(!BoolT1_ && (++T1_ >= 15))
    {
        // T1时间内没接收到 I/U 帧回复， 重启tcp
        BoolT1_ = true;
        T1_ = 0;
        LOG_INFO("T1 timerout-----reboot--------");
        tcpClientPtr_->restart();
    }

    if(!BoolT2_ && (++T2_ >= 10))
    {
        // 发送s确认帧
        LOG_INFO("T2 timerout-----s frame--------");
        BoolT2_ = true;
        T2_ = 0;
        union_uint16TOuchar data;
        data.u16_data = iec104AnalysePtr_->getRX_SN();
        std::string buf(s_frame_.begin(), s_frame_.end());
        buf.push_back(data.uchar_data[0]);
        buf.push_back(data.uchar_data[1]);

        iot_device device;
        iot_template templat;
        templat.rw = enum_read;
        sendedFrame_ = nextFrame(frame(buf.begin(), buf.end()), pair_frame(device, templat));

        printFrame("TX", frame(buf.begin(), buf.end()));
        tcpClientPtr_->SendData(buf);
    }

    if(!BoolT3_ && (++T3_ >= 20))
    {
        LOG_INFO("T3 timerout-----u_test frame--------");
        T3_ = 0;

        BoolT1_ = false;  // T1 开启
        // T1_ = 0;

        // U帧测试链路帧

        iot_device device;
        iot_template templat;
        templat.rw = enum_read;
        sendedFrame_ = nextFrame(u_testFrame_, pair_frame(device, templat));

        std::string buf(u_testFrame_.begin(), u_testFrame_.end());
        printFrame("TX", u_testFrame_);
        tcpClientPtr_->SendData(buf);
    }
}

void IEC104Mediator::start()
{
    iec104FramePtr_->start();
    tcpClientPtr_->start(IEC104Next_Freq);
}

void IEC104Mediator::addControlFrame(const nextFrame& controlFrame)
{
    iec104FramePtr_->addControlFrame(controlFrame);
}

void IEC104Mediator::HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType)     //解析完成后
{
    if(rw == enum_write)
    {
        if(ok)
        {
            // 写 成功
            LOG_INFO("IEC104Mediator 写成功...");
        }
        else
        {
            // 写 失败
            LOG_INFO("IEC104Mediator 写失败...");
        }
    }
    else if(rw == enum_read)
    {
        if(ok)
        {
            // 读 成功
            LOG_INFO("IEC104Mediator 读成功...");
            HandleResult(result);
            HandleFrameType(frameType);
        }
        else
        {
            // 读 失败
            LOG_INFO("IEC104Mediator 读失败...");
        }
    }
}

void IEC104Mediator::onNextFrame()
{
    // nextFrame next;
    // if(iec104FramePtr_->getNextReadFrame(next))
    // {
    //     sendedFrame_ = next;
    //     std::string buf(next.first.begin(), next.first.end());
    //     tcpClientPtr_->SendData(buf);
    //     printFrame("TX", frame(next.first.begin(), next.first.end()));
    // }
    // else
    // {
    //     LOG_ERROR("ModbusRtuMediator::sendNextFrame  error........");
    // }
}

void IEC104Mediator::onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time) //有新消息
{
    std::string msg = buf->retrieveAllAsString();
    printFrame("RX", frame(msg.begin(), msg.end()));
    poolPtr_->run(std::bind(&Analyse::AnalyseFunc, iec104AnalysePtr_, msg, sendedFrame_));
}

void IEC104Mediator::onConnection()
{
    BoolT1_ = false; // T1 开启
    T1_ = 0;

    iot_device device;
    iot_template templat;
    templat.rw = enum_read;
    sendedFrame_ = nextFrame(u_startFrame_, pair_frame(device, templat));
    
    printFrame("TX", u_startFrame_);
    tcpClientPtr_->SendData(std::string(u_startFrame_.begin(), u_startFrame_.end())); // 发送U帧启动帧
}

void IEC104Mediator::onClose(const ConnectionPtr& conn)
{

}

void IEC104Mediator::HandleResult(AnalyseResult& result)
{
    switch(result)
    {
        case ENUM_Normal:
            break;
        case ENUM_RebootSocket:
        {
            tcpClientPtr_->restart();
            break;
        }
        case ENUM_Send_S_Frame:
            break;
        // case ENUM_Send_U_testFrame:
        // {
        //     std::string buf(u_testFrame_.begin(), u_testFrame_.end());

        //     iot_device device;
        //     iot_template templat;
        //     templat.rw = enum_read;
        //     sendedFrame_ = nextFrame(u_testFrame_, pair_frame(device, templat));

        //     printFrame("TX", u_testFrame_);
        //     tcpClientPtr_->SendData(buf);
        //     break;
        // }
        case ENUM_Send_U_testRespFrame:
        {
            std::string buf(u_testRespFrame_.begin(), u_testRespFrame_.end());
            
            iot_device device;
            iot_template templat;
            templat.rw = enum_read;
            sendedFrame_ = nextFrame(u_testRespFrame_, pair_frame(device, templat));

            printFrame("TX", u_testRespFrame_);
            tcpClientPtr_->SendData(buf);
            break;
        }
        case ENUM_SendFirst_I_Frame:
        {
            nextFrame next;
            if(iec104FramePtr_->getNextReadFrame(next))
            {
                sendedFrame_ = next;
                updateFrame(next.first, iec104AnalysePtr_->getTX_SN(), iec104AnalysePtr_->getRX_SN());
                std::string buf(next.first.begin(), next.first.end());

                BoolT1_ = false; // T1 开启
                // T1_ = 0;

                printFrame("TX", frame(buf.begin(), buf.end()));
                tcpClientPtr_->SendData(buf);
                iec104AnalysePtr_->IncreaseTX();
            }
            else
            {
                LOG_ERROR("second getNextReadFrame error...");
            }
            break;
        }
        case ENUM_SendNext_I_Frame:
        {
            if(iec104FramePtr_->cycleFinish())
            {
                LOG_INFO("没有后续I帧....");
                // 没有后续 YM
            }
            else
            {
                nextFrame next;
                if(iec104FramePtr_->getNextReadFrame(next))
                {
                    sendedFrame_ = next;
                    updateFrame(next.first, iec104AnalysePtr_->getTX_SN(), iec104AnalysePtr_->getRX_SN());
                    std::string buf(next.first.begin(), next.first.end());
                    
                    BoolT1_ = false; // T1 开启
                    // T1_ = 0;

                    printFrame("TX", frame(buf.begin(), buf.end()));
                    tcpClientPtr_->SendData(buf);
                    iec104AnalysePtr_->IncreaseTX();
                }else{
                    LOG_ERROR("first getNextReadFrame error...");
                }
            }
            break;
        }
    }
}

void IEC104Mediator::HandleFrameType(std::pair<int, IEC104FrameType>& frameType)
{
    static int IFrame_count = 0;
    switch (frameType.second)
    {
        case ENUM_Normal_Frame:
            break;
        case ENUM_U_Frame:
        {
            BoolT1_ = true; // T1 关闭
            T1_ = 0;

            BoolT3_ = false; // T3 开启
            T3_ = 0;
            break;
        }
        case ENUM_I_Frame: // 接收到I帧
        {
            BoolT1_ = true; // T1 关闭
            T1_ = 0;

            BoolT2_ = false; // T2 开启
            T2_ = 0;

            BoolT3_ = false; // T3 开启
            T3_ = 0;

            IFrame_count += frameType.first;
            if(IFrame_count >= W_)
            {
                IFrame_count = 0;
                union_uint16TOuchar data;
                data.u16_data = iec104AnalysePtr_->getRX_SN();
                std::string buf(s_frame_.begin(), s_frame_.end());
                buf.push_back(data.uchar_data[0]);
                buf.push_back(data.uchar_data[1]);

                iot_device device;
                iot_template templat;
                templat.rw = enum_read;
                sendedFrame_ = nextFrame(frame(buf.begin(), buf.end()), pair_frame(device, templat));

                printFrame("TX", frame(buf.begin(), buf.end()));
                tcpClientPtr_->SendData(buf);
            }
            break;
        }
        case ENUM_S_Frame:
        {
            BoolT3_ = false; // T3 开启
            T3_ = 0;
            break;
        }
    }
}

void IEC104Mediator::updateFrame(frame& data, const u_int16_t& txsn, const u_int16_t& rxsn)
{
    if(data.size() < 6)
    {
        LOG_ERROR("data.size < 6...");
        return;
    }
    union_uint16TOuchar txtmp;
    txtmp.u16_data = txsn << 1;

    data[2] = txtmp.uchar_data[0];
    data[3] = txtmp.uchar_data[1];

    union_uint16TOuchar rxtmp;
    rxtmp.u16_data = rxsn << 1;

    data[4] = rxtmp.uchar_data[0];
    data[5] = rxtmp.uchar_data[1];
}
