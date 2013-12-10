#include <stdio.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include "syn_communication.h"
#include "data_operation.h"
#include "data_serialization.h"
#include "name_server_info.h"


static struct cnode_list clist;
static struct index *Dindex;

static pthread_mutex_t M_clist = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t M_index = PTHREAD_MUTEX_INITIALIZER;
static sem_t *S_num_node;

void * dispatch(void *mes){
    void *data;
    struct parameter *para;
    unsigned short i;
    char succ;
    para = (struct parameter *)mes;
    receive_data(para->socket_fd,  &data);
    succ = (char)CMD_SUCC; 
    // Register a computation node
    if(  *((char *)data) == (char)CMD_REGISTER){
        i = *(unsigned short *)(data+1);
        do_reg(&clist, para->ip, i);
        send_data(para->socket_fd, &succ, sizeof(char));
    }
    // A client do an index
    else if (*((char *)data) == (char)CMD_INDEX){
        do_index(para->socket_fd, (data+1));    // +1 skip the command filed
    }
    else if(*((char *)data) == (char)CMD_RETRIEVE){
        do_retr(para->socket_fd, data); 
    }
    free(data);
    close(para->socket_fd);
    pthread_exit(0);
}

int do_reg(struct cnode_list *list, unsigned int ip, 
        unsigned short port){
    struct in_addr client;
    struct cnode *node;
    char *address;
    client.s_addr = ip;
    node = (struct cnode *)malloc(sizeof(struct cnode));
    memset((void *)node, 0, sizeof(struct cnode));
    node->ip = ip;
    node->port = port;
    node->status = IDLE;
    
    // lock the clist
    pthread_mutex_lock(&M_clist);
    node->next = list->list;
    list->list = node;
    sem_post(S_num_node);
    list->num_node +=1;
    list->ava_node +=1;

    pthread_mutex_unlock(&M_clist);
    address = inet_ntoa(client);
    printf("[Message] Calculation Node: %s registered\n", address);

    return 0;
}

int do_index(int socket_fd, void *data){
    int num_workers;
    struct workers *workers, *p_worker;
    char type;
    char *path;
    struct doc *doc;
    struct doc_part_list *doc_list;
    struct count *count;
    int i,j;
    char respond;
    respond = (char)CMD_SUCC;
    type = *(char *)data;
    data += 1;
    path = data;
    // load one file, path is a file.
    if(type == (char)CMD_INDEX_FILE){
        load_doc_from_f(path, &doc);
        create_count(&count, doc->name);
        num_workers = get_workers(&clist, &workers);
        split_doc(doc, num_workers, &doc_list);
        p_worker = workers;
        while(p_worker != NULL){
            p_worker->parameter = (void *)(doc_list->list + p_worker->index);
            pthread_create(&(p_worker->thread), NULL,
                   index_agent, (void *)p_worker); 
            p_worker = p_worker->next;
        }
        p_worker = workers;
        while(p_worker != NULL){
            pthread_join(p_worker->thread, NULL);
            p_worker = p_worker->next;
        }
        p_worker = workers;
        while(p_worker != NULL){
            combine_count(count, (struct count *)(p_worker->result));
            p_worker = p_worker->next;
        }
        free_workers(&workers);
        add_count_to_index(Dindex, count);
        free_count(&count);
        do_update();
    }
    // load all file, path is a folder
    else if(type  == (char)CMD_INDEX_FILE){
    }
    else{
        return -1;
    }
    send_data(socket_fd, &respond,sizeof(char));
    return 0;
}

void *index_agent(void *msg){
    // send doc_part to node
    // get count data;
    struct workers *worker;
    int size_data;
    int connect_fd;
    void *data;
    struct count *count;
    char commd;
    commd = (char)INDEX;
    // send doc_part
    worker = (struct workers *)msg;
    size_data = dump_doc_part((struct doc_part *)(worker->parameter) ,&data);
    *(char *)data = commd;    // set flag, I menas index
    connect_fd = open_clientfdi(worker->ip, worker->port);
    send_data(connect_fd, data, size_data);
    free(data);
    data = NULL;
    // receive count 
    size_data = receive_data(connect_fd, &data);
    load_count(&count, data);
    free(data);
    worker->result = (void *)count;
    worker->finshed = 1;
    pthread_exit(NULL);
}
int do_retr(int socket_fd, void *data){
    struct query *query;
    struct workers *workers, *p_worker;
    struct query_rsl *qrsl;
    int num_worker;
    int i;
    struct out_buf *out;
    int limit =10;
    qrsl = NULL;
    load_query(&query, data);
    split_query(query, &workers);
    p_worker = workers;
    while(p_worker != NULL){
        pthread_create(&(p_worker->thread), NULL,
                   retr_agent, (void *)p_worker); 
        p_worker = p_worker->next;
    }
    p_worker = workers;
    while(p_worker != NULL){
        pthread_join(p_worker->thread, NULL);
        p_worker = p_worker->next;
    }
    p_worker = workers;
    while(p_worker != NULL){
        combine_query_rsl(&qrsl, (struct query_rsl **)&((p_worker->result)));
        p_worker = p_worker->next;
    }
    create_out_buf(&out);
    while(qrsl != NULL && limit >0){
        write_buf(out, qrsl->doc_name);
        write_buf(out, "\t\t");
        write_buf(out, "\n");
        qrsl = qrsl->next; 
        limit--;
    }
    free_workers(&workers);
    send_data(socket_fd, out->buf, out->size);
    return 0;
}
void *retr_agent(void *msg){
    struct workers *worker;
    int size_data;
    int connect_fd;
    void *data;
    struct query_rsl *rsl;
    char commd;
    commd = (char)CMD_RETRIEVE;
    worker = (struct workers *)msg;
    size_data = dump_query((struct query *)(worker->parameter), &data);
    *(char *)data = commd;    // set flag, I menas index
    connect_fd = open_clientfdi(worker->ip, worker->port);
    send_data(connect_fd, data, size_data);
    free(data);
    data = NULL;
    size_data = receive_data(connect_fd, &data);
    load_query_rsl(&rsl, data);
    free(data);
    worker->result = (void *)rsl;
    worker->finshed = 1;
    pthread_exit(NULL);
}
// split query based on the term number and availbe workers
int split_query(struct query *query, struct workers **wlist){
    int num_query, num_worker,i;
    struct query *pquery;
    struct workers *workers;
    pquery = query;
    num_query = 0;
    while(pquery != NULL){
        num_query ++;
        pquery =pquery->next;
    }
    num_worker = get_workers_n(&clist, &workers, num_query);
    (*wlist) = workers;
    pquery = query;
    for(i=0; i < num_worker-1; i++){
        workers->parameter = (struct query *)pquery;
        pquery = pquery->next;
        ((struct query *)(workers->parameter))->next = NULL;
        workers = workers->next;
    }
    workers->parameter = (struct query *)pquery;
    
    return num_worker;
}

