/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Dec 1, 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libgen.h>

#include "data_operation.h"
#include "data_serialization.h"


int load_doc_from_f(char *filepath, struct doc **doc){
    int len, i;
    FILE *fh;
    char *ptr;

    (*doc) = (struct doc *)malloc(sizeof(struct doc));
    (*doc)->size = 0;
    (*doc)->name = NULL;
    (*doc)->data = NULL;

    if( (ptr = basename(filepath)) == NULL)
        return -1;
    len = strlen(ptr) + 1;  // really length 
    (*doc)->name = (char *)malloc( sizeof(char) * len );
    memcpy((void *)(*doc)->name, (void *)ptr, sizeof(char) * len);

    fh = fopen(filepath, "rb");
    fseek(fh, 0, SEEK_END);
    len = (int)ftell(fh);
    fseek(fh, 0, SEEK_SET);

    (*doc)->size = len;
    (*doc)->data = (char *)malloc( sizeof(char) *len);
    memset((*doc)->data, 0, sizeof(char) * len);
    ptr = (*doc)->data;
    while( (i=fread(ptr, sizeof(char), 1024, fh)) > 0){
        ptr += i;
    }
    fclose(fh);
    return 0;
}

int split_doc(struct doc *doc, int n, struct doc_part_list **list){
    int block_size, len, i;
    struct doc_part *pptr;
    char *ptr_start, *ptr_end;

    block_size = doc->size / n;
    if(block_size < 0)
        return -1;

    (*list) = (struct doc_part_list *)malloc(sizeof(struct doc_part_list));

    (*list)->doc_name = doc->name;
    
    (*list)->list = (struct doc_part *)malloc(sizeof(struct doc_part) * n);
    memset((*list)->list, 0, sizeof(struct doc_part) * n);
    (*list)->len_list = n;

    ptr_start = doc->data;
    pptr = (*list)->list;
    // loop n-1 times
    while(--n){
        ptr_end = ptr_start + block_size;
        while(*ptr_end != '\n')
            ptr_end++;

        pptr->doc_name = doc->name;
        pptr->size = ptr_end - ptr_start + 1;
        pptr->start = ptr_start;
        pptr->end = ptr_end;
    
        ptr_start = ptr_end + 1;
        pptr++;
    }

    // for the last(n th) doc_part
    pptr->doc_name = doc->name;
    pptr->start = ptr_start;
    pptr->end = doc->data + doc->size;
    pptr->size = pptr->end - pptr->start + 1;
    
    return 0;
}
    
int free_doc(struct doc **doc){
    if((*doc) == NULL)
        return 0;
    free((*doc)->name);
    free((*doc)->data);
    (*doc) = NULL;
    return 0;
}
int free_doc_part_list(struct doc_part_list **list){
    if((*list) == NULL)
        return 0;
    free((*list)->list);
    free(*list);
    (*list) = NULL;
    return 0;
}
int test_doc(){
    struct doc *doc;
    struct doc_part_list *list;
    struct doc_part *part;
    char *ptr;
    printf("*********** TEST FOR DOC ************\n");
    load_doc_from_f("doc.test", &doc);
    split_doc(doc, 5, &list);
    part = list->list;
    ptr = part->start;
    while(ptr!= part->end){
        printf("%c", *ptr);
        ptr++;
    }
    free_doc_part_list(&list);
    free_doc(&doc);
    printf("*********** FINISH ************\n");
    return 1;
}

int compare_string(char *str1, char *str2){
    int len,i, m,n;
    m = strlen(str1);
    n = strlen(str2);
    if(m == n)
        len = m;
    else if( m > n)
        len = n;
    else
        len =m;
    for(i=0; i< len; i++){
        if((int)str1[i] == (int)str2[i])
            continue;
        else if((int)str1[i] < (int)str2[i])
            return -1;  // str1 is smaller
        else
            return 1;
    }
    if(i == m && i == n)
        return 0;
    else if(i == m)
        return -1;   // str1 is shorter so it's "smaller"
    else
        return 1;
}

