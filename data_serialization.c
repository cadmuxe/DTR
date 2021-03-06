/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Dec 7, 2013
 */

// Data serialization implement

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_operation.h"
#include "data_serialization.h"

int dump_doc_part(struct doc_part *part, void **data){
    // the order of data:
    // data_type, length_of_doc_name, doc_name, size, data
    size_t size;
    void *ptr;
    int new_size, len_name;
    char mytype = (char)DOC;
    size = 0;
    size += sizeof(char);   // a flag place;
    size += sizeof(char);    // for data_type  
    size += sizeof(int);     // for length of doc_name
    len_name = strlen(part->doc_name) + 1;
    size += len_name * sizeof(char); // doc_name
    size += sizeof(int);     // for the size
    // add a '\0' to the end of data
    new_size = part->size + 1;
    size += sizeof(char) * new_size; // for data
    
    (*data) = malloc(size);
    ptr = (*data);
    memset(ptr, 0, size);
    ptr += sizeof(char);    // flag
    memcpy(ptr, (void *)(&mytype), sizeof(char)); //data_type
    ptr += sizeof(char);
    memcpy(ptr, (void *)(&len_name), sizeof(int));//length of doc_name
    ptr += sizeof(int);
    // set the doc name
    memcpy(ptr, (void *)(part->doc_name), len_name * sizeof(char));
    ptr += len_name * sizeof(char);
    memcpy(ptr, (void *)(&new_size), sizeof(int)); // data size
    ptr += sizeof(int);
    // set the data;
    memcpy(ptr, (void *)(part->start), sizeof(char) * part->size);
    ptr += sizeof(char) * part->size;
    memset(ptr, 0, sizeof(char));
    return size;
}
int dump_doc(struct doc *doc, void **data){
    // the order of data:
    // data_type, length_of_doc_name, doc_name, size, data
    size_t size;
    void *ptr;
    int new_size, len_name;
    char mytype = (char)DOC;
    size = 0;
    size += sizeof(char);   // a flag place;
    size += sizeof(char);    // for data_type  
    size += sizeof(int);     // for length of doc_name
    len_name = strlen(doc->name) + 1;
    size += len_name * sizeof(char); // doc_name
    size += sizeof(int);     // for the size
    // add a '\0' to the end of data
    new_size = doc->size + 1;
    size += sizeof(char) * new_size; // for data
    
    (*data) = malloc(size);
    ptr = (*data);
    memset(ptr, 0, size);
    ptr += sizeof(char);    // flag
    memcpy(ptr, (void *)(&mytype), sizeof(char)); //data_type
    ptr += sizeof(char);
    memcpy(ptr, (void *)(&len_name), sizeof(int));//length of doc_name
    ptr += sizeof(int);
    // set the doc name
    memcpy(ptr, (void *)(doc->name), len_name * sizeof(char));
    ptr += len_name * sizeof(char);
    memcpy(ptr, (void *)(&new_size), sizeof(int)); // data size
    ptr += sizeof(int);
    // set the data;
    memcpy(ptr, (void *)(doc->data), sizeof(char) * doc->size);
    ptr += sizeof(char) * doc->size;
    memset(ptr, 0, sizeof(char));
    return size;
}

int load_doc(struct doc **doc, void *data){
    int len_name, len;
    char *doc_name;
    char mytype;
   
    data += sizeof(char); // flag
    // check
    memcpy(&mytype, data, sizeof(char));
    data += sizeof(char);
    if (mytype != (char) DOC)
        return -1;

    (*doc) = (struct doc *)malloc(sizeof(struct doc));
    // load len_name
    memcpy((void *)(&len_name), data, sizeof(int));
    data += sizeof(int);
    // load name
    (*doc)->name  = (char *)malloc(sizeof(char) * len_name);
    memcpy((void *)( (*doc)->name), data, len_name);
    data += len_name * sizeof(char);
    // load size
    memcpy((void *)(&( (*doc)->size )), data, sizeof(int));
    data += sizeof(int);
    // load data
    (*doc)->data = (char *)malloc(sizeof(char) * (*doc)->size);
    memcpy((void *)(  (*doc)->data), data, 
            sizeof(char) * (*doc)->size);
    return 0;
}

