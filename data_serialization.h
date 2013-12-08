/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Nov 30, 2013
 */

// Provide data serialization services.
#include "data_operation.h"
#ifndef DATA_SERIALIZATION_H_
#define DATA_SERIALIZATION_H_

enum data_type{
    DOC_PART = 0,
    COUNT,
    INDEX
};

// serialize the struct and store in *data
// return the size of *data
extern int dump_doc_part(struct doc_part *part, void **data);
extern int load_doc(struct doc **doc, void *data);

// serialize the struct and store in *data
// return the size of *data
extern int dump_count(struct count *count, void **data);
extern int load_count(struct count **count, void *data);

// serialize the struct and store in *data
// return the size of *data
extern int dump_index(struct index *index, void **data);
extern int load_index(struct index **index, void *data);

extern int test_serialization();
#endif
