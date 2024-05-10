#include "Frame.h"

Frame::Frame(const iot_gateway& gatewayConf, int index, const std::string& name) 
        : gatewayConf_(gatewayConf) 
        , index_(index)
        // , opcindex_(index)
        , name_(name)
        // , mark_(0)
{
    
}

Frame::~Frame() 
{

}

bool Frame::cycleFinish()
{
    return index_ == 0;
}

void Frame::addControlFrame(const nextFrame& controlFrame)
{
    {
        std::unique_lock<std::mutex> lock(W_QueueMutex);
        W_Queue.push(controlFrame);
        LOG_INFO("%s control frame ++++++++++++", name_.c_str());
    }
}

bool Frame::getNextReadFrame(structNextFrame& strnextframe)
{
    // 取 写队列
    if(!W_Queue.empty())
    {
        {
            std::unique_lock<std::mutex> lock(W_QueueMutex);
            strnextframe.rw = enum_write;
            strnextframe.nextframe = W_Queue.front();
            W_Queue.pop();
        }
        return true;
    }

    // 取 读队列 
    if(0 == R_Vector.size())
    {
        LOG_FATAL("%sFrame::getNextReadFrame error : R_Vector.size == 0", name_.c_str());
        return false;
    }
    try
    {
        strnextframe.rw = enum_read;
        strnextframe.nextframe = R_Vector.at(index_);
        if(++index_ >= R_Vector.size())
        {
            index_ = 0;
        }
    }
    catch(std::out_of_range)
    {
        LOG_ERROR("%s Frame::getNextReadFrame  out_of_range...", name_.c_str());
        strnextframe.rw = enum_read;
        strnextframe.nextframe = R_Vector.at(index_);
        index_ = 0;
    }
    return true;
}

// bool Frame::getOPCNextReadFrame(opcnextFrame& opcnextframe)
// {
//     // 取 读队列 
//     if(0 == R_OPCVector.size())
//     {
//         LOG_FATAL("%sFrame::getOPCNextReadFrame error : R_OPCVector.size == 0", name_.c_str());
//         return false;
//     }
//     try
//     {
//         opcnextframe = R_OPCVector.at(opcindex_);
//         if(++opcindex_ >= R_OPCVector.size())
//         {
//             opcindex_ = 0;
//         }
//     }
//     catch(std::out_of_range)
//     {
//         LOG_ERROR("%sFrame::getOPCNextReadFrame  out_of_range...", name_.c_str());
//         opcnextframe = R_OPCVector.at(opcindex_);
//         opcindex_ = 0;
//     }
//     return true;
// }
