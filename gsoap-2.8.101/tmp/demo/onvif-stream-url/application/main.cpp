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

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif�涨���鲥��ַ

#define SOAP_ITEM       ""                                                      // Ѱ�ҵ��豸��Χ
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // Ѱ�ҵ��豸����

#define SOAP_SOCK_TIMEOUT    (10)               // socket��ʱʱ�䣨�����룩




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
    struct soap *soap = nullptr;                                                   // soap��������

            SOAP_ASSERT(nullptr != (soap = soap_new()));

    soap_set_namespaces(soap, namespaces);                                      // ����soap��namespaces
    soap->recv_timeout    = timeout;                                            // ���ó�ʱ������ָ��ʱ��û�����ݾ��˳���
    soap->send_timeout    = timeout;
    soap->connect_timeout = timeout;

#if defined(__linux__) || defined(__linux)                                      // �ο�https://www.genivia.com/dev.html#client-c���޸ģ�
    soap->socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
#endif

    soap_set_mode(soap, SOAP_C_UTFSTRING);                                      // ����ΪUTF-8���룬�����������OSD������

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
**������ONVIF_init_header
**���ܣ���ʼ��soap������Ϣͷ
**������
        [in] soap - soap��������
**���أ���
**��ע��
    1). �ڱ������ڲ�ͨ��ONVIF_soap_malloc������ڴ棬����ONVIF_soap_delete�б��ͷ�
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
**������ONVIF_init_ProbeType
**���ܣ���ʼ��̽���豸�ķ�Χ������
**������
        [in]  soap  - soap��������
        [out] probe - ���Ҫ̽����豸��Χ������
**���أ�
        0����̽�⵽����0����δ̽�⵽
**��ע��
    1). �ڱ������ڲ�ͨ��ONVIF_soap_malloc������ڴ棬����ONVIF_soap_delete�б��ͷ�
************************************************************************/
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe)
{
    struct wsdd__ScopesType *scope = nullptr;                                      // �����������������Web����

            SOAP_ASSERT(nullptr != soap);
            SOAP_ASSERT(nullptr != probe);

    scope = (struct wsdd__ScopesType *)ONVIF_soap_malloc(soap, sizeof(struct wsdd__ScopesType));
    soap_default_wsdd__ScopesType(soap, scope);                                 // ����Ѱ���豸�ķ�Χ
    scope->__item = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ITEM) + 1);
    strcpy(scope->__item, SOAP_ITEM);

    memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
    soap_default_wsdd__ProbeType(soap, probe);
    probe->Scopes = scope;
    probe->Types  = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TYPES) + 1);     // ����Ѱ���豸������
    strcpy(probe->Types, SOAP_TYPES);
}

void ONVIF_DetectDevice(void (*cb)(char *DeviceXAddr))
{
    int i;
    int result = 0;
    unsigned int count = 0;                                                     // ���������豸����
    struct soap *soap = nullptr;                                                   // soap��������
    struct wsdd__ProbeType      req;                                            // ���ڷ���Probe��Ϣ
    struct __wsdd__ProbeMatches rep;                                            // ���ڽ���ProbeӦ��
    struct wsdd__ProbeMatchType *probeMatch;

            SOAP_ASSERT(nullptr != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_init_header(soap);                                                    // ������Ϣͷ����
    ONVIF_init_ProbeType(soap, &req);                                           // ����Ѱ�ҵ��豸�ķ�Χ������
    result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, nullptr, &req);        // ���鲥��ַ�㲥Probe��Ϣ
    while (SOAP_OK == result)                                                   // ��ʼѭ�������豸���͹�������Ϣ
    {
        memset(&rep, 0x00, sizeof(rep));
        result = soap_recv___wsdd__ProbeMatches(soap, &rep);
        if (SOAP_OK == result) {
            if (soap->error) {
                soap_perror(soap, "ProbeMatches");
            } else {                                                            // �ɹ����յ��豸��Ӧ����Ϣ
                //  printf("__sizeProbeMatch:%d\n",rep.wsdd__ProbeMatches->__sizeProbeMatch);

                if (nullptr != rep.wsdd__ProbeMatches) {
                    count += rep.wsdd__ProbeMatches->__sizeProbeMatch;
                    for(i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                        probeMatch = rep.wsdd__ProbeMatches->ProbeMatch + i;
                        if (nullptr != cb ) {
                            std::string url = probeMatch->XAddrs;
                            if(url == "http://192.168.1.8/onvif/device_service"){
                                cb(probeMatch->XAddrs);                             // ʹ���豸�����ִַ�к����ص�
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
**������ONVIF_SetAuthInfo
**���ܣ�������֤��Ϣ
**������
        [in] soap     - soap��������
        [in] username - �û���
        [in] password - ����
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
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
**������ONVIF_GetDeviceInformation
**���ܣ���ȡ�豸������Ϣ
**������
        [in] DeviceXAddr - �豸�����ַ
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
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
**������ONVIF_GetSnapshotUri
**���ܣ���ȡ�豸ͼ��ץ�ĵ�ַ(HTTP)
**������
        [in]  MediaXAddr    - ý������ַ
        [in]  ProfileToken  - the media profile token
        [out] uri           - ���صĵ�ַ
        [in]  sizeuri       - ��ַ�����С
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
    1). �������е�ProfileToken��֧��ͼ��ץ�ĵ�ַ��������XXXƷ�Ƶ�IPC��������������profile0/profile1/TestMediaProfile������TestMediaProfile���ص�ͼ��ץ�ĵ�ַ���ǿ�ָ�롣
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
**������ONVIF_GetCapabilities
**���ܣ���ȡ�豸������Ϣ
**������
        [in] DeviceXAddr - �豸�����ַ
        [in]
**���أ�
        0�����ɹ�����0����ʧ��
**��ע��
    1). ��������Ҫ�Ĳ���֮һ��ý������ַ
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
**������make_uri_withauth
**���ܣ����������֤��Ϣ��URI��ַ
**������
        [in]  src_uri       - δ����֤��Ϣ��URI��ַ
        [in]  username      - �û���
        [in]  password      - ����
        [out] dest_uri      - ���صĴ���֤��Ϣ��URI��ַ
        [in]  size_dest_uri - dest_uri�����С
**���أ�
        0�ɹ�����0ʧ��
**��ע��
    1). ���ӣ�
    ����֤��Ϣ��uri��rtsp://100.100.100.140:554/av0_0
    ����֤��Ϣ��uri��rtsp://username:password@100.100.100.140:554/av0_0
************************************************************************/
static int make_uri_withauth(const std::string& src_uri, const std::string&username, const std::string&password, std::string *dest_uri)
{
    int result = 0;


            SOAP_ASSERT(!src_uri.empty());

    if (username.empty() &&password.empty()) {                       // �����µ�uri��ַ
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
**������ONVIF_GetStreamUri
**���ܣ���ȡ�豸������ַ(RTSP)
**������
       [in]  MediaXAddr    - ý������ַ
       [in]  ProfileToken  - the media profile token
**���أ�
       0�����ɹ�����0����ʧ��
**��ע��
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
    sprintf(cmd, "wget -O %s '%s'",   "out.jpeg", snapAuthUri.c_str());                        // ʹ��wget����ͼƬ
    system(cmd);*/
}

int main(int argc, char **argv)
{
    ONVIF_DetectDevice(cb_discovery);
    return 0;
}

