#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    int blk_num = 5;
    double* ptrs[blk_num];
    for (int i = 0; i < blk_num; i++){
        ptrs[i] = sf_malloc(40);
        // sf_show_heap();
    }

    for (int i = 0; i < blk_num; i++){
        sf_free(ptrs[i]);
        // if (i == blk_num-1){
        //     sf_show_heap();
        // }
    }

    sf_show_heap();

    // double* ptr = sf_malloc(50);

    // printf("finish allocating space for ptr1\n");
    // double* ptr2 = sf_malloc(1000);
    // *ptr = 320320320e-320;
    // *ptr2 = 320320320e-320;

    // sf_free((void*) ptr2);
    // sf_free((void*) ptr);

    // double* ptr3 = sf_malloc(1000);
    // *ptr3 = 320320320e-320;

    // double* ptr4 = sf_memalign(2000, 72);
    // *ptr4 = 320320320e-320;

    // size_t sz_x = sizeof(int), sz_y = 10, sz_x1 = sizeof(int) * 20;
	// void *x = sf_malloc(sz_x);
	// /* void *y = */ sf_malloc(sz_y);
    
	// x = sf_realloc(x, sz_x1);
    // sf_block *bp = (sf_block *)((char *)x - sizeof(sf_header));
    // printf("%ld\n",bp->header & THIS_BLOCK_ALLOCATED);

    return EXIT_SUCCESS;
}


