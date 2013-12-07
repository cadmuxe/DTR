/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Nov 30, 2013
 */

// This model provide synchronous commnication via sokcet.
#ifndef ASYN_COMMUNICATION_H_
#define ASYN_COMMUNICATION_H_
#define ASYN_COMMUNICATION_LISTENQ 5

int open_listenfd(int port);
int open_clientfd(char *hostname, int port);

int send_data(int socket_fd, void *data, int size);

// receive data and sotre in *data
// *data = null, it will allocate memory for storage
// return the size of data.
int receive_data(int socket_fd, void **data);

#endif
