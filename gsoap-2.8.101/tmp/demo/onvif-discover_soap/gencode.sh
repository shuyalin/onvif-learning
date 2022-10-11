#!/bin/bash
DIR=soap
mkdir $DIR
cd $DIR
 ../bin/soapcpp2 -2 -x  ../onvif_head/onvif.h  -L -I ../gsoap/import -I ../gsoap/
