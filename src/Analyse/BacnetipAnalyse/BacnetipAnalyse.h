#pragma once

#include "Analyse/Analyse.h"
#include "myHelper/myHelper.h"

/**
 *      iot_templat.reagster_quantity : 数据解析类型 
 *      iot_templat.correct_mode      : propertyValue  (85 --present-value)
 **/



/*四、写属性
    810A
    0018    --长度
    01      --版本号
    04      --控制字
    00		--PDUtype
    03		--480字节
    06		--Invoke ID
    0F		--写服务
    0C00000000  0C=1100 -->100=4个长度  00000000 == AI instance=0
    19		--11001
    55		--present-value=85
    3E		--opening
    443F8E147B  --44-->应用标记  后跟4个长度的是数据  3F8E147B=1.1
    3F		--closing
    49		--应用标记
    0A		--优先级
*/

class BacnetipAnalyse : public Analyse
{
public:
    BacnetipAnalyse();
    ~BacnetipAnalyse();

    bool isOpening(const BacnetIP_Tag& tag);
    bool isClosing(const BacnetIP_Tag& tag);
    int AnalyseErrorClass(const frame& data, int& index);
    int AnalyseErrorCode(const frame& data, int& index);
    void prientErrorClass(int &errorClass);
    void prientErrorCode(int& errorCode);

    void AnalyseFunc(const std::string &msg, const nextFrame &nextframe, void* pending);
    void AnalyseNPDU_Control(const frame& data, int& index);
    void AnalyseAPDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);

    void BA_Confirmed_Request_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_Unconfirmed_Request_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_SimpleACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_ComplexACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_SegmentACK_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_Error_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_Reject_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void BA_Abort_PDU(const frame& data, int& index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);

    void AnalyseServiceChioce(const frame &data, int &index, int& serviceChoice, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);

    void Analyse_I_AM_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void AnalyseReadProperty_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void AnalyseReadPropertyMutiple_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    // void listPropertyResults(const frame &data, int &index);

    void AnalyseWriteProperty_Request(const frame &data, int &index, u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);
    void AnalyseWritePropertyMutiple_Request(const frame &data, int &index,u_int16_t len, const iot_device& device, const std::vector<iot_template>& v_templat);

    BacnetIP_Tag AnalyseTag(const frame&data, int& index);
    void AnalyseTagNumber(const frame&data, int& index, u_int8_t& src, u_int8_t& dest);
    void AnalyseTVL(const frame& data, int& index, u_int8_t tvl, BacnetIP_Tag& tag);

    BacnetIP_ObjectIdentifity AnalyseObject(const frame& data, int& index);
    void AnalyseListofResults(const frame &data, int &index, BacnetIP_ObjectIdentifity object, const iot_device& device, const std::vector<iot_template>& v_templat);

    int AnalysePropertyIdentifier(const frame& data, int& index, const BacnetIP_Tag& tag);
    void AnalysePropertyValue(const frame& data, int& index, BacnetIP_ObjectIdentifity object, int& prepertyValue, const iot_device& device, const std::vector<iot_template>& v_templat);
    std::string AnalyseValue(const frame& data, int& index, BacnetIP_Tag& applicationTag);

    bool findParam(const BacnetIP_ObjectIdentifity& object, const std::vector<iot_template>& v_templat, iot_template& res);

private:


};
