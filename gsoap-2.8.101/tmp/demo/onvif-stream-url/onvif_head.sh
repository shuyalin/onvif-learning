#!/bin/bash
mkdir onvif_head
cd onvif_head
../bin/wsdl2h -o  onvif.h  -s -d -x -t ../gsoap/WS/typemap.dat \
http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl \
https://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl \
http://www.onvif.org/ver10/media/wsdl/media.wsdl
