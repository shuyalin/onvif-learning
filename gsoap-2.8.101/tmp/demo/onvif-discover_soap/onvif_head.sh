#!/bin/bash
mkdir onvif_head
cd onvif_head
../bin/wsdl2h -o onvif.h   -t ../gsoap/WS/typemap.dat http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl
