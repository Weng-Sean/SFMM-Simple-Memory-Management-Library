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

#define PRINTLINE print_line(__LINE__)
#define PROLOGUE_SIZE 32
#define EPILOGUE_SIZE 8
#define ALIGN_SIZE 8
#define MIN_BLOCK_SIZE 32

/*
 * This is your implementation of sf_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If size is 0, then NULL is returned without setting sf_errno.
 * If size is nonzero, then if the allocation is successful a pointer to a valid region of
 * memory of the requested size is returned.  If the allocation is not successful, then
 * NULL is returned and sf_errno is set to ENOMEM.
 */

static void insert_block_to_main_free_list(sf_block *block);
static void print_line(int line);
static void *sf_malloc_align(size_t size, size_t align);
static int check_coal(sf_block *block);
static size_t get_block_size(sf_block *block);
static sf_block *get_quick_list_block(int idx);

static void print_block(sf_block *block)
{
    //printf("BLOCK INFORMATION:\n");
    // if (block == NULL){
        //printf("BLOCK IS NULL\n");
    // }
    //printf("Block at address %p:\n", block);
    //printf("Header:%ld\n", block->header);
    // size_t *footer_location = (void *)block + get_block_size(block) - sizeof(sf_footer);
    //printf("Footer:%ld\n", *(footer_location));
}

static void set_footer(sf_block *block)
{
    void *footer_location = (void *)block + get_block_size(block) - sizeof(sf_footer);
    *((sf_footer *)footer_location) = block->header;
}

static size_t get_footer(sf_block *block)
{
    void *footer_location = (void *)block + get_block_size(block) - sizeof(sf_footer);
    return *((sf_footer *)footer_location);
}

static int is_used(sf_block *block)
{
    return block->header & THIS_BLOCK_ALLOCATED;
}

static void combine_blocks(sf_block *s1, sf_block *s2)
{
    s1->header += s2->header / ALIGN_SIZE * ALIGN_SIZE;
    s2->header = 0;
}

static sf_block *get_next_block(sf_block *block)
{
    sf_block *next_block = ((void *)block) + get_block_size(block);
    if ((void *)next_block >= sf_mem_end())
    {
        return NULL;
    }
    return next_block;
}

static sf_block *get_prev_block(sf_block *block)
{
    size_t *prev_footer_location = ((void *)block) - sizeof(sf_footer);
    if ((void *)prev_footer_location < sf_mem_start() + PROLOGUE_SIZE)
    {
        return NULL;
    }
    if ((*prev_footer_location & THIS_BLOCK_ALLOCATED) == 0)
    {
        return block - (*prev_footer_location) / ALIGN_SIZE * ALIGN_SIZE;
    }
    else
    {
        return NULL;
    }
}

static void split_block_and_push_remaining_to_main_free_list(sf_block *block, size_t size)
{
    // TODO
    //PRINTLINE;
    //printf("%ld\n", size);
    print_block(block);
    if (get_block_size(block) >= size + MIN_BLOCK_SIZE)
    {
        sf_block *remain_block = ((void *)block) + size;
        // //printf("%d: %p\n", __LINE__, remain_block);
        remain_block->header = (get_block_size(block) - size) | PREV_BLOCK_ALLOCATED;
        set_footer(remain_block);
        // //printf("%ld\n", remain_block->header);

        // //printf("%d: %ld\n", __LINE__, iter->header / 8 * 8 - size);

        insert_block_to_main_free_list(remain_block);
        int last_three_bits = block->header % 8;
        block->header = size + last_three_bits;
        block->header |= THIS_BLOCK_ALLOCATED;
        set_footer(block);
    }
}

static size_t get_block_size(sf_block *block)
{
    return block->header / ALIGN_SIZE * ALIGN_SIZE;
}

static int get_quick_list_index(size_t size)
{
    if (size < 32)
    {
        return NUM_QUICK_LISTS + 1;
    }
    // //printf("get_quick_list_index(size_t size)\n");
    return (size - 32) / 8;
}

