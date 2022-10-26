/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>

#define SIZE_MASK (0xFFFFFFFFFFFFFFE0)

static void* save_the_trees(sf_header req_size);
static void check_valid_pointer(void *pp);
static int get_sf_start_index(size_t size);
static void* place_block(sf_block *block, sf_header req_size);
static void remove_block_links(sf_block *block);
static void add_block_to_list(sf_block *block);
static void* search_free_list(sf_block *head, size_t size);
static void* split_block(sf_block *block, sf_header req_size);
static void coales(sf_block *block);
static sf_block* get_next_block(sf_block *block);
static sf_block* get_prev_block(sf_block *block);
static sf_footer* get_block_footer(sf_block *block);

void *sf_malloc(size_t size) {
    void* mem_start = sf_mem_start();
    void* mem_end = sf_mem_end();
    if (mem_start == mem_end) { /* First INIT */
        for(int i = 0; i < NUM_FREE_LISTS; i++) {
            sf_free_list_heads[i].body.links.next = sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        }
        mem_start = sf_mem_grow(); /* maybe make sure size good maybe */
        mem_start = mem_start + 24;
        mem_end = sf_mem_end();
        sf_block prologue;
        sf_header new_header = 32 | THIS_BLOCK_ALLOCATED;
        prologue.header = new_header;
        sf_block* temp_ptr = (sf_block*) mem_start;
        *temp_ptr = prologue; /*Allocating prologue*/
        sf_footer *prologue_footer = mem_start + 24;
        *prologue_footer = new_header;
        mem_start = mem_start + 32;
        mem_end = mem_end - sizeof(sf_header);
        new_header = mem_end - mem_start; /* mem_start has prologue added, subtracting sizeof for epilogue (not created yet) */
        sf_block wilderness;
        wilderness.header = new_header;
        wilderness.body.links.next = &sf_free_list_heads[7];
        wilderness.body.links.prev = &sf_free_list_heads[7];
        temp_ptr = (sf_block*) ((size_t)(mem_start));
        *temp_ptr = wilderness;
        *(((sf_footer*)((void*)temp_ptr+new_header-8))) = new_header;
        sf_free_list_heads[7].body.links.next = temp_ptr;
        sf_free_list_heads[7].body.links.prev = temp_ptr;
        sf_header* epilogue = (sf_header*)mem_end;
        *epilogue = 0x0;
    }
    if (sf_free_list_heads[7].body.links.next == &sf_free_list_heads[7]) { /* Wilderness gone, need to make a new one*/
        sf_block *new_wilderness = sf_mem_grow() - 8;
        new_wilderness->header = PAGE_SZ;
        sf_footer *new_epilogue = sf_mem_end() - 8;
        *new_epilogue = 0x0;
        sf_free_list_heads[7].body.links.next = new_wilderness;
        sf_free_list_heads[7].body.links.prev = new_wilderness;
    }
    size_t req_block_size;
    if(size > 16) {
        req_block_size = ((16 + size) + 31) & SIZE_MASK; /* Header footer plus size */
    } else {
        req_block_size = 32; /*2 * sizeof(sf_header) + 2 * sizeof(sf_block*);*/
    }
    for(int i = get_sf_start_index(req_block_size); i < (NUM_FREE_LISTS - 1); i++) {
        void* res_body = search_free_list(&sf_free_list_heads[i], req_block_size);
        if(res_body != NULL) {
            return res_body;
        }
    }

    return save_the_trees(req_block_size);
}

