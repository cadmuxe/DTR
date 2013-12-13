#include <stdio.h>
#include <arpa/inet.h>
#include "name_server_info.h"

#ifdef USE_CUSTOM_INET_ATON 
    
int inet_aton(const char *name, struct in_addr *addr) { 
    in_addr_t a = inet_addr (name); 
    addr->s_addr = a; 
    return a != (in_addr_t)-1; 
} 

#endif 


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
