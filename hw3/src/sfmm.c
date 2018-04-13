/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *extendHeap(void* pointer, size_t size){
sf_free_header *tempheader;
tempheader = (sf_free_header *)pointer;
while(size > tempheader->header.block_size<<4){
sf_sbrk();
tempheader->header.block_size = (PAGE_SZ + (tempheader->header.block_size<<4))>>4;
}
return tempheader;
}

void *find_fit(int startlist, size_t size){
    free_list *list = seg_free_list;
    list = list + startlist;
    void *pointer;
    sf_free_header *tempheader;
    while(startlist < FREE_LIST_COUNT){
    pointer = (*list).head;
    tempheader = (sf_free_header *)pointer;
    if(tempheader!= NULL){
    }
    while(tempheader != NULL && tempheader->header.allocated == 0){
            if(tempheader->header.block_size<<4 >= size){
                //debug("PointerFound:%p",tempheader);
                return tempheader;
            }
            if(tempheader->next == NULL){
                //debug("%s","well fuck coalesce or someshit here");
                    return NULL;
            }
            tempheader = tempheader->next;
        }
    list = list + 1;
    startlist ++;
    }
    if(pointer == (void *)-1){
        return NULL;
    }
    else{
    //debug("%s","Oh No no space Coalescing Here?");
        pointer =  sf_sbrk();
        tempheader = (sf_free_header *)pointer;
        tempheader->header.block_size = PAGE_SZ>>4;
            if(size <= PAGE_SZ){
                return tempheader;
            }
            else{
            return extendHeap(pointer, size);
            }
    }
    return NULL;
}

free_list* find_freelist(size_t size){
    if(size >= seg_free_list[0].min && size <= seg_free_list[0].max){
    return seg_free_list;
    }
    else if(size >= seg_free_list[1].min && size  <= seg_free_list[1].max){
    return seg_free_list + 1;
}
    else if(size >= seg_free_list[2].min && size <= seg_free_list[2].max){
    return seg_free_list + 2;
    }
    else{
    return seg_free_list + 3;
    }
}

void place(void* pointer, size_t size, size_t og_size){
    sf_header *newhead;
    sf_footer *newfoot;

    newhead = (sf_header *)pointer;
    size_t Blocksize =newhead->block_size << 4;
    if(Blocksize < 32){
    size += 32;
    Blocksize -= 32;
    }
    newhead->block_size = size>>4;
    newhead->allocated = 1;

    newfoot = (sf_footer *)(pointer + (((*newhead).block_size<<4)) - 8);
    newfoot->block_size = size>>4;
    newfoot->allocated = 1;
    newfoot->requested_size = og_size;
    if(size != newfoot->requested_size  + 16){
    newhead->padded = 1;
    newfoot->padded = 1;
    }

    //sf_blockprint(newhead);
    if((Blocksize - size) >= 32){
    free_list *freelist = find_freelist((Blocksize - size));
    freelist->head = (sf_free_header *)(pointer + size);
    freelist->head->header.block_size = (Blocksize - size) >> 4;
    freelist->head->header.allocated = 0;
    freelist->head->header.padded = 0;
    }
    for(int i = 0;i < 4;i++){
    if(seg_free_list[i].head != NULL && seg_free_list[i].head->header.allocated == 1){
    seg_free_list[i].head = NULL;
    }
    }
    newfoot = (sf_footer *)(get_heap_end() - 8);
    newfoot->block_size = (Blocksize - size) >> 4;
}


void *sf_malloc(size_t size) {
    size_t adjusted_size;
    void *pointer;
    //Check for invalid requests
    if(size == 0 || size > PAGE_SZ << 2){
        sf_errno  = EINVAL;
        return NULL;
    }

    //Get adjusted size accounting for header,footer,padding
    if(size <= 16){
        adjusted_size = 32;
        //debug("Adjusted Size:%lu\n", adjusted_size);
    }
    else{
        adjusted_size = (16 * ((size +31)/16));
        //debug("Adjusted Size:%lu\n",adjusted_size);
    }
    if(adjusted_size > PAGE_SZ << 2){
        sf_errno  = ENOMEM;
        return NULL;
    }
    //Search for correct list
    if(adjusted_size >= seg_free_list[0].min && adjusted_size <= seg_free_list[0].max){
        pointer = find_fit(0, adjusted_size);
        if(pointer == NULL){
            sf_errno  = ENOMEM;
            return NULL;
        }
        place(pointer, adjusted_size,size);
    }
    else if(adjusted_size >= seg_free_list[1].min && adjusted_size <= seg_free_list[1].max){
        pointer = find_fit(1, adjusted_size);
        if(pointer == NULL){
            sf_errno  = ENOMEM;
            return NULL;
        }
        place(pointer, adjusted_size,size);
    }
    else if(adjusted_size >= seg_free_list[2].min && adjusted_size <= seg_free_list[2].max){
        pointer = find_fit(2, adjusted_size);
        if(pointer == NULL){
            sf_errno  = ENOMEM;
            return NULL;
        }
        place(pointer, adjusted_size,size);
    }
    else{
        pointer = find_fit(3, adjusted_size);
        if(pointer == NULL){
            sf_errno  = ENOMEM;
            return NULL;
        }
        place(pointer, adjusted_size,size);
    }
    return pointer+8;
}