int get_count_term(struct count *count, char *term){
    struct count_term_node *ptr;
    int com;
    ptr = count->list;
    while(ptr != NULL){
        com = compare_string(term, ptr->term);
        if(com == 0)
            return ptr->count;
        // term is bigger than current node
        // since the list is sorted, so can not find.
        else if( com == -1)
            break;
    }
    return 0; // can not found, so the count is 0;
}
int add_count_term(struct count *count, char *term, int n){
    struct count_term_node *ptr, *new;
    int com;
    ptr = count->list;
    if(ptr == NULL){
        create_count_term_node(&new, term, n);
        new->pre = NULL;
        new->next =NULL;
        count->list = new;
        return n;
    }
    while(1){
        com = compare_string(term, ptr->term);
        if(com == 0 ){
            ptr->count += n;
            return ptr->count;
        }
        else if(com == -1){
            // the first node biger then term.
            // add a new node(conatin term) before
            // this one.
            create_count_term_node(&new, term, n);
            if(ptr->pre == NULL){
                count->list = new;
            }
            else{
                ptr->pre->next = new;
            }
            new->pre = ptr->pre;
            new->next = ptr;
            ptr->pre = new;
            return n;
        }
        if(ptr->next == NULL){
            break;
        }
        else{
            ptr = ptr->next;
        }
    }

    create_count_term_node(&new, term, n);
    new->next = NULL;
    new->pre = ptr; 
    ptr->next = new;
    return n;
}

int count_doc_part(struct doc_part part, struct count **count){
    struct count *cptr;
    int len;
    char word[50];
    char *pch_doc, *pch_wd;
    char c;
    create_count(&cptr, part.doc_name);
    (*count) = cptr;

    pch_doc = part.start;
    pch_wd = word;
    while(pch_doc <= part.end){
        c = tolower(*pch_doc);
        if((((int)c >=97) && ((int)c <=122)) ){
            while( (((int)c >=97) && ((int)c <=122)) || c == '-'){
                *pch_wd = c;
                pch_wd++;
                pch_doc++;
                c = tolower(*pch_doc);
            }
            *pch_wd = '\0';
            if(add_count_term(cptr, word, 1) == 1)
                cptr->term_count++;
            pch_wd = word;
        }
        else
            pch_doc++;
    }
    return cptr->term_count;
}

int count_doc(struct doc *doc, struct count **count){
    struct count *cptr;
    int len;
    char word[50];
    char *pch_doc, *pch_wd;
    char c;
    create_count(&cptr, doc->name);
    (*count) = cptr;

    pch_doc = doc->data;
    pch_wd = word;
    while(*pch_doc != '\0'){
        c = tolower(*pch_doc);
        if((((int)c >=97) && ((int)c <=122)) ){
            while( (((int)c >=97) && ((int)c <=122)) || c == '-'){
                *pch_wd = c;
                pch_wd++;
                pch_doc++;
                c = tolower(*pch_doc);
            }
            *pch_wd = '\0';
            if(add_count_term(cptr, word, 1) == 1)
                cptr->term_count++;
            pch_wd = word;
        }
        else
            pch_doc++;
    }
    return cptr->term_count;
}
int combine_count(struct count *main, struct count *asst){
    struct count_term_node *pn_main, *pn_asst;
    struct count_term_node *temp;
    int com;
    
    pn_main = main->list;
    pn_asst = asst->list;
    // disable asst, the list will be freed
    asst->list = NULL;

    if(pn_main == NULL){
        main->list = pn_asst;
        pn_asst = pn_asst->next;
        pn_main = main->list;
        main->list->next = NULL;
        main->list->pre = NULL;
        main->term_count = 1;
        asst->term_count -=1;
    }
    
    while(pn_asst != NULL){
        com = compare_string(pn_main->term, pn_asst->term);
        if(com == 0){
            pn_main->count += pn_asst->count;
            temp = pn_asst;
            pn_asst = pn_asst->next;
            free_count_term_node(&temp);
            asst->term_count -= 1;
        }
        else if(com == 1){ // main is bigger
            temp = pn_asst;
            pn_asst = pn_asst->next;
            if(pn_main->pre == NULL){
                main->list = temp;
            }
            else{
                pn_main->pre->next = temp;
            }
            temp->pre = pn_main->pre;
            temp->next = pn_main;
            pn_main->pre = temp;
            pn_main = temp;
            main->term_count += 1;
            asst->term_count -= 1;
        }
        else{
            if(pn_main->next != NULL)
                pn_main = pn_main->next;
            else{
                break;
            }
        }
    }
    if(pn_asst != NULL && pn_main->next == NULL){
        main->term_count += asst->term_count;
        pn_main->next = pn_asst;
        pn_asst->pre = pn_main;
    }
    return main->term_count;
}
int create_count_term_node( struct count_term_node **node, char *term, int n){
    int len;
    (*node) =  (struct count_term_node *)malloc(sizeof(struct count_term_node));
    len = strlen(term) + 1;
    (*node)->term = (char *)malloc(sizeof(char) * len);
    memcpy((void *)((*node)->term), term, sizeof(char) * len);
    (*node)->count = n;
    (*node)->next = NULL;
    (*node)->pre = NULL;
    return 0;
 
}
int create_count(struct count **count, char *name){
    int len;
    len = strlen(name) + 1;
    (*count) = (struct count *)malloc(sizeof(struct count));
    (*count)->name = (char *)malloc(sizeof(char) * len);
    memcpy((void *)(*count)->name, (void *)name, sizeof(char) * len);
    (*count)->whole = 0;
    (*count)->term_count = 0;
    (*count)->list = NULL;

    return 0;
}

