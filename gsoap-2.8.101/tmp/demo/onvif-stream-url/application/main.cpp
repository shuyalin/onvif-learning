#include <assert.h>
#include "soapH.h"
#include "wsdd.nsmap"
#include "soapStub.h"
#include "wsseapi.h"
#include "wsaapi.h"
#include <map>




#define SOAP_ASSERT     assert
#define SOAP_DBGLOG     printf
#define SOAP_DBGERR     printf

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (10)               // socket超时时间（单秒秒）




void soap_perror(struct soap *soap, const char *str)
{
    if (nullptr == str) {
        SOAP_DBGERR("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    } else {
        SOAP_DBGERR("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    }
}

void* ONVIF_soap_malloc(struct soap *soap, unsigned int n)
{
    void *p = nullptr;

    if (n > 0) {
        p = soap_malloc(soap, n);
                SOAP_ASSERT(nullptr != p);
        memset(p, 0x00 ,n);
    }
    return p;
}

struct soap *ONVIF_soap_new(int timeout)
{
    struct soap *soap = nullptr;                                                   // soap环境变量

            SOAP_ASSERT(nullptr != (soap = soap_new()));

    soap_set_namespaces(soap, namespaces);                                      // 设置soap的namespaces
    soap->recv_timeout    = timeout;                                            // 设置超时（超过指定时间没有数据就退出）
    soap->send_timeout    = timeout;
    soap->connect_timeout = timeout;

#if defined(__linux__) || defined(__linux)                                      // 参考https://www.genivia.com/dev.html#client-c的修改：
    soap->socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
#endif

    soap_set_mode(soap, SOAP_C_UTFSTRING);                                      // 设置为UTF-8编码，否则叠加中文OSD会乱码

    return soap;
}

void ONVIF_soap_delete(struct soap *soap)
{
    soap_destroy(soap);                                                         // remove deserialized class instances (C++ only)
    soap_end(soap);                                                             // Clean up deserialized data (except class instances) and temporary data
    soap_done(soap);                                                            // Reset, close communications, and remove callbacks
    soap_free(soap);                                                            // Reset and deallocate the context created with soap_new or soap_copy
}

/************************************************************************
**函数：ONVIF_init_header
**功能：初始化soap描述消息头
**参数：
        [in] soap - soap环境变量
**返回：无
**备注：
    1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
************************************************************************/
void ONVIF_init_header(struct soap *soap)
{
    struct SOAP_ENV__Header *header = nullptr;

            SOAP_ASSERT(nullptr != soap);

    header = (struct SOAP_ENV__Header *)ONVIF_soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
    soap_default_SOAP_ENV__Header(soap, header);
    header->wsa__MessageID =  (char*)soap_wsa_rand_uuid(soap);
    header->wsa__To        = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TO) + 1);
    header->wsa__Action    = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ACTION) + 1);
    strcpy(header->wsa__To, SOAP_TO);
    strcpy(header->wsa__Action, SOAP_ACTION);
    soap->header = header;
}

/************************************************************************
**函数：ONVIF_init_ProbeType
**功能：初始化探测设备的范围和类型
**参数：
        [in]  soap  - soap环境变量
        [out] probe - 填充要探测的设备范围和类型
**返回：
        0表明探测到，非0表明未探测到
**备注：
    1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
************************************************************************/
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe)
{
    struct wsdd__ScopesType *scope = nullptr;                                      // 用于描述查找哪类的Web服务

            SOAP_ASSERT(nullptr != soap);
            SOAP_ASSERT(nullptr != probe);

    scope = (struct wsdd__ScopesType *)ONVIF_soap_malloc(soap, sizeof(struct wsdd__ScopesType));
    soap_default_wsdd__ScopesType(soap, scope);                                 // 设置寻找设备的范围
    scope->__item = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ITEM) + 1);
    strcpy(scope->__item, SOAP_ITEM);

    memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
    soap_default_wsdd__ProbeType(soap, probe);
    probe->Scopes = scope;
    probe->Types  = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TYPES) + 1);     // 设置寻找设备的类型
    strcpy(probe->Types, SOAP_TYPES);
}