int ValidCheck(void *pointer){
    sf_header *header;
    sf_footer *footer;
    size_t size;

    header = (sf_header *) (pointer - 8);
    size = header->block_size << 4;
    footer = (sf_footer *) (pointer + size - 16)
    ;
    if(pointer == NULL){
    return 1;
    }
    if((pointer-8) < get_heap_start() || (pointer-8) > get_heap_end()){
    return 1;
    }
    if(header->allocated == 0 || footer->allocated == 0){
    return 1;
    }
    if(header->allocated != footer->allocated){
    return 1;
    }
    if(header->padded != footer->padded){
    return 1;
    }
    return 0;
}
void *sf_realloc(void *ptr, size_t size) {
    sf_header *header;
    sf_footer *footer;
    size_t Blocksize;
    size_t Requestedsize;
    size_t adjusted_size;

    if(size == 0){
    sf_free(ptr);
    return NULL;
    }
    if(ValidCheck(ptr) == 1){
    abort();
    sf_errno  = EINVAL;
    return NULL;
    }
    header = (sf_header *) (ptr - 8);
    Blocksize = header->block_size << 4;
    footer = (sf_footer *) (ptr + Blocksize - 16);
    Requestedsize = footer->requested_size;
    if(size <= 16){
        adjusted_size = 32;
    }
    else{
        adjusted_size = (16 * ((size +31)/16));
    }
    if(Requestedsize == size){
    return ptr;
    }
    //Reallocated to larger size:
    if(Requestedsize <= size){
    void* pointer = sf_malloc(size);
    if(pointer == NULL){
    return NULL;
    }
    memcpy(pointer,ptr,Requestedsize);
    sf_free(ptr);
    return pointer;
    }
    //Reallocated to smaller size:
    else{
    //Splinter
    if((Requestedsize - adjusted_size) < 32){
    footer->requested_size = size;
    if(adjusted_size != size + 16){
            header->padded = 1;
            footer->padded = 1;
    }
    return ptr;
    }
    else{
    //No Splinter
    header = (sf_header *) (ptr - 8);
    header->block_size = adjusted_size >> 4;
    header->padded = 0;
    header->allocated = 1;
    footer = (sf_footer *)(ptr + (header->block_size<<4) - 16);
    footer->block_size = adjusted_size >> 4;
    footer->requested_size = size;
    footer->allocated = 1;
    if(adjusted_size != size + 16){
            header->padded = 1;
            footer->padded = 1;
    }
    header = (sf_header *) (ptr + adjusted_size - 8);
    header->block_size = (Blocksize - adjusted_size)>>4;
    footer = (sf_footer *)((void *) header + (header->block_size<<4) - 8);
    header->allocated = 1;
    footer->allocated = 1;
    header->padded = 1;
    footer->padded = 1;
    sf_free(header+1);
    return ptr;
    }
    }
	return NULL;
}

void sf_free(void *ptr) {
    sf_header *header;
    sf_footer *footer;
    size_t size;
    sf_header * nextblock;
    sf_free_header * nextfreehead;
    free_list *freelist;

    if(ValidCheck(ptr) == 1){
    abort();
    }

    header = (sf_header *) (ptr - 8);
    size = header->block_size << 4;
    footer = (sf_footer *) (ptr + size - 16);
    nextblock = (sf_header *)(ptr + size - 8);

    if(nextblock->allocated == 0){
    freelist = find_freelist(nextblock->block_size<<4);
    if(freelist->head != NULL){
    if(&freelist->head->header == &*nextblock){
    freelist->head = NULL;
    }
    else{
    sf_free_header *tempheader;
    tempheader = freelist->head;
    while(&tempheader->header != &*nextblock){
    tempheader = freelist->head->next;
    }
    tempheader->prev->next = 0;
    tempheader->prev = 0;
    }
}
    size += nextblock->block_size<<4;
    header->block_size = size >> 4;
    footer = (sf_footer *) (ptr + size - 16);
    footer->block_size = size >> 4;
    }

    header->allocated = 0;
    footer->allocated = 0;
    header->padded = 0;
    footer->padded = 0;
    footer->requested_size = 0;

    if(size > 0){
    freelist = find_freelist(size);
    nextfreehead = (sf_free_header *)(ptr - 8);
    if(freelist->head != NULL){
    freelist->head->prev = nextfreehead;
    }
    nextfreehead->next = freelist->head;
    freelist->head = nextfreehead;
    }
}