int free_count(struct count **count){
    if(*count == NULL)
        return 0;
    free((*count)->name);
    free(*count);
    (*count) = NULL;
    return 0;
}
int free_count_term_node(struct count_term_node **term){
    free((*term)->term);
    free(*term);
    *term = NULL;
    return 0;
}

int test_count(){
    struct doc *doc;
    struct doc_part_list *list;
    struct doc_part *part;
    struct count *count, *count_n; 
    struct count_term_node *tnode;
    char *ptr;
    int i,j;
    load_doc_from_f("doc.test", &doc);
    split_doc(doc, 5, &list);

    count_doc_part(*(list->list), &count);
    for(i=1; i<list->len_list; i++){
        count_doc_part(*(list->list + i), &count_n);
        combine_count(count, count_n);
        free_count(&count_n);
    }
    tnode = count->list;
    printf("**************** Print Count ***************");
    while(tnode != NULL){
        printf("Term: %s\t%d\n", tnode->term, tnode->count);
        tnode = tnode->next;
    }
    free_count(&count);
    return 1;
}
int create_index(struct index **index){
    (*index) = (struct index *)malloc(sizeof(struct index));
    (*index)->len_list = 0;
    (*index)->list = NULL;
    return 0;
}

int create_index_term_node(struct index_term_node **termn, char *term){
    int len;
    (*termn) = (struct index_term_node *)malloc(sizeof(struct index_term_node));
    len = strlen(term) + 1;
    (*termn)->term = (char *)malloc(sizeof(char) * len);
    memcpy((void *)(*termn)->term,
            (void *)(term), 
            sizeof(char) * len);
    (*termn)->len_list = 0;
    (*termn)->list = NULL;
    (*termn)->pre = NULL;
    (*termn)->next = NULL;
    return 1;
}
int create_index_doc_node(struct index_doc_node **docn, char *name, int n){
    int len;
    len = strlen(name) + 1;
    (*docn) = (struct index_doc_node *)malloc(sizeof(struct index_doc_node));
    (*docn)->name = (char *)malloc(sizeof(char) * len);
    memcpy((void *)(*docn)->name, name, sizeof(char) * len);
    (*docn)->count = n;
    (*docn)->pre = NULL;
    (*docn)->next = NULL;
    return 1;
}