void ONVIF_DetectDevice(void (*cb)(char *DeviceXAddr))
{
    int i;
    int result = 0;
    unsigned int count = 0;                                                     // 搜索到的设备个数
    struct soap *soap = nullptr;                                                   // soap环境变量
    struct wsdd__ProbeType      req;                                            // 用于发送Probe消息
    struct __wsdd__ProbeMatches rep;                                            // 用于接收Probe应答
    struct wsdd__ProbeMatchType *probeMatch;

            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_init_header(soap);                                                    // 设置消息头描述
    ONVIF_init_ProbeType(soap, &req);                                           // 设置寻找的设备的范围和类型
    result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, nullptr, &req);        // 向组播地址广播Probe消息
    while (SOAP_OK == result)                                                   // 开始循环接收设备发送过来的消息
    {
        memset(&rep, 0x00, sizeof(rep));
        result = soap_recv___wsdd__ProbeMatches(soap, &rep);
        if (SOAP_OK == result) {
            if (soap->error) {
                soap_perror(soap, "ProbeMatches");
            } else {                                                            // 成功接收到设备的应答消息
                //  printf("__sizeProbeMatch:%d\n",rep.wsdd__ProbeMatches->__sizeProbeMatch);

                if (nullptr != rep.wsdd__ProbeMatches) {
                    count += rep.wsdd__ProbeMatches->__sizeProbeMatch;
                    for(i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                        probeMatch = rep.wsdd__ProbeMatches->ProbeMatch + i;
                        if (nullptr != cb ) {
                            std::string url = probeMatch->XAddrs;
                            if(url == "http://192.168.1.8/onvif/device_service"){
                                cb(probeMatch->XAddrs);                             // 使用设备服务地址执行函数回调
                            }
                        }
                    }
                }
            }
        } else if (soap->error) {
            break;
        }
    }

    SOAP_DBGLOG("\ndetect end! It has detected %d devices!\n", count);

    if (nullptr != soap) {
        ONVIF_soap_delete(soap);
    }

}


#define SOAP_CHECK_ERROR(result, soap, str) \
    do { \
        if (SOAP_OK != (result) || SOAP_OK != (soap)->error) { \
            soap_perror((soap), (str)); \
            if (SOAP_OK == (result)) { \
                (result) = (soap)->error; \
            } \
            goto EXIT; \
        } \
} while (0)



