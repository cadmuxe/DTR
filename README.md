####Compile
Compile under Mac shold comment the 

CFLAGS += -pthread

in the makefile

Tested under:

 * Mac 10.9 (gcc : Apple LLVM version 5.0)
 * Ubuntu 12.04 (gcc version 4.6.3)


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

####Structure
List some core functions:

 * server.h/c : The main server
   * dispatch(): the the enterenc funciton, will create an new thread to run the dispatch funciton for each income request
   
   * do_reg() : register an cnode(calculation node)
   
   * do_index(): index a file, it will allocate create N index_agent(each in an new therad, and it will connect to a cnode to finish the work) to do the work. The N which is the number of index_agent and also the number of the calculation nodes are depended on the total cnode and availbe cnode(IDLE cnode)
   
   * index_agent: commnicate with a calculation node to finish a work
   
   * do_retr(): retrieve 1-N terms, it will split the terms in to many grounps, the nubmer depend on hwo many cnode that can be used for this task. Let's say we have 5 cnode for this task, and 5 terms, so each cnode work retreive one term then combine the result.
   
   * retr_agent: simliar to index_agent,  but for retrieving data.
   
 * cnode.h/c : indexing or retrieving data
 
 * client.h.c : the client
 
 *data_operation.h/c : provide the basic data structures and related operations, the main function of indexing, retrieving are done in this part
 
   * doc : contain all the content of a file
   
   * doc_part : contain part of doc, whith generated from split_doc operation
   
   * count : contain the count information of a doc or doc_part
   
   * index : the index structure, contain term, count, doc infor.
   
 * data_serialization.h/c : data serialization, which enable we dump the c structure into binary data, and load them back.
 
 * syn_communication.h/c : helper functions for open socket and send/receive data
 
   * sned_data, receive_data : send/receive binary data