void sf_free(void *pp) {
    if(pp == NULL) {
        abort();
    }
    sf_block *block = (sf_block*) (pp-8);
    if((void*)block < (sf_mem_start()+24)) {
        abort();
    }
    sf_header block_header = block->header;
    sf_footer block_footer = *((sf_footer*) (((void *) block) + (block_header & SIZE_MASK) - 8));
    size_t block_size = block_header & SIZE_MASK;
    if(block_header != block_footer || ((((void *) block) + block_size - 8) > (void*)sf_free_list_heads[7].body.links.next && (sf_free_list_heads[7].body.links.next != &sf_free_list_heads[7]))) {
        abort();
    }
    if((block_header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) {
        abort();
    }

    if(((block_size % 32) != 0) || (block_size < 32)) {
        abort();
    }
    block->header = block_header & SIZE_MASK;
    coales(block);
}

void *sf_realloc(void *pp, size_t rsize) {
    size_t req_block_size = (rsize + 16 + 31) & SIZE_MASK;
    /*sf_header *cur_block_header = pp - 8;*/
    /* Check valid*/
    if(pp == NULL) {
        abort();
    }
    sf_block *block = (sf_block*) (pp-8);
    if((void*)block < (sf_mem_start()+24)) {
        abort();
    }
    sf_header block_header = block->header;
    sf_footer block_footer = *((sf_footer*) (((void *) block) + (block_header & SIZE_MASK) - 8));
    size_t block_size = block_header & SIZE_MASK;
    if(block_header != block_footer || (((void *) block) + block_size - 8) > (void*)sf_free_list_heads[7].body.links.next) {
        abort();
    }
    if((block_header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) {
        abort();
    }
    if(((block_size % 32) != 0) || (block_size < 32)) {
        abort();
    }
    if(rsize == 0) {
        sf_free(pp);
        return NULL;
    } else {
        if(req_block_size == block_size) {
            return pp;
        } else if (req_block_size < block_size){
            if(block_size - req_block_size < 32) {
                return pp;
            } else {

                return split_block((sf_block*) (pp-8), req_block_size) + 8;
            }
        } else {
            void *new_pp = sf_malloc(rsize);
            if (new_pp == NULL) {
                return NULL; /* Maybe do later ?*/
            }
            memcpy(new_pp,pp,block_size-16);
            sf_free(pp);
            return new_pp;
        }
    }
}

void *sf_memalign(size_t size, size_t align) {
    if(align < 32 || ((align & (align - 1)) != 0)) {
        sf_errno = EINVAL;
        return NULL;
    } else {
        void *pp = sf_malloc(align + 32 + 16 + size);
        /*size_t old_block_size = *(((sf_header*)pp-8));*/
        size_t req_block_size = (size + 16 + 31) & SIZE_MASK;
        /*sf_footer *old_block_footer = (pp+old_block_size-8);*/
        if(((size_t)pp & (align-1)) == 0) {
            return split_block((sf_block*) (pp-8),req_block_size)  + 8 ;
        } else {
            size_t mask = ~(align-1);
            void *new_pp = (void*)((((size_t) pp) + align) & mask);

            sf_block *block2_3 = (new_pp-8);
            sf_block *block1 = (pp-8);
            size_t old_size = (void*)get_block_footer(block1)  - (void*) block1 + 8;
            size_t block1_size = (void*) block2_3 - (void*) block1;
            size_t block2_3_size = old_size - block1_size;
            block1->header = block1_size | THIS_BLOCK_ALLOCATED; /* Set block1 size*/
            *get_block_footer(block1) = block1->header;
            block2_3->header = block2_3_size | THIS_BLOCK_ALLOCATED;
            *get_block_footer(block2_3) = block2_3_size;
            sf_free(pp);
            return split_block(block2_3,req_block_size) + 8;
        }
    }
}

/* Size = block size */
static int get_sf_start_index(size_t size) {
    if (size <= 32) {
        return 0;
    } else if (size <= 64) {
        return 1;
    } else if (size <= 96) {
        return 2;
    } else if (size <= 160) {
        return 3;
    } else if (size <= 256) {
        return 4;
    } else if (size <= 416) {
        return 5;
    } else {
        return 6;
    }
}

static void* search_free_list(sf_block *head, size_t req_block_size) {
    sf_block* current_block = head->body.links.next;
    while (current_block != head) {
        sf_header current_block_size = current_block->header & SIZE_MASK;
        if ((current_block_size) >= req_block_size) {
            if(current_block_size - req_block_size >= 32) {
                /* Split block */
                return split_block(current_block,req_block_size);
            }
            else {
                return place_block(current_block, req_block_size);
            }
        } else {
            current_block = current_block->body.links.next;
        }
    }
    return NULL;
}

static void* place_block(sf_block *block, sf_header req_size) {
    block->header |= THIS_BLOCK_ALLOCATED;
    remove_block_links(block);
    return (void*)block->body.payload;
}

static void* split_block(sf_block *block, sf_header req_size) {
    size_t old_size = block->header & SIZE_MASK;
    block->header = req_size | THIS_BLOCK_ALLOCATED;
    /*sf_header new_block_size = req_size - old_size;*/
    sf_header new_block_size = old_size - req_size;
    sf_block new_block;
    new_block.header = new_block_size & SIZE_MASK;
    sf_block *new_block_loc = (((void*)block) + req_size);
    sf_footer* new_footer_loc = (sf_footer *) (((sf_footer) new_block_loc) - 8);
    *new_footer_loc = block->header; /* For first ALlo block */
    *new_block_loc = new_block;
    *(get_block_footer(new_block_loc)) = new_block.header;
    /*add_block_to_list(new_block_loc);*/
    coales(new_block_loc);
    return block;
}

static void remove_block_links(sf_block *block) {
    sf_block *prev = block->body.links.prev;
    sf_block *next = block->body.links.next;
    prev->body.links.next = next;
    next->body.links.prev = prev;
}

static void add_block_to_list(sf_block *block) {
    sf_header block_size = block->header;
    int index = get_sf_start_index(block_size);
    sf_block* prev = &sf_free_list_heads[index];
    sf_block* next = sf_free_list_heads[index].body.links.next;
    block->body.links.next = next;
    next->body.links.prev = block;
    block->body.links.prev = prev;
    prev->body.links.next = block;
}

/* recursive */
static void* save_the_trees(sf_header req_size) {
    sf_block* wilderness = sf_free_list_heads[7].body.links.next;
    sf_header wilderness_size = wilderness->header;
    if(wilderness_size > req_size && wilderness_size - req_size >= 32) {
        size_t old_size = wilderness->header;
        wilderness->header = req_size | THIS_BLOCK_ALLOCATED;
        sf_header new_wilderness_size = old_size - req_size;
        sf_block new_wilderness;
        new_wilderness.header = new_wilderness_size;
        sf_block *new_block_loc = (((void *) wilderness) + req_size);
        sf_footer *new_footer_loc = (sf_footer *) (((void*) new_block_loc) - 8);
        *new_footer_loc = wilderness->header; /* For first ALlo block */
        *new_block_loc = new_wilderness;
        new_footer_loc = (((void *) new_block_loc) + new_wilderness_size - 8);
        *new_footer_loc = new_wilderness_size;
        sf_free_list_heads[7].body.links.next = new_block_loc;
        sf_free_list_heads[7].body.links.prev = new_block_loc;
        new_block_loc->body.links.next = new_block_loc->body.links.prev = &sf_free_list_heads[7];
        return (void*)wilderness->body.payload;
    } else if (req_size == wilderness_size) {
        sf_free_list_heads[7].body.links.next = &sf_free_list_heads[7];
        sf_free_list_heads[7].body.links.prev = &sf_free_list_heads[7];
        wilderness->header |= THIS_BLOCK_ALLOCATED;
        *get_block_footer(wilderness) = wilderness->header;
        return (void*)wilderness->body.payload;
    } else {
            void* end = sf_mem_grow();
            if (end == NULL) {
                sf_errno = ENOMEM;
                return NULL;
            }
            sf_footer *new_epilogue = sf_mem_end() - 8;
            *new_epilogue = 0x0;
            wilderness_size = wilderness_size + PAGE_SZ;
            wilderness->header = wilderness_size;
            sf_footer *new_footer_loc = ((void*) wilderness) + wilderness_size - sizeof(sf_footer);
            *new_footer_loc = wilderness_size;
            return save_the_trees(req_size);
        }
    }



/* Joins prev and next block if possible, returns block*/
static void coales(sf_block *block) {
    sf_block *new_block = block;
    sf_block *next_block = get_next_block(block);
    sf_block *prev_block = get_prev_block(block);
    int index = 0;
    if(next_block != NULL && ((next_block->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)) {
        if (next_block->header == 0x0) {
            index = 7;
        } else {
            if (next_block == sf_free_list_heads[7].body.links.next) {
                index = 7;
            }
            remove_block_links(next_block);
            block->header += (next_block->header & SIZE_MASK);
            sf_footer* block_footer = get_block_footer(block);
            *block_footer = block->header;
            new_block = block;
        }
    }
    if(prev_block != NULL && ((prev_block->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)) {
        remove_block_links(prev_block);
        prev_block->header += (block->header & SIZE_MASK);
        sf_footer* block_footer = get_block_footer(prev_block);
        *block_footer = prev_block->header;
        new_block = prev_block;
    }
    if (!index) {
        index = get_sf_start_index(new_block->header);
    }

    sf_block *next = sf_free_list_heads[index].body.links.next;
    next->body.links.prev = new_block;
    new_block->body.links.next = next;
    new_block->body.links.prev = &sf_free_list_heads[index];
    sf_free_list_heads[index].body.links.next = new_block;
    new_block->header = new_block->header & SIZE_MASK;
    *(get_block_footer(new_block)) = new_block->header;
}

static sf_block* get_next_block(sf_block *block) {
    sf_block *next_block = (sf_block*)(((unsigned long)block->header & SIZE_MASK) + (unsigned long)block);
    return next_block;
}

static sf_block* get_prev_block(sf_block *block) {
    if((((size_t) block) - 8) < (size_t)(sf_mem_start() + 24 + 32)) {
        return NULL;
    }
    size_t prev_block_size = *((sf_footer*)(((unsigned long) block) - 8)) & SIZE_MASK;
    sf_block *prev_block = (sf_block*)((unsigned long)block - prev_block_size);
    return prev_block;
}

static sf_footer* get_block_footer(sf_block *block) {
    sf_footer *block_footer = (((void *) block) + (block->header & SIZE_MASK) - 8);
    return block_footer;
}

static void check_valid_pointer(void *pp) {

}