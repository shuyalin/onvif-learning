#!/bin/bash
DIR=soap
mkdir $DIR
cd $DIR
 ../bin/soapcpp2 -2 -x -C ../onvif_head/onvif.h  -L -I ../gsoap/import -I ../gsoap/