int get_workers(struct cnode_list *cl, struct workers **wl){
    int n;
    n = cl->ava_node;
    if(n==0)
        n=1;
    return get_workers_n(cl, wl, n);

}
int get_workers_n(struct cnode_list *cl, struct workers **wl, int n){
    int i;
    struct cnode *p_node;
    struct workers *worker;
    if(cl->num_node <1){
        printf("No calculation node connted.\n");
        return 0;
    }
    if(n> cl->num_node)
        n = cl->num_node;
    else
        n = cl->ava_node;
    if(n==0)
        n =1;
    for(i=0; i<n; i++){
        sem_wait(S_num_node);
    }
    worker = (struct workers *)malloc(sizeof(struct workers) * n);
    memset(worker, 0, sizeof(struct workers) * n);
    (*wl) = worker;
    pthread_mutex_lock(&M_clist); 
    p_node = cl->list;
    for(i=0; i<n; i++){
        if(p_node == NULL){
            printf("Error!\n");
            return -1;
        }
        if(p_node->status == IDLE){
            worker->index = 0;
            worker->ip = p_node->ip;
            worker->port = p_node->port;
            worker->finshed = 0;
            if(i!=n-1){
                worker->next = worker+1;
            }
            worker = worker+1;
            p_node->status = WORKING;
            cl->ava_node -=1;
        }
        p_node = p_node->next;
    }
    pthread_mutex_unlock(&M_clist);
    return n;
}
int free_workers(struct workers **list){
    struct workers *worker;
    struct cnode *node;
    int n,i;
    n = 0;
    worker = *list;
    pthread_mutex_lock(&M_clist);
    while(worker != NULL){
        n++;
        node = clist.list;
        while(node != NULL){
            if(node->ip == worker->ip && node->port == worker->port){
                node->status = IDLE;
            }
            node = node->next;
        }
        worker = worker->next;
    }
    clist.ava_node += n;
    pthread_mutex_unlock(&M_clist);
    for(i=0; i<n; i++){
        sem_post(S_num_node);
    }
    free(*list);
    (*list) = NULL;
    return 0;
}

int do_update(){
    int connect_fd,size;
    struct cnode *pnode;
    void *data;
    char commd;
    commd = (char)CMD_UPDATE;
    pnode = clist.list;
    pthread_mutex_lock(&M_index);
    size = dump_index(Dindex, &data);
    pthread_mutex_unlock(&M_index);
    *((char *)data) = commd;
    while(pnode != NULL){
        connect_fd = open_clientfdi(pnode->ip, pnode->port);
        send_data(connect_fd, data, size);
        close(connect_fd);
        pnode = pnode->next;
    }
    free(data);
    return 0;
}

int main(int argc, char *argv[]){
    int server_fd;
    int connect_fd;
    struct parameter *p;
    struct sockaddr_in clientaddr;
    int clientlen;
    pthread_t thread;
   
    if ((S_num_node = sem_open("/semaphore", O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    server_fd= open_listenfd(NAME_PORT);
    create_index(&Dindex);
    printf("Server: Running\n");
    while(1){
        connect_fd = accept(server_fd, (struct sockaddr *)&clientaddr, 
                (socklen_t *)&clientlen);
        p = (struct parameter *)malloc(sizeof(struct parameter));
        memset(p, 0, sizeof(struct parameter));
        p->socket_fd = connect_fd;
        p->ip = clientaddr.sin_addr.s_addr;
        pthread_create(&thread, NULL, dispatch, (void *)p); 
    }
   
    if (sem_close(S_num_node) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/semaphore") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    return 0;

}