static void insert_block_to_quick_list(sf_block *block)
{
    size_t size = get_block_size(block);

    int idx = get_quick_list_index(size);

    set_footer(block);

    // //printf("idx: %d\n", idx);

    if (idx < NUM_QUICK_LISTS)
    {
        //PRINTLINE;
        //printf("%d\n", sf_quick_lists[idx].length);


        if (sf_quick_lists[idx].length >= QUICK_LIST_MAX)
        {
            

            for (int i = 0; i < QUICK_LIST_MAX; i++)
            {  

                // //PRINTLINE;

                // //PRINTLINE;
                // print_block(iter);
                // //PRINTLINE;
                // //sf_show_heap();

                sf_block *iter = 
                get_quick_list_block(idx);

                //PRINTLINE;
                // //sf_show_heap();

                //PRINTLINE;
                // //sf_show_heap();

                // //PRINTLINE;
                // //printf("%d\n", i);
                // //sf_show_heap();

                //PRINTLINE;
                //printf("%d\n", i);
                // //sf_show_heap();

                insert_block_to_main_free_list(iter);
                // check_coal(iter);

                //PRINTLINE;
                //printf("%d\n", i);
                // //sf_show_heap();
                

                
                


                
                // //sf_show_heap();
            }
        }

        if (sf_quick_lists[idx].first == NULL)
        {
            sf_quick_lists[idx].first = block;
            block->body.links.next = NULL;
            block->body.links.prev = NULL;
            sf_quick_lists[idx].length = 1;
        }
        else
        {
            block->body.links.next = sf_quick_lists[idx].first;
            block->body.links.prev = NULL;
            sf_quick_lists[idx].first->body.links.prev = block;
            sf_quick_lists[idx].first = block;
            sf_quick_lists[idx].length += 1;
        }

        // block->header += 4;
        block->header |= IN_QUICK_LIST;
        block->header |= THIS_BLOCK_ALLOCATED;

        set_footer(block);
        // print_line(__LINE__);

        // print_block(block);
    }
    else
    {
        insert_block_to_main_free_list(block);
        check_coal(block);
    }
}

static void *get_address_without_header(sf_block *block)
{
    return ((void *)block) + sizeof(sf_header);
}

static void print_line(int line)
{
    //printf("%d\n", line);
}
static int get_main_list_index(size_t size)
{
    /*
    0 -> 32
    1 -> (32, 64]
    2 -> (64, 128]
    3 -> (128, 256]
    4 -> (256, 512]
    5 -> (512, 1024]
    6 -> (1024, 2048]
    7 -> (2048, 4096]
    8 -> (4096, 8192]
    9 ->  > 8192
    */
    if (size <= 32)
    {
        return 0;
    }

    if (size > 8192)
    {
        return 9;
    }

    int temp = 64;
    int count = 1;
    while (size > temp)
    {
        count++;
        temp *= 2;
    }
    return count;
}

static int is_initialized = 0;

static sf_block *add_new_page()
{
    // free previous epilogue
    size_t *prev_epilogue = sf_mem_end() - EPILOGUE_SIZE;

    // //printf("before insert\n");
    // visualize_mem();
    if (sf_mem_grow() == NULL)
    {
        //printf("MAX HEAP SIZE!\n");
        return NULL;
    }

    sf_block *block = (void *)prev_epilogue;
    block->header = PAGE_SZ;

    insert_block_to_main_free_list(block);

    // epilogue
    sf_header *epilogue = sf_mem_end() - EPILOGUE_SIZE;
    *epilogue = 1;

    // visualize_mem();

    //printf("add new page: %ld\n", sf_mem_end() - sf_mem_start());
    return block;
}
static void initialize_heads()
{

    if (!is_initialized)
    {
        for (int i = 0; i < NUM_FREE_LISTS; i++)
        {
            sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
            sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        }

        sf_mem_grow();
        // prologue 32
        // epilogue 8
        // page size 4096

        // prologue
        sf_block *prologue = sf_mem_start();
        prologue->header = 33;
        set_footer(prologue);

        // epilogue
        sf_header *epilogue = sf_mem_end() - EPILOGUE_SIZE;
        *epilogue = 1;

        // long int val = *((int *) epilogue);
        // //printf("epilogue: %ld\n", val);

        sf_block *newBlock = sf_mem_start() + 32;
        newBlock->header = (PAGE_SZ - PROLOGUE_SIZE - EPILOGUE_SIZE) | PREV_BLOCK_ALLOCATED;


        insert_block_to_main_free_list(newBlock);

        // 4096 - 32 - 8

        // //printf("pointer: %p\n", sf_mem_start());

        for (int j = 0; j < QUICK_LIST_MAX; j++)
        {
            sf_quick_lists[j].first = NULL;
        }

        is_initialized = 1;
    }
}

