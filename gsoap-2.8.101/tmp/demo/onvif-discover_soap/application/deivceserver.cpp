#include "stdsoap2.h"
#include "soapStub.h"
//#include "wsdd.nsmap"
/** Web service one-way operation 'SOAP_ENV__Fault' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap*soap, char *faultcode, char *faultstring,
                                          char *faultactor, struct SOAP_ENV__Detail *detail,
                                          struct SOAP_ENV__Code *SOAP_ENV__Code,
                                          struct SOAP_ENV__Reason *SOAP_ENV__Reason,
                                          char *SOAP_ENV__Node, char *SOAP_ENV__Role,
                                          struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
    printf("SOAP_ENV__Fault %s, %d\n", __FUNCTION__, __LINE__);

    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap*, struct wsdd__HelloType *wsdd__Hello){
    printf("__wsdd__Hello %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap*, struct wsdd__ByeType *wsdd__Bye){
    printf("__wsdd__Bye %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap*, struct wsdd__ProbeType *wsdd__Probe){
    printf("__wsdd__Probe %s, %d\n", __FUNCTION__, __LINE__);
    								 
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap*, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches){
    printf("__wsdd__ProbeMatches %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap*, struct wsdd__ResolveType *wsdd__Resolve){
    printf("__wsdd__Resolve %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap*, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches){
    printf("__wsdd__ResolveMatches %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap*, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType &tdn__HelloResponse)
{
    printf("__tdn__Hello %s, %d\n", __FUNCTION__, __LINE__);

    return 0;
}
/** Web service operation '__tdn__Bye' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap*, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType &tdn__ByeResponse)
{
    printf("__tdn__Bye %s, %d\n", __FUNCTION__, __LINE__);

    return 0;
}
/** Web service operation '__tdn__Probe' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap*, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType &tdn__ProbeResponse)
{
    printf("__tdn__Bye %s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

#if 0
int main(int argc, char **argv){
    int m, s;
    
    struct ip_mreq mcast;

    struct soap probe_soap;
    soap_init2(&probe_soap, SOAP_IO_UDP|SOAP_IO_FLUSH, SOAP_IO_UDP|SOAP_IO_FLUSH); //????????????UDP
    probe_soap.bind_flags = SO_REUSEADDR;  // ????????????????????????????????????????????????
    soap_set_namespaces(&probe_soap, namespaces);
    if(!soap_valid_socket(soap_bind(&probe_soap, NULL, 3702, 10)))
    {
        soap_print_fault(&probe_soap, stderr);
        exit(1);
    }


    mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250"); //?????????
    mcast.imr_interface.s_addr = htonl(INADDR_ANY); //????????????????????????IP??????????????????INADDR_ANY????????????ip
    if(setsockopt(probe_soap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
    {
        printf("setsockopt error!\n");
        return 0;
    }

    //??????????????????????????????
    for(;;)
    {
        if(!soap_valid_socket(soap_accept(&probe_soap)))
        {
            soap_print_fault(&probe_soap, stderr);
            exit(-1);
        }

        fprintf(stderr, "Socket connection successful:slave socket = %d \n", s);

        /* ????????????????????????????????????????????????????????????????????????????????????????????????
            ??????????????????????????????????????????????????????????????????????????????????????????web??????
            ???????????????????????????????????????soap????????????
        */
        if (soap_serve(&probe_soap))
            soap_print_fault(&probe_soap, stderr);
        soap_destroy(&probe_soap);	//?????????????????????soap??????
        soap_end(&probe_soap);		//????????????????????????????????????
    }

    soap_done(&probe_soap);
}
#endif