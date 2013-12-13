CFLAGS += -pthread

# for sun os
#CFLAGS += -lpthread -lsocket -lnsl -lrt -lnsl -lc -D USE_CUSTOM_INET_ATON

# for gdb
#CFLAGS += -g

all: server cnode client clean

server: data_serialization.o syn_communication.o name_server_info.o data_operation.o server.o
	gcc -o server data_serialization.o syn_communication.o name_server_info.o data_operation.o server.o $(CFLAGS)

cnode: data_serialization.o syn_communication.o name_server_info.o data_operation.o cnode.o
	gcc -o cnode data_serialization.o syn_communication.o name_server_info.o data_operation.o cnode.o $(CFLAGS)

client: data_serialization.o syn_communication.o name_server_info.o data_operation.o client.o
	gcc -o client data_serialization.o syn_communication.o name_server_info.o data_operation.o client.o $(CFLAGS)

client.o:
	gcc -c client.c $(CFLAGS)

data_serialization.o:
	gcc -c data_serialization.c $(CFLAGS)

syn_communication.o:
	gcc -c syn_communication.c $(CFLAGS)

cnode.o:
	gcc -c cnode.c

name_server_info.o:
	gcc -c name_server_info.c $(CFLAGS)

data_operation.o:
	gcc -c data_operation.c

server.o:
	gcc -c server.c $(CFLAGS)

clean:
	rm *.o

