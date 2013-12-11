####Compile
Compile under Mac shold comment the 

CFLAGS += -pthread

in the makefile

####Name Server
Change the default ip and port of name server in name_server_info.h

#####Uage
* server is the main server
* cnode is the calculation node, at least need 1 cnode.
* client
  * Command:
    * index local_path_of_file: indexing
    * search term1 term2 ...  : searching
  * Usage:
    * ./client command parameter : use it directly
    * ./client : enter the shell
