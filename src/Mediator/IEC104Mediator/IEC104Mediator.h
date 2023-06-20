#pragma once 

#include "Mediator.h"
#include "Factory/IEC104Factory/IEC104Factory.h"
#include "myHelper/myHelper.h"

class IEC104Mediator : public Mediator
{
public:
    IEC104Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr, const FactoryPtr& iec104Factory);
    ~IEC104Mediator();

    void start();
    void addControlFrame(const nextFrame& controlFrame);
    void secTimer();

private:
    void HandleAnalyseFinishCallback(bool ok, enum_RW rw, AnalyseResult result, std::pair<int, IEC104FrameType> frameType);
    void updateFrame(frame &data, const u_int16_t &txsn, const u_int16_t &rxsn);

    void onNextFrame();
    void onMessage(const ConnectionPtr &conn, Buffer *buf, Timestamp time); //有新消息
    void onConnection();
    void onClose(const ConnectionPtr& conn);

    void HandleResult(AnalyseResult& result);
    void HandleFrameType(std::pair<int, IEC104FrameType>& frameType);

    NetSerialPtr tcpClientPtr_;
    AnalysePtr iec104AnalysePtr_;
    FramePtr iec104FramePtr_;

    const frame u_startFrame_;      // u帧启动帧
    const frame u_stopFrame_;       // u帧停止帧
    const frame u_stopRespFrame_;   // u帧停止确认帧
    const frame u_testFrame_;       // u帧测试帧
    const frame u_testRespFrame_;   // u帧测试确认帧
    const frame s_frame_;

    // bool BoolT0_;
    // int T0_;    // 30s内tcp连接超时

    std::atomic_bool BoolT1_;
    std::atomic_int T1_;        // 15s内发送方发送一个I格式报文或U格式报文后，必须在t1的时间内得到接收方的认可，
                                // 否则发送方认为TCP连接出现问题并应重新建立连接
    
    std::atomic_bool BoolT2_;
    std::atomic_int T2_;        // 10s 接收方在接收到I格式报文后，若经过t2时间未再收到新的I格式报文，则必须向发送方发送S格式帧对已经接收到的I格式报文进行认可
    
    std::atomic_bool BoolT3_;
    std::atomic_int T3_;        // 20s 每接收一帧I帧、S帧或者U帧将重新触发计时器t3，若在t3内未接收到任何报文，将向对方发送测试链路帧

    int K_; //未被确认的I格式APDU（k）最大数目；（默认值12 ）
    int W_; //接收方收到w个I格式APDU后确认, w 不应超过 k 的三分之二
};
