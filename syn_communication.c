/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Nov 30, 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>

#include "syn_communication.h"

int open_clientfd(char *hostname, int port){
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if((hp = gethostbyname(hostname)) == NULL)
        return -2;
    memset((void *)&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memcpy((void *)hp->h_addr, 
            (void *)&serveraddr.sin_addr.s_addr, 
            hp->h_length);
    serveraddr.sin_port = htons(port);

    if( connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

int open_listenfd(int port){
    int listenfd, optval;
    optval = 1;
    struct sockaddr_in serveraddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <0)
        return -1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval, sizeof(int)) < 0)
        return -1;

    memset( (void *)&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons( (unsigned short)port);

    if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    if(listen(listenfd, ASYN_COMMUNICATION_LISTENQ) < 0)
        return -1;

    return listenfd;
}

int send_data(int socket_fd, void *data, int size){
    void *ptr;
    int  n;
    int32_t len;

    len = (int32_t)size;
    ptr = data;
    

    send(socket_fd, (void *)&len, sizeof(int32_t), 0);
    while(len){
        n = send(socket_fd, ptr, len, 0);
        ptr +=n;
        len -=n;
    }
    close(socket_fd);
    return 1;
}

int receive_data(int socket_fd, void **data){
    int  n;
    void *ptr;
    int32_t i,len;
    // get the first n byte which is the length of the data,
    // The length is the real length of the cotent, the n byte
    // is not included. n equal to the len of int.
    // the length will include the '\0' if necessary.
    
    if(!recv(socket_fd, (void *)&len, sizeof(int32_t), 0))
        return -1;
    i = len; 
    ptr = (void *)malloc(sizeof(char) * len);
    *data = ptr;

    memset(ptr, 0, len);
    if(ptr == 0)
        return -2;
    while( (n = recv(socket_fd, ptr, i, 0)) > 0){
        ptr += n;
        i -=n;
    }
    close(socket_fd);
    return (int)len;
}

void test(){
    int server_pid, client_pid, pid;
    int server_sk, client_sk;
    int connect_fd;
    int status,len;
    char *rece;
    int n = 2;
    char *str ="test\n";
    printf("len %d\n", (int)strlen(str));
    if( (server_pid =fork()) == 0){
        server_sk = open_listenfd(9999);
        printf("Server: Running\n");
        connect_fd = accept(server_sk, 0 , 0);
        printf("Server: Connected\n");
        len = receive_data(connect_fd, (void **)&rece);
        printf("server: rece code %d\n",len);
        printf("Server: Succ rev- %s(%d)\n", rece, len);
        exit(1);
    }
    else{
        ;
    }
    if( (client_pid = fork()) ==0 ){
        client_sk = open_clientfd("localhost", 9999);
        printf("Client: open!\n");
        status = send_data(client_sk, str, 5);
        close(client_sk);
        printf("Client: closed with %d\n",status);
        printf("Client: closed!\n");
        exit(2);
    }
    while(n){
        pid = wait(&status);
        printf("pid:%d, exit with 0x%x\n", pid, status);
        n--;
    }
    printf("Finished\n");

}


