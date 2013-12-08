#include <stdio.h>
#include "data_operation.h"
#include "data_serialization.h"

int main(int argc, char *argv[]){
    int i;
    printf("################### Test Doc #####################\n");
    test_doc();

    printf("################### Test Count #####################\n");
    test_count();

    printf("################### Test Index #######################\n");
    test_index();

    printf("################### Test Serialization ##################\n");
    test_serialization();
}