int insert_doc_to_term(struct index_term_node *tnode, char *doc_name, int n){
    struct index_doc_node *dnode_p, *new_dnode_p;
    int com;

    if(tnode->list == NULL){
        create_index_doc_node(&new_dnode_p, doc_name, n);
        tnode->list = new_dnode_p;
        tnode->len_list = 1;
        return tnode->len_list;
    }

    dnode_p = tnode->list;
    while(1){
        com = compare_string(dnode_p->name, doc_name);
        if(com == 0 ){
            dnode_p->count += n;
            return tnode->len_list;
        }
        else if(com == 1){
            // dnode is bigger, so insert a new doc node before
            // dnode
            create_index_doc_node(&new_dnode_p, doc_name, n);
            tnode->len_list += 1;
            if(dnode_p->pre == NULL){
                // reach the begining
                new_dnode_p->next = dnode_p;
                tnode->list = new_dnode_p;
                dnode_p->pre = new_dnode_p;
                return tnode->len_list;
            }
            new_dnode_p->pre = dnode_p->pre;
            new_dnode_p->next = dnode_p;
            dnode_p->pre->next = new_dnode_p;
            dnode_p->pre = new_dnode_p;

            return tnode->len_list;
        }
        else{
            if(dnode_p->next != NULL){
                dnode_p = dnode_p->next;
            }
            else{
                create_index_doc_node(&new_dnode_p, doc_name, n);
                tnode->len_list += 1;
                dnode_p->next = new_dnode_p;
                new_dnode_p->pre = dnode_p;
                return tnode->len_list;
            }
        }
    }

}
int add_count_to_index(struct index *index, struct count *count){
    struct count_term_node *tnp;  
    struct index_term_node *itnp, *itnp_temp;
    int com;
    tnp = count->list;
    itnp = index->list;
    if(index->list == NULL){
        create_index_term_node(&itnp, tnp->term);
        insert_doc_to_term(itnp, count->name,
                tnp->count);
        index->list = itnp;
        itnp->next = NULL;
        index->len_list += 1;
        tnp = tnp->next;

    }
    while(tnp != NULL){
        com = compare_string(itnp->term, tnp->term);
        if(com == 0){
            insert_doc_to_term(itnp, count->name,
                    tnp->count);
            tnp = tnp->next;
        }
        else if(com == 1){ // index term is bigger
            create_index_term_node(&itnp_temp, tnp->term);
            insert_doc_to_term(itnp_temp,
                    count->name, tnp->count);
            itnp_temp->pre = itnp->pre;
            if(itnp->pre != NULL){
                itnp->pre->next = itnp_temp;
            }
            else{
                index->list = itnp_temp;
            }
            itnp_temp->next = itnp;
            itnp->pre = itnp_temp;
            tnp = tnp->next;
            itnp = itnp_temp;
            index->len_list += 1;
        }
        else{
            if(itnp->next == NULL){
                create_index_term_node(&itnp_temp, tnp->term);
                insert_doc_to_term(itnp_temp,
                    count->name, tnp->count);
                itnp->next = itnp_temp;
                itnp_temp->pre = itnp;
                tnp = tnp->next;
                index->len_list +=1;
            }
            itnp = itnp->next;
        }
   
   }

    return index->len_list;
}
int free_index(struct index **index){
    struct index_term_node *inode;
    struct index_doc_node *dnode;
    
    inode = (*index)->list;
    while(1){
        if(inode->pre != NULL){
            dnode = inode->pre->list;
            while(1){
                if(dnode->pre != NULL){
                    free(dnode->pre->name);
                    free(dnode->pre);
                }
                if(dnode->next != NULL)
                    dnode = dnode->next;
                else{
                    free(dnode->name);
                    free(dnode);
                    break;
                }
            }
        }
        if(inode->next != NULL)
            inode = inode->next;
        else{
            free(inode->term);
            free(inode);
            break;
        }
    }
    free(*index);
    (*index) = NULL;
    
    return 0;
}
int test_index(){
    struct doc *doc;
    struct index *index, *new_index;
    struct doc_part_list *list;
    struct doc_part *part;

    struct count *count, *count_n; 
    struct count_term_node *tnode;
    int i,j;
    void *data;
    create_index(&index);

    load_doc_from_f("asyoulikeit.txt", &doc);
    split_doc(doc, 5, &list);
    count_doc_part(*list->list, &count);
    for(i=1; i<list->len_list; i++){
        count_doc_part(*(list->list + i), &count_n);
        combine_count(count, count_n);
        free_count(&count_n);
    }
    count->whole =1;
    i = add_count_to_index(index, count);
    dump_index(index, &data);
    load_index(&new_index, data);

    load_doc_from_f("glossary.txt", &doc);
    split_doc(doc, 5, &list);
    count_doc_part(*list->list, &count);
    for(i=1; i<list->len_list; i++){
        count_doc_part(*(list->list + i), &count_n);
        combine_count(count, count_n);
        free_count(&count_n);
    }
    count->whole =1;
    i = add_count_to_index(index, count);
    dump_index(index, &data);
    load_index(&new_index, data);
    printf("%d\n",i);

    return 0;

}