/************************************************************************
**函数：ONVIF_SetAuthInfo
**功能：设置认证信息
**参数：
        [in] soap     - soap环境变量
        [in] username - 用户名
        [in] password - 密码
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
static int ONVIF_SetAuthInfo(struct soap *soap, const char *username, const char *password)
{
    int result = 0;

            SOAP_ASSERT(nullptr != username);
            SOAP_ASSERT(nullptr != password);

    result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
    SOAP_CHECK_ERROR(result, soap, "add_UsernameTokenDigest");

    EXIT:

    return result;
}

#define USERNAME    "admin"
#define PASSWORD    "qwe815454"
/************************************************************************
**函数：ONVIF_GetDeviceInformation
**功能：获取设备基本信息
**参数：
        [in] DeviceXAddr - 设备服务地址
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
int ONVIF_GetDeviceInformation(const char *DeviceXAddr)
{
    int result = 0;
    struct soap *soap = nullptr;
    _tds__GetDeviceInformation           devinfo_req;
    _tds__GetDeviceInformationResponse   devinfo_resp;

            SOAP_ASSERT(nullptr != DeviceXAddr);
            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));



    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);

    result = soap_call___tds__GetDeviceInformation(soap, DeviceXAddr, nullptr, &devinfo_req, devinfo_resp);
    SOAP_CHECK_ERROR(result, soap, "GetDeviceInformation");
    std::cout << "      Manufacturer:\t" << devinfo_resp.Manufacturer << "\n";
    std::cout << "      Model:\t" << devinfo_resp.Model << "\n";
    std::cout << "      FirmwareVersion:\t" << devinfo_resp.FirmwareVersion << "\n";
    std::cout << "      SerialNumber:\t" << devinfo_resp.SerialNumber << "\n";
    std::cout << "      HardwareId:\t" << devinfo_resp.HardwareId << "\n";
	std::cout << "555555555555555555555555555555555 " << "\n";


    EXIT:

    if (nullptr != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}



/************************************************************************
**函数：ONVIF_GetSnapshotUri
**功能：获取设备图像抓拍地址(HTTP)
**参数：
        [in]  MediaXAddr    - 媒体服务地址
        [in]  ProfileToken  - the media profile token
        [out] uri           - 返回的地址
        [in]  sizeuri       - 地址缓存大小
**返回：
        0表明成功，非0表明失败
**备注：
    1). 并非所有的ProfileToken都支持图像抓拍地址。举例：XXX品牌的IPC有如下三个配置profile0/profile1/TestMediaProfile，其中TestMediaProfile返回的图像抓拍地址就是空指针。
************************************************************************/
int ONVIF_GetSnapshotUri(const std::string& MediaXAddr, const std::string& ProfileToken, std::string * snapUri)
{
    int result = 0;
    struct soap *soap = nullptr;
    _trt__GetSnapshotUri         req;
    _trt__GetSnapshotUriResponse rep;

            SOAP_ASSERT(!MediaXAddr.empty() && !ProfileToken.empty());
            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);
    req.ProfileToken = const_cast<char *>(ProfileToken.c_str());
    result = soap_call___trt__GetSnapshotUri(soap, MediaXAddr.c_str(), NULL, &req, rep);
    SOAP_CHECK_ERROR(result, soap, "GetSnapshotUri");


    if (nullptr != rep.MediaUri && nullptr != rep.MediaUri->Uri) {
        *snapUri = rep.MediaUri->Uri;
    }

    EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}


bool ONVIF_GetProfiles(const std::string& mediaXAddr, std::string  * profilesToken)
{
    int result = 0;
    struct soap *soap = nullptr;
    _trt__GetProfiles           devinfo_req;
    _trt__GetProfilesResponse   devinfo_resp;

            SOAP_ASSERT(!mediaXAddr.empty());
            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);
    result = soap_call___trt__GetProfiles(soap, mediaXAddr.c_str(), nullptr, &devinfo_req, devinfo_resp);
    SOAP_CHECK_ERROR(result, soap, "ONVIF_GetProfiles");

            SOAP_ASSERT(devinfo_resp.__sizeProfiles > 0);

    *profilesToken = (*devinfo_resp.Profiles)->token;

    EXIT:
    if (nullptr != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}

