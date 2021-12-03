#include "headfile/minicrt.h"

/*
the  actual  Linux system  call returns the new program break on success.  On failure, the system call returns the current breaks.
*/
void* brk(void* end){
    //WARNING: use int 0x80 means 32bit system calls. which may occur error if cur_end is above 0xffffffff. I guess:)
    void* ret=0;
    asm volatile("syscall":"=a"(ret):"a"(12),"D"(end));    
    return ret; 
}

static struct list_head lhead;
typedef struct heap_header{
    struct list_head lnode;
    int sz;
    enum{
        USED=0x11223344,
        FREE=0x44332211
    }state;
}heap_header;
#define HEAP_SIZE 1024*1024*32

int init_heap(){
    //sbrk 32MB
    char* _cur_end=brk(0);
    char* cur_end=brk(_cur_end+HEAP_SIZE);    
    if(cur_end==_cur_end){
        //brk fail
        return -1;
    }
    
    INIT_LIST_HEAD(&lhead);
    heap_header* p=(void*)_cur_end;//note this is _cur_end not cur_end -_-
    __list_add(&(p->lnode),&lhead,&lhead);
    p->sz=HEAP_SIZE-sizeof(heap_header);
    p->state=FREE;

    return 0;
}

//FF
void* malloc(int sz){
    heap_header* p;
    list_for_each_entry(p,&lhead,lnode){
        if(p->state==USED){
            continue;
        }
        else{
            if(p->sz > sz+sizeof(heap_header)){
                //split
                heap_header* newp=(heap_header*)((char*)p+sz+sizeof(heap_header));
                __list_add(&newp->lnode,&p->lnode,p->lnode.next);
                
                newp->state=FREE;
                newp->sz=p->sz-sz-sizeof(heap_header);
                p->state=USED;
                p->sz=sz;
                return (char*)p+sizeof(heap_header);
            }
            else if(p->sz >= sz && p->sz <= sz+sizeof(heap_header)){
                p->state=USED;
                return (char*)p+sizeof(heap_header);
            }
            else{
                continue;
            }
        }
    }
    return NULL;
}

//note to merge
void free(void* FreeAddress){
    heap_header* p=(heap_header*)((char*)FreeAddress-sizeof(heap_header));
    if(p->state!=USED){
        return;
    }
    else{
        heap_header* ppre=list_entry(p->lnode.prev,heap_header,lnode);
        heap_header* pnext=list_entry(p->lnode.next,heap_header,lnode);
        if(ppre->state==FREE){
            //merge
            __list_del(&ppre->lnode,&pnext->lnode);
            ppre->sz+=p->sz+sizeof(heap_header);
            p=ppre;//To make sure code about pnext run correctly
        }
        else{
            p->state=FREE;
        }

        if(pnext->state=FREE){
            //merge
            __list_del(&p->lnode,pnext->lnode.next);
            p->sz+=pnext->sz+sizeof(heap_header);
        }
        else{
            //nop
        }
    }
}