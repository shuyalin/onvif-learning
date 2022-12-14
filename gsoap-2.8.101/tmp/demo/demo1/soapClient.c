/* soapClient.c
   Generated by gSOAP 2.8.101 for add.h

gSOAP XML Web services tools
Copyright (C) 2000-2020, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.8.101 2022-09-19 07:16:13 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns2__add(struct soap *soap, const char *soap_endpoint, const char *soap_action, int num1, int num2, int *sum)
{	if (soap_send_ns2__add(soap, soap_endpoint, soap_action, num1, num2) || soap_recv_ns2__add(soap, sum))
		return soap->error;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send_ns2__add(struct soap *soap, const char *soap_endpoint, const char *soap_action, int num1, int num2)
{	struct ns2__add soap_tmp_ns2__add;
	if (soap_endpoint == NULL)
		soap_endpoint = "";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns2__add.num1 = num1;
	soap_tmp_ns2__add.num2 = num2;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/"; /* use SOAP encoding style */
	soap_serializeheader(soap);
	soap_serialize_ns2__add(soap, &soap_tmp_ns2__add);
	if (soap_begin_count(soap))
		return soap->error;
	if ((soap->mode & SOAP_IO_LENGTH))
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns2__add(soap, &soap_tmp_ns2__add, "ns2:add", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns2__add(soap, &soap_tmp_ns2__add, "ns2:add", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv_ns2__add(struct soap *soap, int *sum)
{
	struct ns2__addResponse *soap_tmp_ns2__addResponse;
	if (!sum)
		return soap_closesock(soap);
	soap_default_int(soap, sum);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns2__addResponse = soap_get_ns2__addResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns2__addResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (sum && soap_tmp_ns2__addResponse->sum)
		*sum = *soap_tmp_ns2__addResponse->sum;
	return soap_closesock(soap);
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.c */
