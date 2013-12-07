/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Dec 1, 2013
 */

#ifndef DATA_OPERATION_H_
#define DATA_OPERATION_H_
/*****************************
 * This part about the raw document data.
 *
 *****************************/
// used for contain the document and the info.
struct doc{
    char *name;
    int size;    // size of data;
    char *data;
};

// part of doc, not have really memory, but point to 
// struct doc - data

struct doc_part{
    char *doc_name;
    int size;  // size in byte
    char *start;  // start point of part of doc
    char *end;   // end point of part of doc
};

struct doc_part_list{
    char *doc_name;
    int len_list;
    struct doc_part *list;
};

extern int load_doc_from_f(char *filename, struct doc **doc);
extern int split_doc(struct doc *doc, int n, struct doc_part_list **list);
extern int free_doc(struct doc **doc);
extern int free_doc_part_list(struct doc_part_list **list);
extern int test_doc();

/*************************
 * For count one document
 *
 *************************/

struct count{
    char *name;
    char whole;  // whether this count is for whole document.
    int term_count;
    struct count_term_node *list;
};

struct count_term_node{
    char *term;
    int count;
    struct count_term_node *pre;
    struct count_term_node *next;
};

// str1 == str2 : 0
// str1 > str2  : 1
// str1 < str2  : -1
extern int compare_string(char *str1, char *str2);
extern int get_count_term(struct count *count, char *term);
extern int add_count_term(struct count *count, char *term, int n);

// get the count info of one  doc
extern int count_doc_part(struct doc_part part, struct count **count);
extern int count_doc(struct doc *doc, struct count **count);
// combine two count
extern int combine_count(struct count *main, struct count *asst);
extern int create_count_term_node(struct count_term_node **node, char *term, int n);
extern int create_count(struct count **count, char *name);

extern int free_count(struct count **count);
extern int free_count_term_node(struct count_term_node **term);
extern int test_count();
/*****************************
 * For main index
 *
 *
 *****************************/


struct index{
    int len_list; // the count of terms.
    struct index_term_node *list;
};

struct index_term_node{
    char *term;
    int len_list;
    struct index_doc_node *list;
    struct index_term_node *pre;
    struct index_term_node *next;
};

struct index_doc_node{
    char *name;
    int count;
    struct index_doc_node *pre;
    struct index_doc_node *next;
};

extern int create_index(struct index **index);
extern int create_index_term_node(struct index_term_node **termn, char *term);
extern int create_index_doc_node(struct index_doc_node **docn, char *name, int n);
// insert doc count info to an terms
extern int insert_doc_to_term(struct index_term_node *node, char *doc_name, int n);
extern int add_count_to_index(struct index *index, struct count *count);
extern int free_index(struct index **index);

extern int test_index();
#endif