int retrieve_f_index(struct index *index, int n, 
        struct query *query, struct query_rsl **rsl){
    struct query_rsl *n_rsl, *nn_rsl, *newrsl;
    struct query *ptr_query;
    struct index_term_node *inode;
    struct index_doc_node *dnode;
    int i,com, flag;

    i=0;
    flag=1;
    n_rsl = NULL;
    ptr_query= query;
    while(ptr_query != NULL){
        inode = index->list;
        nn_rsl = NULL;
        while(inode != NULL){
            com = compare_string(ptr_query->term, inode->term);
            if(com == 0){
                dnode = inode->list;
                while(dnode != NULL){
                    if(n!=0){
                        i++;    // every term, get the top n docment.
                        if(i>n)
                            break;
                    }       
                    create_query_rsl(&newrsl, dnode->name, dnode->count);
                    if(nn_rsl == NULL)
                        nn_rsl = newrsl;
                    else{
                        newrsl->next = nn_rsl->next;
                        nn_rsl->next = newrsl;
                    }
                    dnode = dnode->next;
                }
            }
            inode = inode->next;
        }
        if(flag){
            flag = 0;
            n_rsl = nn_rsl;
        }
        else
            combine_query_rsl(&n_rsl, &nn_rsl);
        ptr_query = ptr_query->next;
    }
    (*rsl) = n_rsl;
    return 0;
}

int create_query_rsl(struct query_rsl **rsl, char *doc_name, int weight){
    int len;
    (*rsl) = (struct query_rsl *)malloc(sizeof(struct query_rsl));
    memset((void *)(*rsl), 0, sizeof(struct query_rsl));
    len = strlen(doc_name) + 1;
    (*rsl)->doc_name = malloc(sizeof(char) * len);
    memcpy((void *)((*rsl)->doc_name), (void *)doc_name, sizeof(char) * len);
    (*rsl)->weight = weight;
    (*rsl)->next = NULL;
    return 0;
}
int free_query_rsl(struct query_rsl **rsl){
    struct query_rsl *p, *p1;
    p = (*rsl);
    (*rsl) = NULL;
    while(p != NULL){
        p1 = p->next;
        free(p->doc_name);
        free(p);
        p = p1;
    }
    return 0;
}

int create_query(char *term, struct query **query){
    int len;
    len = strlen(term) + 1;
    (*query) = (struct query *)malloc(sizeof(struct query));
    memset((void *)(*query), 0, sizeof(struct query));
    (*query)->term = (char *)malloc(sizeof(char) * len);
    memset((void *)((*query)->term), 0, sizeof(char)*len);
    memcpy((void *)((*query)->term), (void *)term, sizeof(char) * len);
    return 0;
}
int free_query(struct query **query){
    struct query *p, *p1;
    p = (*query);
    (*query) = NULL;
    while(p!= NULL){
        p1 = p->next;
        free(p->term);
        free(p);
        p = p1;
    }
    return 0;
}
int combine_query_rsl(struct query_rsl **rsl, struct query_rsl **rsl_a){
    struct query_rsl *ptr, *ptr2, *ptr_temp;
    struct query_rsl *list, *new;
    int com;
    int i,j;
    list = NULL;
    if(*rsl == NULL){
        return 0;
    }
    else if(*rsl_a == NULL){
        (*rsl) = NULL;
        return 0;
    }
    // find the short one.
    ptr = (*rsl);
    i = 0;
    j = 0;
    while(ptr != NULL){
        i++;
        ptr = ptr->next;
    }
    ptr = (*rsl_a);
    while(ptr != NULL){
        j++;
        ptr = ptr->next;
    }
    if(i<j){
        ptr = (*rsl);
        ptr_temp = (*rsl_a);
    }
    else{
        ptr = (*rsl_a);
        ptr_temp = (*rsl);
    }

    ptr = (*rsl);
    while(ptr != NULL){
         ptr2 = ptr_temp;
         while(ptr2 != NULL){
            com = compare_string(ptr->doc_name, ptr2->doc_name);
            if(com == 0){
                create_query_rsl(&new, ptr->doc_name, ptr->weight + ptr2->weight);
                if(list == NULL)
                    list = new;
                else{
                    new->next = list->next;
                    list->next = new;
                }
            }
            ptr2 = ptr2->next;
         }
        ptr = ptr->next;
    }
    free_query_rsl(rsl);
    free_query_rsl(rsl_a);
    (*rsl) = list;
    return 0;
}

