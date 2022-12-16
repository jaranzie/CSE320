#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    /*double* ptr = sf_malloc(sizeof(double));*/

    /*
     * void sf_show_block(sf_block *bp);
    void sf_show_blocks();
    void sf_show_free_list(int index);
    void sf_show_free_lists();
    void sf_show_heap();

     *
     *
     * */
    /*sf_show_heap();
    sf_show_blocks();

    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);*/

    sf_errno = 0;
    // We want to allocate up to exactly four pages.

    /*int* y = sf_malloc(1);
    sf_show_blocks();
    *y = 1;*/
    sf_errno = 0;
    int align = 64;
    void* y = sf_malloc(2);
    void *x = sf_memalign(64, align);





    sf_show_blocks();
    sf_free(x);
    sf_free(y);



    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