/************************************************************************
**函数：ONVIF_GetCapabilities
**功能：获取设备能力信息
**参数：
        [in] DeviceXAddr - 设备服务地址
        [in]
**返回：
        0表明成功，非0表明失败
**备注：
    1). 其中最主要的参数之一是媒体服务地址
************************************************************************/
int ONVIF_GetCapabilities(const std::string& deviceXAddr, std::string * mediaXAddr)
{
    int result = 0;
    struct soap *soap = nullptr;
    _tds__GetCapabilities            devinfo_req;
    _tds__GetCapabilitiesResponse    devinfo_resp;


            SOAP_ASSERT(!deviceXAddr.empty());
            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    result = soap_call___tds__GetCapabilities(soap, deviceXAddr.c_str(), NULL, &devinfo_req, devinfo_resp);
    SOAP_CHECK_ERROR(result, soap, "GetCapabilities");

    if(devinfo_resp.Capabilities->Media != nullptr){
        *mediaXAddr = devinfo_resp.Capabilities->Media->XAddr;
    }

    EXIT:

    if (nullptr != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}


/************************************************************************
**函数：make_uri_withauth
**功能：构造带有认证信息的URI地址
**参数：
        [in]  src_uri       - 未带认证信息的URI地址
        [in]  username      - 用户名
        [in]  password      - 密码
        [out] dest_uri      - 返回的带认证信息的URI地址
        [in]  size_dest_uri - dest_uri缓存大小
**返回：
        0成功，非0失败
**备注：
    1). 例子：
    无认证信息的uri：rtsp://100.100.100.140:554/av0_0
    带认证信息的uri：rtsp://username:password@100.100.100.140:554/av0_0
************************************************************************/
static int make_uri_withauth(const std::string& src_uri, const std::string&username, const std::string&password, std::string *dest_uri)
{
    int result = 0;


            SOAP_ASSERT(!src_uri.empty());

    if (username.empty() &&password.empty()) {                       // 生成新的uri地址
        *dest_uri = src_uri;
    } else {
        std::string::size_type position = src_uri.find("//");
        if (std::string::npos == position) {
            SOAP_DBGERR("can't found '//', src uri is: %s.\n", src_uri.c_str());
            result = -1;
            return result;
        }

        position += 2;
        dest_uri->append(src_uri,0,   position) ;
        dest_uri->append(username + ":" + password + "@");
        dest_uri->append(src_uri,position, std::string::npos) ;
    }


    return result;
}


/************************************************************************
**函数：ONVIF_GetStreamUri
**功能：获取设备码流地址(RTSP)
**参数：
       [in]  MediaXAddr    - 媒体服务地址
       [in]  ProfileToken  - the media profile token
**返回：
       0表明成功，非0表明失败
**备注：
************************************************************************/
int ONVIF_GetStreamUri(const std::string&MediaXAddr, const std::string&ProfileToken)
{
    int result = 0;
    struct soap *soap = nullptr;
    tt__StreamSetup              ttStreamSetup;
    tt__Transport                ttTransport;
    _trt__GetStreamUri           req;
    _trt__GetStreamUriResponse   rep;

     SOAP_ASSERT(!MediaXAddr.empty());
     SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));


    ttStreamSetup.Stream                = tt__StreamType__RTP_Unicast;
    ttStreamSetup.Transport             = &ttTransport;
    ttStreamSetup.Transport->Protocol   = tt__TransportProtocol__RTSP;
    ttStreamSetup.Transport->Tunnel     = nullptr;
    req.StreamSetup                     = &ttStreamSetup;
    req.ProfileToken                    = const_cast<char *>(ProfileToken.c_str());

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);
    result = soap_call___trt__GetStreamUri(soap, MediaXAddr.c_str(), nullptr, &req, rep);
    SOAP_CHECK_ERROR(result, soap, "GetServices");



    if (nullptr != rep.MediaUri) {
        if (nullptr != rep.MediaUri->Uri) {
            std::cout << rep.MediaUri->Uri << "\n";
        }
    }

    EXIT:

    if (nullptr != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}
void cb_discovery(char *deviceXAddr)
{
    std::string  mediaXAddr, profilesToken, snapUri, snapAuthUri;
    ONVIF_GetCapabilities(deviceXAddr, &mediaXAddr);
	std::cout << "11111111111111111111111111 " << "\n";
    ONVIF_GetProfiles(mediaXAddr, &profilesToken);
	std::cout << "2222222222222222222222222 " << "\n";
    ONVIF_GetStreamUri(mediaXAddr, profilesToken);
	std::cout << "3333333333333333333333333 " << "\n";
/*    ONVIF_GetSnapshotUri(mediaXAddr, profilesToken, &snapUri);
    make_uri_withauth(snapUri, USERNAME, PASSWORD, &snapAuthUri);

    char cmd[256];
    sprintf(cmd, "wget -O %s '%s'",   "out.jpeg", snapAuthUri.c_str());                        // 使用wget下载图片
    system(cmd);*/
}

int main(int argc, char **argv)
{
    ONVIF_DetectDevice(cb_discovery);
    return 0;
}

