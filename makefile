all: server cnode client

server: data_serialization.o syn_communication.o name_server_info.o data_operation.o server.o
	gcc -o server data_serialization.o syn_communication.o name_server_info.o data_operation.o server.o

cnode: data_serialization.o syn_communication.o name_server_info.o data_operation.o cnode.o
	gcc -o cnode data_serialization.o syn_communication.o name_server_info.o data_operation.o cnode.o

client: data_serialization.o syn_communication.o name_server_info.o data_operation.o client.o
	gcc -o client data_serialization.o syn_communication.o name_server_info.o data_operation.o client.o

client.o:
	gcc -c client.c

data_serialization.o:
	gcc -c data_serialization.c

syn_communication.o:
	gcc -c syn_communication.c

cnode.o:
	gcc -c cnode.c

name_server_info.o:
	gcc -c name_server_info.c

data_operation.o:
	gcc -c data_operation.c

server.o:
	gcc -c server.c

clean:
	rm *.o

