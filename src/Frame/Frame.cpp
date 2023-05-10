#include "Frame.h"

Frame::Frame(const iot_gateway& gatewayConf, int index, const std::string& name) 
        : gatewayConf_(gatewayConf) 
        , index_(index)
        , name_(name)
{
    
}

Frame::~Frame() 
{

}

void Frame::addControlFrame(const nextFrame& controlFrame)
{
    {
        std::unique_lock<std::mutex> lock(W_QueueMutex);
        W_Queue.push(controlFrame);
        LOG_INFO("%s control frame ++++++++++++", name_.c_str());
    }
}

bool Frame::getNextReadFrame(nextFrame& next_frame)
{
    if(!W_Queue.empty())
    {
        {
            std::unique_lock<std::mutex> lock(W_QueueMutex);
            next_frame = W_Queue.front();
            W_Queue.pop();
        }
        return true;
    }

    if(0 == R_Vector.size())
    {
        LOG_FATAL("%sFrame::getNextReadFrame error : R_Vector.size == 0", name_.c_str());
        return false;
    }
    try
    {
        next_frame = R_Vector.at(index_);
        if(++index_ >= R_Vector.size())
        {
            index_ = 0;
        }
    }
    catch(std::out_of_range)
    {
        LOG_ERROR("%sFrame::getNextReadFrame  out_of_range...", name_.c_str());
        next_frame = R_Vector.at(index_);
        index_ = 0;
    }
    return true;
}