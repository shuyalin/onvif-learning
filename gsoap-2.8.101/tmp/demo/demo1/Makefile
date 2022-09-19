ROOT_PATH=/home/brian/work_dir/onvif-learning/gsoap-2.8.101/tmp/
CC=gcc
INCLUDE=-I$(ROOT_PATH)
SERVER_OBJS=addserver.o soapC.o stdsoap2.o soapServer.o 
CLIENT_OBJS=addclient.o soapC.o stdsoap2.o soapClient.o addtest.o
all:server
server:$(SERVER_OBJS)
	$(CC) $(INCLUDE) -o addserver $(SERVER_OBJS)
	
client:$(CLIENT_OBJS)
	$(CC) $(INCLUDE) -o addtest $(CLIENT_OBJS)
	
clean:
	rm -rf *.o addserver addclient
	