int create_out_buf(struct out_buf **buf){
    (*buf) =(struct out_buf *)malloc(sizeof(struct out_buf));
    (*buf)->buf = (char *)malloc(sizeof(char) * 100);
    memset((void *)((*buf)->buf), 0, sizeof(char)*100);
    (*buf)->size = 100 * sizeof(char);
    (*buf)->use = 0;
    return 0;
}
int write_buf(struct out_buf *buf, char *str){
    int len;
    char *ptr;
    len = strlen(str);
    if(len + buf->use  >= buf->size ){
        buf->buf = realloc((void *)(buf->buf), buf->size + 100 *sizeof(char));
        buf->size += 100 * sizeof(char);
    }
    ptr = buf->buf + buf->use;
    memcpy(ptr, str, sizeof(char) * (len + 1));
    buf->use += len;
    return 0;
}
int write_buf_c(struct out_buf *buf, char c){
    char *ptr;
    if(1 + buf->use  >= buf->size ){
        buf->buf = realloc((void *)(buf->buf), buf->size + 100 *sizeof(char));
        buf->size += 100 * sizeof(char);
    }
    *(buf->buf + buf->use) = c;
    *(buf->buf + buf->use + 1) = '\0';
    buf->use +=1;
    return 0;
}
int write_buf_i(struct out_buf *buf, int i){
    char str[15];
    sprintf(str, "%d", i);
    write_buf(buf, str);
    return 0;
}

int free_buf(struct out_buf **buf){
    free((*buf)->buf);
    free(*buf);
    *buf = NULL;
    return 0;
}

int test_query(){
    struct doc *doc;
    struct index *index;
    struct count *count; 
    struct query *query, *q2;
    struct query_rsl *rsl;
    int i,j;
    
    create_index(&index);

    load_doc_from_f("../shakespeare/comedies/allswellthatendswell", &doc);
    count_doc(doc, &count);
    count->whole =1;
    add_count_to_index(index, count);
    free_doc(&doc);
    free_count(&count);
    
    load_doc_from_f("../shakespeare/comedies/allswellthatendswell", &doc);
    count_doc(doc, &count);
    count->whole =1;
    add_count_to_index(index, count);
    free_doc(&doc);
    free_count(&count);
    
    create_query("a", &query);
    create_query("the", &q2);
    query->next = q2;

    retrieve_f_index(index, 100, query, &rsl);
    return 0;
}
int test_buf(){
    struct out_buf *buf;
    create_out_buf(&buf);
    write_buf_c(buf, '!');
    write_buf(buf, "jdijdiskdidjdijdiddjdidjdijdiskdidjdijdiddjdidjdijdiskdidjdij");
    write_buf(buf, "jdijdiskdidjdijdiddjdidjdijdiskdidjdijdiddjdidjdijdiskdidjdij");
    write_buf(buf, "jdijdiskdidjdijdiddjdidjdijdiskdidjdijdiddjdidjdijdiskdidjdij");
    write_buf(buf, "jdijdiskdidjdijdiddjdidjdijdiskdidjdijdiddjdidjdijdiskdidjdij");
    free_buf(&buf);
    return 0;
}
