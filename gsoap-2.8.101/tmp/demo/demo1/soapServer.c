/* soapServer.c
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

SOAP_SOURCE_STAMP("@(#) soapServer.c ver 2.8.101 2022-09-19 07:16:13 GMT")
SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	soap->keep_alive = soap->max_keep_alive + 1;
#endif
	do
	{
#ifndef WITH_FASTCGI
		if (soap->keep_alive > 0 && soap->max_keep_alive > 0)
			soap->keep_alive--;
#endif
		if (soap_begin_serve(soap))
		{	if (soap->error >= SOAP_STOP)
				continue;
			return soap->error;
		}
		if ((soap_serve_request(soap) || (soap->fserveloop && soap->fserveloop(soap))) && soap->error && soap->error < SOAP_STOP)
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}
#ifdef WITH_FASTCGI
		soap_destroy(soap);
		soap_end(soap);
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#ifndef WITH_NOSERVEREQUEST
SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	(void)soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "ns2:add"))
		return soap_serve_ns2__add(soap);
	return soap->error = SOAP_NO_METHOD;
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns2__add(struct soap *soap)
{	struct ns2__add soap_tmp_ns2__add;
	struct ns2__addResponse soap_tmp_ns2__addResponse;
	int soap_tmp_int;
	soap_default_ns2__addResponse(soap, &soap_tmp_ns2__addResponse);
	soap_default_int(soap, &soap_tmp_int);
	soap_tmp_ns2__addResponse.sum = &soap_tmp_int;
	soap_default_ns2__add(soap, &soap_tmp_ns2__add);
	if (!soap_get_ns2__add(soap, &soap_tmp_ns2__add, "ns2:add", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ns2__add(soap, soap_tmp_ns2__add.num1, soap_tmp_ns2__add.num2, soap_tmp_ns2__addResponse.sum);
	if (soap->error)
		return soap->error;
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/"; /* use SOAP encoding style */
	soap_serializeheader(soap);
	soap_serialize_ns2__addResponse(soap, &soap_tmp_ns2__addResponse);
	if (soap_begin_count(soap))
		return soap->error;
	if ((soap->mode & SOAP_IO_LENGTH))
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns2__addResponse(soap, &soap_tmp_ns2__addResponse, "ns2:addResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns2__addResponse(soap, &soap_tmp_ns2__addResponse, "ns2:addResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapServer.c */
