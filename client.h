 /* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Dec 9, 2013
 */
#ifndef CLIENT_H_
#define CLIENT_H_
#include "data_operation.h"

int run();
int req_index(char *path);
int req_retr(struct query *query);
int process(void *data, int len);
int main(int argc, char *argv[]);



#endif