int dump_count(struct count *count, void **data){
    int len_name, len_term;
    size_t size;
    void *ptr;
    struct count_term_node *tnode;
    char mytype;
    mytype =(char)COUNT;

    // calculate the size of data
    size = 0;
    size += sizeof(char);   //flag
    size += sizeof(char);   // mytype
    len_name = strlen(count->name) + 1;
    size += sizeof(int);    // for the length of name
    size += sizeof(char) * len_name; // for the name
    size += sizeof(char);   // for the whole
    size += sizeof(int);    // for the term_count;
    
    tnode = count->list;
    while(tnode != NULL){
        len_term = strlen(tnode->term) + 1;
        size += sizeof(int);    // for the length of term
        size += sizeof(char) * len_term; // for the term
        size += sizeof(int);    //for the count;
        tnode = tnode->next;  
    }

    // serialization
    (*data) = malloc( size);
    ptr = *data;
    memset(ptr, 0, size);
    ptr += sizeof(char); // flag
    memcpy(ptr, (void *)&mytype, sizeof(char)); //mytype
    ptr += sizeof(char);
    
    memcpy(ptr, &len_name, sizeof(int));
    ptr += sizeof(int);

    memcpy(ptr, count->name, len_name * sizeof(char));
    ptr += len_name * sizeof(char);

    memcpy(ptr, &(count->whole), sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, &(count->term_count), sizeof(int));
    ptr += sizeof(int);

    tnode = count->list;
    while(tnode != NULL){
        len_term = strlen(tnode->term) + 1;
        memcpy(ptr, &len_term, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, tnode->term, sizeof(char) * len_term);
        ptr += sizeof(char) * len_term;
        memcpy(ptr, &(tnode->count), sizeof(int));
        ptr += sizeof(int);
        tnode = tnode->next;
    }

    return (int)size;
}

int load_count(struct count **count, void *data){
    int len_name, len_term, len_list, i, j, icount;
    char buf[100];  // buffer for name and term
    struct count_term_node *pre, *cur;
    char mytype;
    
    data += sizeof(char); // flag
    // check
    memcpy(&mytype, data, sizeof(char));
    data += sizeof(char);
    if (mytype != (char) COUNT)
        return -1;

    memcpy(&len_name, data, sizeof(int));
    data += sizeof(int);
    memcpy(buf, data, sizeof(char)* len_name);
    data += sizeof(char) * len_name;
    create_count(count, buf);
    memcpy((void *) &((*count)->whole), data, sizeof(char));
    data += sizeof(char);
    memcpy((void *)(&len_list), data, sizeof(int));
    data += sizeof(int);

    (*count)->term_count = len_list;
    pre = NULL;
    cur = NULL;
    for(i=0; i<len_list; i++){
        memcpy((void *) &len_term, data, sizeof(int));
        data += sizeof(int);
        memcpy((void *)buf, data, sizeof(char) * len_term);
        data += sizeof(char) * len_term;
        memcpy((void *)&icount, data, sizeof(int));
        data += sizeof(int);
        create_count_term_node(&cur, buf, icount);
        cur->pre = pre;
        if(pre != NULL){
            pre ->next = cur;
        }
        else
            (*count)->list = cur;
        pre = cur;
    }

    return 0;
}

int dump_index(struct index *index, void **data){
    size_t size;
    int len_term, len_name;
    char mytype;
    void *ptr;
    struct index_term_node *tnode;
    struct index_doc_node *dnode;
    mytype = (char)INDEX; 
    size = 0;
    size += sizeof(char); // flag
    size += sizeof(char);   // type
    size += sizeof(int);    // index.len_list
    tnode = index->list;
    while(tnode != NULL){
        len_term = strlen(tnode->term) + 1;
        size += sizeof(int);    // length of term
        size += sizeof(char) * len_term; // term
        size += sizeof(int);    // index_term_node.len_list
        dnode = tnode->list;
        while(dnode != NULL){
            len_name = strlen(dnode->name) + 1;
            size += sizeof(int);  // len of name
            size += sizeof(char) * len_name;
            size += sizeof(int);  // for count
            dnode = dnode->next;
        }
        tnode = tnode->next;
    }
    (*data) = malloc(size);
    memset((*data), 0, size);
    ptr = (*data);
    ptr += sizeof(char);  // flag
    memcpy(ptr, &mytype, sizeof(char));
    ptr += sizeof(char);
    memcpy(ptr, &(index->len_list), sizeof(int));
    ptr += sizeof(int);
    tnode = index->list;
    while(tnode != NULL){
        len_term = strlen(tnode->term) + 1;
        memcpy(ptr, &len_term, sizeof(int)); // length of term
        ptr += sizeof(int);
        memcpy(ptr, tnode->term, sizeof(char)*len_term); // term
        ptr += sizeof(char) * len_term;
        memcpy(ptr, &(tnode->len_list), sizeof(int));  // len_list
        ptr += sizeof(int);

        dnode = tnode->list;
        while(dnode != NULL){
            len_name = strlen(dnode->name) + 1;
            memcpy(ptr, &len_name, sizeof(int));   // length of name
            ptr += sizeof(int);
            memcpy(ptr, dnode->name, sizeof(char) *len_name); // name
            ptr += sizeof(char) * len_name;
            memcpy(ptr, &(dnode->count), sizeof(int)); // count
            ptr  += sizeof(int);
            dnode = dnode->next;
        }
        tnode = tnode->next;
    }
    return size;
}

