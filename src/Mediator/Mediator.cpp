#include "Mediator.h"

Mediator::Mediator(EventLoop* loop, const iot_gateway& gateway, const std::shared_ptr<ThreadPool>& poolPtr) 
            : loop_(loop), gateway_(gateway), poolPtr_(poolPtr)
{
    // LOG_INFO("Mediator::Mediator() ctor...");
}

Mediator::~Mediator()
{
    
}