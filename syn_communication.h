/* Author: Koonwah Chen
 * E-mail: cadmuxeATgmail.com
 * Nov 30, 2013
 */

// This model provide synchronous commnication via sokcet.
#ifndef SYN_COMMUNICATION_H_
#define SYN_COMMUNICATION_H_
#define SYN_COMMUNICATION_LISTENQ 5

extern int open_listenfd(int port);
extern int open_clientfd(char *hostname, int port);
extern int open_clientfdi(unsigned int ip, int port);
extern int send_data(int socket_fd, void *data, int size);

// receive data and sotre in *data
// *data = null, it will allocate memory for storage
// return the size of data.
extern int receive_data(int socket_fd, void **data);

extern int test_syn_communication();

#endif