static void insert_block_to_main_free_list_helper(sf_block *block);
static void insert_block_to_main_free_list(sf_block *block){
    // //PRINTLINE;
    // //sf_show_heap();
    insert_block_to_main_free_list_helper(block);

    check_coal(block);
}
static void insert_block_to_main_free_list_helper(sf_block *block)
{
    //PRINTLINE;
    //sf_show_heap();
    // //printf("%ld\n", block->header);
    size_t size = get_block_size(block);

    //PRINTLINE;

    //printf("size: %ld\n", size);

    int idx = get_main_list_index(size);
    //PRINTLINE;
    //printf("idx %d\n", idx);

    // add footer
    // // size_t payload_size = newBlock->header & 0xFFFF;
    // void *footer_location = (void*)block + block->header- sizeof(sf_footer);
    // *((sf_footer*)footer_location) = block->header;


    set_footer(block);
    
    sf_block *last_block = sf_free_list_heads[idx].body.links.prev;

    block->body.links.next = &sf_free_list_heads[idx];
    block->body.links.prev = last_block;
    // //PRINTLINE;
    // //sf_show_heap();

    last_block->body.links.next = block;
    sf_free_list_heads[idx].body.links.prev = block;
    //PRINTLINE;

    // check_coal(block);


    // //PRINTLINE;
    // //sf_show_heap();

    //PRINTLINE;
    //sf_show_heap();
}

static int remove_block_from_main_free_lists(sf_block *block)
{
    int size = get_block_size(block);
    int cls_idx = get_main_list_index(size);

    sf_block *iter = sf_free_list_heads[cls_idx].body.links.next;

    while (iter != block && iter != &sf_free_list_heads[cls_idx])
    {
        iter = iter->body.links.next;
    }
    if (iter == block)
    {
        sf_block *prev = iter->body.links.prev;
        sf_block *next = iter->body.links.next;

        // case: remove the only element
        if (prev == next)
        {
            prev->body.links.prev = prev;
            prev->body.links.next = prev;
        }
        else
        {
            prev->body.links.next = iter->body.links.next;
            next->body.links.prev = iter->body.links.prev;
        }
        return 1;
    }
    return 0;
}

static int is_block_in_main_free_lists(sf_block *block)
{
    size_t size = get_block_size(block);
    int cls_idx = get_main_list_index(size);

    sf_block *iter = sf_free_list_heads[cls_idx].body.links.next;

    // main free list is empty
    if (iter == &sf_free_list_heads[cls_idx])
    {
        // //printf("main free list is empty, size: %ld\n", size);
        return false;
    }

    // sf_block * first = iter;

    while (iter != block && iter != &sf_free_list_heads[cls_idx])
    {
        iter = iter->body.links.next;
        // //printf("first: %p\n", first);
        // //printf("%p\n", iter);
    }
    if (iter == block)
    {
        return true;
    }
    return false;
}

static sf_block *get_block_from_main_free_lists(int size)
{
    // //printf("size: %d\n", size);
    int cls_idx = get_main_list_index(size);
    sf_block *iter;

    // visualize_mem();

    print_line(__LINE__);

    while (cls_idx < NUM_FREE_LISTS)
    {
        // //printf("cls_idx: %d\n", cls_idx);
        // //printf("header: %ld\n", sf_free_list_heads[7].body.links.next->header);
        iter = sf_free_list_heads[cls_idx].body.links.next;
        while (iter != &sf_free_list_heads[cls_idx])
        {
            print_line(__LINE__);

            // determine whether the block is big enough to fit
            if (iter->header/8*8 >= size)
            {
                print_line(__LINE__);

                sf_block *prev = iter->body.links.prev;
                sf_block *next = iter->body.links.next;

                // case: remove the only element
                print_line(__LINE__);

                if (prev == next)
                {
                    prev->body.links.prev = prev;
                    prev->body.links.next = prev;
                }
                else
                {
                    prev->body.links.next = iter->body.links.next;
                    next->body.links.prev = iter->body.links.prev;
                }
                // //printf("p1: %p\n", prev);
                // //printf("p2: %p\n", iter);
                // //printf("p3: %p\n", prev->body.links.next);
                // //printf("p4: %p\n", prev->body.links.prev);


                print_line(__LINE__);
                // //sf_show_heap();

                split_block_and_push_remaining_to_main_free_list(iter, size);

                //PRINTLINE;
                // //sf_show_heap();
                print_block(iter);
                return iter;
            }

            // //printf("iter:%p\n", iter);
            // //printf("next:%p\n", iter->body.links.next);
            print_line(__LINE__);

            iter = iter->body.links.next;

            print_line(__LINE__);

        }
        cls_idx++;
    }

    return NULL;
}



