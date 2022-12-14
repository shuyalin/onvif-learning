#include "soapStub.h"
#include "wsdd.nsmap"
#include "soapH.h"


//probe消息仿照 soap_wsdd_Probe 函数编写

int main(int argc, char *argv[])
{
    //soap环境变量
    struct soap *soap;

    //发送消息描述
    struct wsdd__ProbeType req;
    struct wsdd__ProbeType wsdd__Probe;

    struct __wsdd__ProbeMatches resp;

    //描述查找那类的Web消息
    struct wsdd__ScopesType sScope;

    //soap消息头消息
    struct SOAP_ENV__Header  header;

    //获得的设备信息个数
    int count = 0;

    //返回值
    int result = 0;

    //存放uuid 格式(8-4-4-4-12)
    char uuid_string[64];

    printf("%s: %d 000: \n", __FUNCTION__, __LINE__);
    sprintf(uuid_string, "464A4854-4656-5242-4530-110000000000");
    printf("uuid = %s \n", uuid_string);

    //soap初始化，申请空间
    soap = soap_new();
    if(soap == nullptr)
    {
        printf("malloc soap error \n");
        return -1;
    }


    soap_set_namespaces(soap, namespaces);  //设置命名空间，就是xml文件的头
    soap->recv_timeout = 5;  //超出5s没数据就推出，超时时间

    //将header设置为soap消息，头属性，暂且认为是soap和header绑定
    soap_default_SOAP_ENV__Header(soap, &header);
    header.wsa__MessageID = uuid_string;
    header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header.wsa__Action = "http://schemas.xmllocal_soap.org/ws/2005/04/discovery/Probe";
    //设置soap头消息的ID
    soap->header = &header;

    /* 设置所需寻找设备的类型和范围，二者至少设置一个
        否则可能收到非ONVIF设备，出现异常
     */

    //设置soap消息的请求服务属性
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = "onvif://www.onvif.org";
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;

    /* 设置所需设备的类型，ns1为命名空间前缀，在wsdd.nsmap 文件中
       {"tdn","http://www.onvif.org/ver10/network/wsdl"}的tdn,如果不是tdn,而是其它,
       例如ns1这里也要随之改为ns1
    */
    req.Types = "dn:NetworkVideoTransmitter";

    //调用gSoap接口 向 239.255.255.250:3702 发送udp消息
    result = soap_send___wsdd__Probe(soap,  "soap.udp://239.255.255.250:3702", nullptr, &req);

    if(result == -1)
    {
        printf("soap error: %d, %s, %s \n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        result = soap->error;
    }
    else
    {
        do{
            printf("%s: %d, begin receive probematch... \n", __FUNCTION__, __LINE__);
            printf("count = %d \n", count);
			memset(&resp, 0x00, sizeof(resp));
            //接收 ProbeMatches，成功返回0，错误返回-1
            result = soap_recv___wsdd__ProbeMatches(soap, &resp);
            printf(" --soap_recv___wsdd__ProbeMatches() result=%d \n",result);
            if(result == -1)
            {
                printf("Find %d devices!\n", count);
                break;
            }
            else
            {
                //读取服务器回应的Probematch消息
                printf("soap_recv___wsdd__Probe: __sizeProbeMatch = %d \n", resp.wsdd__ProbeMatches->__sizeProbeMatch);
                printf("Target EP Address : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
                printf("Target Type : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->Types);
                printf("Target Service Address : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                printf("Target Metadata Version: %d \n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
                printf("Target Scope Address : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
                count++;
            }
        }while(true);
    }
    //清除soap
    // clean up and remove deserialized data
    soap_end(soap);
    //detach and free runtime context
    soap_free(soap);

    return result;
}


