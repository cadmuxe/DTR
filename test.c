#include <stdio.h>
#include "data_operation.h"
#include "data_serialization.h"
#include "syn_communication.h"

int main(int argc, char *argv[]){
    int i;

    printf("################### Test Syncommunication #############\n");
    test_syn_communication();
    printf("################### Test Serialization ##################\n");
    test_serialization();
}
