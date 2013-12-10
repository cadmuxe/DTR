/* Author: Koonwah Chen
 * Email: cadmuxeATgmail.com
 * Date: Dec 8, 2013
 */

#ifndef SERVER_H_
#define SERVER_H_
#include <netinet/in.h>
#include "data_operation.h"
#include "comm.h"

enum node_state{
    IDLE = 0,
    WORKING
};
struct parameter{
    int socket_fd;
    unsigned int ip;
};

struct cnode_list{
    int num_node;
    int ava_node;
    struct cnode *list;
};
struct cnode{
    unsigned int ip;
    unsigned short port;
    enum node_state status;
    struct cnode *next;
};

struct workers{
    short index; 
    unsigned int ip;
    unsigned short port;
    char finshed;   // 0 or 1
    void *parameter;
    void *result;
    pthread_t thread;
    struct workers *next;
};

extern int get_workers(struct cnode_list *clist, struct workers **wlist);
extern int get_workers_n(struct cnode_list *clist, 
        struct workers **wlist, int n);
extern int free_workers(struct workers **list);

extern void *dispatch(void *mes);
extern int do_reg(struct cnode_list *list, unsigned int ip, 
        unsigned short port);
extern int do_index(int socket_fd, void *data);
extern void *index_agent(void * msg);
extern int do_retr(int socket_fd, void *data);
extern void *retr_agent(void *msg);
extern int do_update();
extern int split_query(struct query *query, struct workers **wlist);


#endif