int load_index(struct index **index, void *data){
    struct index_term_node *tnode_pre, *tnode_cur;
    struct index_doc_node *dnode_pre, *dnode_cur;
    int len_term, len_name;
    char buf[50];
    int len, i, j, count;
    char mytype;
    
    data += sizeof(char); // flag
    // check
    memcpy(&mytype, data, sizeof(char));
    data += sizeof(char);
    if (mytype != (char) INDEX)
        return -1;

    memcpy(&len, data, sizeof(int));
    data += sizeof(int);
    
    create_index(index);
    (*index)->len_list = len;
    
    tnode_pre = NULL;
    for(i=0; i < (*index)->len_list; i++){
        memcpy(&len_term, data, sizeof(int));  // length of term
        data += sizeof(int);
        memcpy(buf, data, sizeof(char) * len_term); // term
        data += sizeof(char) * len_term;
        create_index_term_node(&tnode_cur, buf);
        memcpy(&len, data, sizeof(int));        // len_list
        data += sizeof(int);
        tnode_cur->len_list = len;
        tnode_cur->pre = tnode_pre;
        if(tnode_pre == NULL){
            (*index)->list = tnode_cur;
        }
        else{
            tnode_pre->next = tnode_cur;
        }
        
        dnode_pre = NULL;
        for(j=0; j<tnode_cur->len_list; j++){
            memcpy(&len_name, data, sizeof(int));
            data += sizeof(int);
            memcpy(buf, data, sizeof(char)*len_name);
            data += sizeof(char) * len_name;
            memcpy(&count, data, sizeof(int));
            data += sizeof(int);
            create_index_doc_node(&dnode_cur, buf, count);
            dnode_cur->pre = dnode_pre;
            if(dnode_pre == NULL){
                tnode_cur->list = dnode_cur;
            }
            else{
                dnode_pre->next = dnode_cur;
            }
            dnode_pre = dnode_cur;
        }
        tnode_pre = tnode_cur;
    }
    return 0;
}

int test_serialization(){
        struct doc *doc, *ndoc;
        struct index *index, *nindex;
        struct doc_part_list *list;
        struct doc_part *part;

        struct count *count, *count_n, *ncount; 
        struct count_term_node *tnode;
        int i,j;

        void *data;

        create_index(&index);

        load_doc_from_f("../shakespeare/comedies/asyoulikeit", &doc);
        split_doc(doc, 5, &list);
        count_doc_part(*list->list, &count);
        for(i=1; i<list->len_list; i++){
            // dump doc_part
            dump_doc_part((list->list + i), &data);
            load_doc(&ndoc, data);
            free(data);

            count_doc(ndoc, &count_n);
            free_doc(&ndoc);
            // dump count 
            dump_count(count_n, &data);
            free_count(&count_n);
            load_count(&ncount, data);
            free(data);

            combine_count(count, ncount);
            free_count(&count_n);
        }
        count->whole =1;
        i = add_count_to_index(index, count);
        load_doc_from_f("../shakespeare/comedies/allswellthatendswell", &doc);
        count_doc(doc, &count);
        count->whole =1;
        i = add_count_to_index(index, count);
        //dump index
        dump_index(index, &data);
        free_index(&index);
        load_index(&nindex, data);
        free(data);

        printf("%d\n",i);

        return 0;

}

int dump_query(struct query *query, void **data){
    size_t size;
    char mytype;
    void *ptr;
    int len_term, len;
    struct query *q;
    
    mytype = (char)QUERY;
    // calculate size;
    size =0;
    size += sizeof(char); //flag
    size += sizeof(char); //type
    size += sizeof(int); // len of query
    
    q = query;
    len = 0;
    while(q != NULL){
        len++;
        len_term = strlen(q->term) +1;
        size += sizeof(int); // len of term
        size += sizeof(char) * len_term;  // term
        q = q->next;
    }
    
    (*data) = malloc(size);
    memset(*data, 0, size);

    ptr = (*data);
    ptr += sizeof(char);    //flag
    
    memcpy(ptr, (void *)&mytype, sizeof(char));
    ptr += sizeof(char);    //type

    memcpy(ptr, (void *)&len, sizeof(int));
    ptr += sizeof(int); // len of query

    q= query;
    while(q != NULL){
        len_term = strlen(q->term) +1;
        memcpy(ptr, (void *)&len_term, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, (void *)(q->term), sizeof(char)*len_term );
        ptr += sizeof(char) * len_term;
        q = q->next;
    }
    return size;
}

