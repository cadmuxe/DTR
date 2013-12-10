#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "name_server_info.h"
#include "data_serialization.h"
#include "syn_communication.h"
#include "data_operation.h"
#include "client.h"
#include "name_server_info.h"
#include "comm.h"

int req_index(char *path){
    struct  out_buf *buf;
    char *ptr;
    ptr = path;
    while(*ptr != NULL){
        if(*ptr == '\n')
            *ptr = '\0';
        ptr++;
    }

    create_out_buf(&buf);
    write_buf_c(buf, (char)CMD_INDEX);
    write_buf_c(buf, (char)CMD_INDEX_FILE);
    write_buf(buf, path);
    process(buf->buf, buf->use +1);
    free_buf(&buf);
    return 0;
}
int process(void *data, int len){
    int client_fd;
    void *respond;

    client_fd = open_clientfdi(get_ip(NAME_IP), NAME_PORT);
    printf("[Client] Connected to server.\n");
    
    send_data(client_fd, data, len);
    printf("[Client] Waitting for respond.\n");
    
    receive_data(client_fd, &respond);
    printf("[Client] Resul:\n");
    printf("%s\n", (char *)respond);
    close(client_fd);
    free(respond);
    return 0;
}

int req_retr(struct query *query){
    int client_fd,len;
    void *data;
    char comm;
    comm = (char)CMD_RETRIEVE;
    len = dump_query(query, &data);
    
    *((char *)data) = comm;

    process(data, len);
    free(data);
    return 0;
}
int run(){
    char buf[100];
    char *ptr;
    char sep[] = " ";
    int com;
    struct query *query, *new;

    while(1){
        query = NULL;
        printf("> ");
        fgets(buf, 100, stdin);
        ptr = buf;
        while(*ptr != NULL){
            if(*ptr == '\n')
                *ptr = '\0';
            ptr++;
        }
        ptr = strtok(buf, sep);
        com = compare_string(ptr, "index");
        if(com == 0){
            ptr = strtok(NULL, sep);
            if(ptr != NULL)
                req_index(ptr);
            else{
                printf("Need a path.\n");
            }
            continue;
        }
        com = compare_string(ptr, "search");
        if(com == 0){
            do{
                ptr = strtok(NULL, sep);
                if(ptr != NULL){
                    create_query(ptr, &new);
                    if(query == NULL){
                        query = new;
                    }
                    else{
                        new->next = query->next;
                        query->next = new;
                    }
                }
            }while(ptr != NULL);
            req_retr(query);
            continue;
        }
        printf("Wrong command.\n");
        printf("\t\tindex filepath:\t index a file\n");
        printf("\t\tsearch term1 term2:\t searching\n");
    }
    return 0;
}

int main(int argc, char *argv[]){
    int i,j,com;
    struct query *query, *new;
    if(argc < 3)
        run();
    else{
        com =  compare_string(argv[1], "index");
        if(com == 0){
            req_index(argv[2]);
            return 0;
        }
        com = compare_string(argv[1], "search");
        printf("%s %d\n",argv[1], com);
        if(com == 0){
            for(i=2; i< argc; i++){
                create_query(argv[i], &new);
                if(query == NULL){
                    query = new;
                }
                else{
                    new->next = query->next;
                    query->next = new;
                }
            }
            req_retr(query);
            return 0;
        }
        printf("Wrong command.\n");
        printf("\t\tindex filepath:\t index a file\n");
        printf("\t\tsearch term1 term2:\t searching\n");

    }
    return 0;
}
