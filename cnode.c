#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

#include "cnode.h"
#include "syn_communication.h"
#include "data_operation.h"
#include "data_serialization.h"
#include "name_server_info.h"
#include "comm.h"

static struct index *Dindex;
static unsigned short port;
static pthread_mutex_t M_index = PTHREAD_MUTEX_INITIALIZER;

void *work(void *mes){
    void *data, *resp;
    struct parameter *para;
    struct doc *doc;
    struct count *count;
    struct query *query;
    struct query_rsl *query_rsl;
    char command;
    int size;
    
    para = (struct parameter *)mes;
    
    receive_data(para->socket_fd, &data);
    memcpy(&command, data, sizeof(char));

    // do index
    if(command == (char)CMD_INDEX){
        load_doc(&doc, data);
        count_doc(doc, &count);
        size = dump_count(count, &resp);
        send_data(para->socket_fd,resp, size);
        printf("[CNode] Do index for %s\n.", doc->name); 
    }
    // do retrieve
    else if(command == (char)CMD_RETRIEVE){
        load_query(&query, data);
        pthread_mutex_lock(&M_index);
        retrieve_f_index(Dindex, 100, query, &query_rsl);
        pthread_mutex_unlock(&M_index);
        size = dump_query_rsl(query_rsl, &resp);
        send_data(para->socket_fd,resp, size);
        printf("[CNode] Retrieve data.\n");
    }
    /// update index
    else if(command == (char)CMD_UPDATE){
        pthread_mutex_lock(&M_index);
        load_index(&Dindex, data);
        pthread_mutex_unlock(&M_index);
        printf("[CNode] Update the index.\n");
    }
    // wrong request
    else{
    }
    free(data);
    free(resp);
    pthread_exit(NULL);
}

int regis(unsigned short iport){
    char buf[50], *ptr;
    int client_fd;
    char com;
    com = (char)CMD_REGISTER;

    memcpy((void *)buf, (void *)&com, sizeof(char));
    memcpy(buf+1, &iport, sizeof(unsigned short));
    ptr += (sizeof(char) + sizeof(unsigned short)); 
    ptr = '\0';

    client_fd = open_clientfdi(get_ip(NAME_IP), NAME_PORT);
    send_data(client_fd, buf, 10);
    receive_data(client_fd, (void *)&buf);
    if(*buf == (char)CMD_SUCC){
        printf("[CNode] Registered!\n");
    }
    close(client_fd);
    return 0;
}

int main(int argc, char *argv[]){
    int server_fd;
    int connect_fd;
    int i;
    struct parameter *p;
    pthread_t thread;
   
    port = NAME_PORT;
    for(i=0;i<100; i++){
        server_fd = open_listenfd(port);
        if(server_fd != -1){
            printf("[CNode] Binding to port(%d).\n", port);
            break;
        }
        printf("[CNode] Failed to bind port, try a new one.\n");
        port++;
    } 
    regis(port); 
    
    while(1){
        connect_fd = accept(server_fd, NULL, NULL);
        p = (struct parameter *)malloc(sizeof(struct parameter));
        memset(p, 0, sizeof(struct parameter));
        p->socket_fd = connect_fd;
        pthread_create(&thread, NULL, work, (void *)p);
    }

    return 0;

}