int load_query(struct query **query, void *data){
    struct query *q, *new;
    char mytype;
    int len, i;
    int len_term;
    char buf[100];
    data += sizeof(char);   // flag
    memcpy((void *)&mytype, data, sizeof(char));
    data += sizeof(char);   // type
    if(mytype != (char)QUERY)
        return -1;
    memcpy((void *)&len, data, sizeof(int));
    data += sizeof(int);    // len of query
    
    memcpy((void *)&len_term, data, sizeof(int));
    data += sizeof(int);
    memcpy((void *)buf, data, sizeof(char) * len_term);
    data += sizeof(char) *len_term;
    create_query(buf, &new);

    (*query) = new;
    q = new;

    for(i=1; i< len; i++){
        memcpy((void *)&len_term, data, sizeof(int));
        data += sizeof(int);
        memcpy((void *)buf, data, sizeof(char) * len_term);
        data += sizeof(char) *len_term;
        create_query(buf, &new);
        q->next = new;
        q=new;
    }
    return 0;
}

int dump_query_rsl(struct query_rsl *rsl, void **data){
    size_t size;
    char mytype;
    void *ptr;
    int len_doc, len;
    struct query_rsl *r;
    
    mytype = (char)QUERY_RSL;
    // calculate size;
    size =0;
    size += sizeof(char); //flag
    size += sizeof(char); //type
    size += sizeof(int); // len of query_rsl
    
    r = rsl;
    len = 0;
    while(r != NULL){
        len++;
        len_doc = strlen(r->doc_name) +1;
        size += sizeof(int); // len of doc_name
        size += sizeof(char) * len_doc;  // doc_name
        size += sizeof(int);    // weight
        r = r->next;
    }
    
    (*data) = malloc(size);
    memset(*data, 0, size);

    ptr = (*data);
    ptr += sizeof(char);    //flag
    
    memcpy(ptr, (void *)&mytype, sizeof(char));
    ptr += sizeof(char);    //type

    memcpy(ptr, (void *)&len, sizeof(int));
    ptr += sizeof(int); // len of queery_rsl

    r= rsl;
    while(r != NULL){
        len_doc = strlen(r->doc_name) +1;
        memcpy(ptr, (void *)&len_doc, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, (void *)(r->doc_name), sizeof(char)*len_doc );
        ptr += sizeof(char) * len_doc;
        memcpy(ptr, (void *)&(r->weight), sizeof(int));
        ptr += sizeof(int);
        r = r->next;
    }
    return size;
}

int load_query_rsl(struct query_rsl **rsl, void *data){
    struct query_rsl *r, *new;
    char mytype;
    int len, i, weight;
    int len_doc;
    char buf[100];
    data += sizeof(char);   // flag
    memcpy((void *)&mytype, data, sizeof(char));
    data += sizeof(char);   // type
    if(mytype != (char)QUERY_RSL)
        return -1;
    memcpy((void *)&len, data, sizeof(int));
    data += sizeof(int);    // len of query_rsl
    if(len == 0){
        (*rsl) = NULL;
        return 0;
    }
    
    memcpy((void *)&len_doc, data, sizeof(int));
    data += sizeof(int);
    memcpy((void *)buf, data, sizeof(char) * len_doc);
    data += sizeof(char) *len_doc;
    memcpy((void *)&weight, data, sizeof(int)); // weight
    data += sizeof(int);
    create_query_rsl(&new, buf, weight);

    (*rsl) = new;
    r = new;

    for(i=1; i< len; i++){
        memcpy((void *)&len_doc, data, sizeof(int));
        data += sizeof(int);
        memcpy((void *)buf, data, sizeof(char) * len_doc);
        data += sizeof(char) *len_doc;
        memcpy((void *)&weight, data, sizeof(int)); // weight
        data += sizeof(int);
        create_query_rsl(&new, buf, weight);
        r->next = new;
        r=new;
    }
    return 0;
}

int test_s_query(){
    struct query *query, *q2, *nq;
    struct query_rsl *rsl, *rsl2, *nr;
    void *data;
    
    create_query("a", &query);
    create_query("the", &q2);
    query->next = q2;
    dump_query(query, &data);
    load_query(&nq, data);

    create_query_rsl(&rsl, "hi",10);
    create_query_rsl(&rsl2, "go", 100);
    rsl->next = rsl2;
    dump_query_rsl(rsl, &data);
    load_query_rsl(&nr, data);
    
   return 0; 

}


