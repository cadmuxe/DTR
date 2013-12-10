/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Dec 9, 2013
 */

#ifndef CALCULATION_NODE_H_
#define CALCULATION_NODE_H_

 
struct parameter{
    int socket_fd;
    unsigned int ip;
};

void *work(void *mes);
int cregis(unsigned short port);

int main(int argc, char *argv[]);
#endif
