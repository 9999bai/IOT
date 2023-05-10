#pragma once

#include "myHelper/myHelper.h"
#include <strings.h>
#include <memory>

class Database
{
public:
    Database(sqlServer_info info, const std::string& Character="utf8")
            : info_(info)
            , Character_(Character)
        {
        }

    virtual ~Database() {}

    virtual void start() = 0;
    std::vector<iot_gateway> getSqlConfigData() { return gatewayConfList_; }
    
private:

    virtual void getGatewayConfigure() = 0;
    virtual void getDeviceConfigure(int gateway_id, std::vector<iot_device>& deviceConfList) = 0;
    virtual void getTemplateConfigure(int template_id, std::vector<iot_template>& templateConfList) = 0;

protected:
    const sqlServer_info info_;
    const std::string Character_;
    std::vector<iot_gateway> gatewayConfList_;
};