void *sf_malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    
    // visualize_mem();

    initialize_heads();

    // visualize_mem();

    return sf_malloc_align(size, ALIGN_SIZE);
}

static sf_block *get_quick_list_block(int quick_list_index)
{
    sf_block *block = sf_quick_lists[quick_list_index].first;

    if (block != NULL){
        sf_quick_lists[quick_list_index].length -= 1;
        sf_quick_lists[quick_list_index].first = block->body.links.next;
        // block-> header -= 4;
        block->header &= ~IN_QUICK_LIST;
        set_footer(block);


    }
    return block;
}
static void *sf_malloc_align(size_t size, size_t align)
{
    // //printf("header: %ld\n", sf_free_list_heads[7].body.links.next->header);
    size_t block_size = size + sizeof(sf_header);
    block_size = block_size % align == 0 ? block_size : (block_size / align + 1) * align;

    if (block_size < MIN_BLOCK_SIZE)
    {
        block_size = MIN_BLOCK_SIZE;
    }
    block_size = block_size % align == 0 ? block_size : (block_size / align + 1) * align;


    //printf("block_size: %ld\n", block_size);

    // if in quick lists
    int quick_list_index = get_quick_list_index(block_size);
    if (quick_list_index < NUM_QUICK_LISTS)
    {
        sf_block *block = get_quick_list_block(quick_list_index);
        print_line(__LINE__);
        // //printf("%d\n", quick_list_index);
        // print_block(block);
        if (block != NULL)
        {
            print_line(__LINE__);
            
            sf_quick_lists[quick_list_index].first = block->body.links.next;
            block->header |= THIS_BLOCK_ALLOCATED;
            //printf("RECEIVED MEMORY FROM QUICK LIST\n");
            get_next_block(block)->header |= PREV_BLOCK_ALLOCATED;
            set_footer(get_next_block(block));

            // visualize_mem();
            return get_address_without_header(block);
        }
    }

    // //printf("header: %ld\n", sf_free_list_heads[7].body.links.next->header);

    // get memory from main free lists

    // //printf("getting memory from main free lists...\n");

    print_line(__LINE__);

    sf_block *block = get_block_from_main_free_lists(block_size);

    print_line(__LINE__);

    if (block != NULL)
    {
        // //printf("block address: %p\n", block);

        //printf("RECEIVED MEMORY MAIN FREE LIST\n");

                // todo
        //PRINTLINE;
        print_block(block);
        get_next_block(block)->header |= PREV_BLOCK_ALLOCATED;
        set_footer(get_next_block(block));
        //PRINTLINE;
        print_block(get_next_block(block));
        //PRINTLINE;

        block->header |= THIS_BLOCK_ALLOCATED;



        return get_address_without_header(block);
    }

    // //printf("header: %ld\n", sf_free_list_heads[7].body.links.next->header);

    // Try to request more memory

    //printf("SIZE BEFORE ADDING NEW PAGE: %ld\n", sf_mem_end() - sf_mem_start());

    sf_block *new_page = add_new_page();
        print_line(__LINE__);

    // visualize_mem();
    if (new_page == NULL)
    {
        // visualize_mem();
        sf_errno = ENOMEM;
        //printf("ERROR\n");
        return NULL;
    }
    else
    {
        //printf("NEW PAGE ADDED!\n");
        //printf("SIZE AFTER ADDING NEW PAGE: %ld\n", sf_mem_end() - sf_mem_start());
        // visualize_mem();
        // check_coal(new_page);
    }

    return sf_malloc_align(size, align);
}

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function sf_malloc.
 *
 * If ptr is invalid, the function calls abort() to exit the program.
 */

