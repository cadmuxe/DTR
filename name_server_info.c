#include <stdio.h>
#include <arpa/inet.h>
#include "name_server_info.h"

unsigned int get_ip(char *str){
    struct in_addr addr;
    
    if(inet_aton(str, &addr) ){
        return addr.s_addr;
    }
    else{
        printf("Can not convert ip\n");
        return 0;
    }
}