void sf_free(void *pp)
{
    if (pp == NULL || 
    pp < sf_mem_start() + PROLOGUE_SIZE + sizeof(sf_header) || 
    pp > sf_mem_end() - MIN_BLOCK_SIZE
    )
    {
        abort();
    }
    sf_block *block = pp - sizeof(sf_header);

    print_block(block);

    if ((
        (block->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) 
    || ((block->header | IN_QUICK_LIST) == IN_QUICK_LIST)
    ||(void*) block + get_block_size(block)> sf_mem_end()
    ){
        //PRINTLINE;
        abort();
    }

    // alloc
    // //PRINTLINE;
    // print_block(block);
    block->header &= ~THIS_BLOCK_ALLOCATED;
    set_footer(block);
    
    // //PRINTLINE;
    // print_block(block);

    get_next_block(block)->header &= (~PREV_BLOCK_ALLOCATED);
    set_footer(get_next_block(block));

    // insert_block_to_main_free_list(block);

    insert_block_to_quick_list(block);


    // visualize_mem();
}

void sf_realloc_free(void *pp)
{
    if (pp == NULL)
    {
        abort();
    }

    sf_block *block = pp;

    // alloc
    block->header &= ~THIS_BLOCK_ALLOCATED;

    // insert_block_to_main_free_list(block);

    insert_block_to_quick_list(block);
    // visualize_mem();
}

/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, the pointer to a valid region of memory is
 * returned, else NULL is returned and sf_errno is set appropriately.
 *
 *   If sf_realloc is called with an invalid pointer sf_errno should be set to EINVAL.
 *   If there is no memory available sf_realloc should set sf_errno to ENOMEM.
 *
 * If sf_realloc is called with a valid pointer and a size of 0 it should free
 * the allocated block and return NULL without setting sf_errno.
 */

void *sf_realloc(void *pp, size_t size)
{
    if (pp == NULL || 
    pp < sf_mem_start() + PROLOGUE_SIZE + sizeof(sf_header) || 
    pp > sf_mem_end() - MIN_BLOCK_SIZE
    )
    {
        abort();
    }
    sf_block *block = pp - sizeof(sf_header);

    print_block(block);

    if ((
        (block->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) 
    || ((block->header | IN_QUICK_LIST) == IN_QUICK_LIST)
    ||(void*) block + get_block_size(block)> sf_mem_end()
    ){
        //PRINTLINE;
        abort();
    }



    if (size < 32)
    {
        size = 32;
    }

    if (abs(size - get_block_size(block)) < 32)
    {
        block->header |= THIS_BLOCK_ALLOCATED;
        return get_address_without_header(block);
    }

    if (size < get_block_size(block))
    {
        split_block_and_push_remaining_to_main_free_list(block, size);
        block->header |= THIS_BLOCK_ALLOCATED;
        // visualize_mem();
        return get_address_without_header(block);
    }
    else
    {
        sf_block *next_block = get_next_block(block);
        size_t size_required = size - get_block_size(next_block);

        if (!is_used(next_block) && (get_next_block(next_block)->header >= size_required))
        {
            remove_block_from_main_free_lists(next_block);
            split_block_and_push_remaining_to_main_free_list(next_block, size_required);
            combine_blocks(block, next_block);
            block->header |= THIS_BLOCK_ALLOCATED;
            return get_address_without_header(block);
        }
        else
        {

            sf_realloc_free(block);
            // visualize_mem();

            block->header |= THIS_BLOCK_ALLOCATED;
            return sf_malloc(size);
        }
    }
}

/*
 * Allocates a block of memory with a specified alignment.
 *
 * @param align The alignment required of the returned pointer.
 * @param size The number of bytes requested to be allocated.
 *
 * @return If align is not a power of two or is less than the default alignment (8),
 * then NULL is returned and sf_errno is set to EINVAL.
 * If size is 0, then NULL is returned without setting sf_errno.
 * Otherwise, if the allocation is successful a pointer to a valid region of memory
 * of the requested size and with the requested alignment is returned.
 * If the allocation is not successful, then NULL is returned and sf_errno is set
 * to ENOMEM.
 */



static int isPowerOfTwo(int n) {
    if (n <= 0) {
        return 0;
    }
    return ((n & (n - 1)) == 0);
}

// TODO


void *sf_memalign(size_t size, size_t align)
{
    if (size == 0){
        return NULL;
    }
    if (!isPowerOfTwo(align) || align < 8)
    {
        sf_errno = EINVAL;
        return NULL;
    }
    initialize_heads();

    // if (size < MIN_BLOCK_SIZE){
    //     size = MIN_BLOCK_SIZE;
    // }
    size_t asize = size + MIN_BLOCK_SIZE + align;
    void* block = sf_malloc(asize) - sizeof(sf_header);
    
    void *ptr = block;
    if ((size_t)(ptr + sizeof(sf_header)) % align != 0){
        ptr += MIN_BLOCK_SIZE;
    }
    while (((size_t)ptr + sizeof(sf_header))% align != 0){
        ptr += ALIGN_SIZE;
    }
    

    size_t malloc_size = ((sf_block*)block)->header / 8  * 8;

    if (ptr - block > MIN_BLOCK_SIZE){

        ((sf_block*)block)->header = ptr - block;
        set_footer((sf_block*)block);
        ((sf_block*) ptr)->header = (malloc_size - ((sf_block*)block)->header);
        set_footer((sf_block*) ptr);

        insert_block_to_main_free_list(block);
    }

    if (size < MIN_BLOCK_SIZE){
        size = MIN_BLOCK_SIZE;
    }

    if (get_block_size((sf_block*) ptr) - size > MIN_BLOCK_SIZE){
        split_block_and_push_remaining_to_main_free_list(block, size);
    }

    
    ((sf_block*) ptr)->header = ((sf_block*) ptr)->header  | THIS_BLOCK_ALLOCATED;

    return ptr + sizeof(sf_header);
}

static int check_coal_next(sf_block *block)
{

    void *next_block_loc = get_next_block(block);

    sf_block *next_block = next_block_loc;
    if ((next_block->header & THIS_BLOCK_ALLOCATED) == 0 && 
    (next_block->header & IN_QUICK_LIST) == 0)
    {
        // next block is available for coalescing

        // visualize_mem();
        //  //printf("%p\n", next_block);
        if (is_block_in_main_free_lists(next_block))
        {
            remove_block_from_main_free_lists(next_block);
            remove_block_from_main_free_lists(block);

            block->header = block->header + next_block->header;
            // //PRINTLINE;
            // //printf("%ld\n", block->header);
            set_footer(block);

            insert_block_to_main_free_list_helper(block);

            return 1;
        }
    }
    return 0;
}


static int check_coal_prev(sf_block *block)
{

    // visualize_mem();

    size_t *prev_footer = ((void *)block) - sizeof(sf_footer);

    // //printf("prev_footer: %ld\n", *prev_footer);

    if (*prev_footer == 0)
    {
        return 0;
    }

    sf_block *prev_block = ((void *)block) - (*prev_footer) / 8 * 8;
    if ((prev_block->header & THIS_BLOCK_ALLOCATED) == 0 &&
    (prev_block->header & IN_QUICK_LIST) == 0)
    {
        // prev block is available for coalescing
        if (is_block_in_main_free_lists(prev_block))
        {
            // //PRINTLINE;
            // print_block(prev_block);
            // print_block(block);
            // visualize_mem();

            // //printf("prev:%p, block: %p\n", prev_block, block);
            remove_block_from_main_free_lists(prev_block);
            // //PRINTLINE;
            //printf("%d\n", remove_prev);
            // visualize_mem();

            remove_block_from_main_free_lists(block);
            prev_block->header = prev_block->header + block->header;
            set_footer(block);

            insert_block_to_main_free_list_helper(prev_block);
            return 1;
        }
    }

    return 0;
}

static int check_coal(sf_block *block)
{



    if (block->header != get_footer(block))
    {
        // //PRINTLINE;
        // print_block(block);
        abort();
    }
    
    int next_coal = check_coal_next(block);

    int prev_coal = check_coal_prev(block);

    

    // if (block->header != get_footer(block)){
    //     //PRINTLINE;
    //     print_block(block);
    //     abort();
    // }


    return next_coal & prev_coal;

    // return 0;
}


