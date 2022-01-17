#include "include/lock.h"
#include "interrupt.h"
#include "debug.h"
//we need lock after we introduce multi-thread
/*
struct semaphore{
    int val;
    struct list_head wait_task_list;
};
*/
void P(struct semaphore* semp){
    enum INTR_STATUS s=disable_intr();
    struct task_struct* cur=get_cur_running();
    while(semp->val==0){
        list_add_tail(&(cur->tag_s),&(semp->wait_task_list));
        block(BLOCKED);
    }
    ASSERT((semp->val)>0);
    semp->val--;

    set_intr_status(s);
}


void V(struct semaphore* semp){
    enum INTR_STATUS s=disable_intr();
    semp->val++;
    if(!list_empty(&(semp->wait_task_list))){
        ASSERT(!list_empty(&(semp->wait_task_list)));
        struct task_struct* p=container_of((semp->wait_task_list).next,struct task_struct,tag_s);
        __list_del((p->tag_s).prev,(p->tag_s).next);
        wakeup(p);
    }
    
    set_intr_status(s);
}

extern struct TSS tss;
void block(enum TASK_STATUS status){
    ASSERT(status==BLOCKED||status==HANGING||status==WAITING);
    enum INTR_STATUS s=disable_intr();
    struct task_struct* cur=get_cur_running();
    cur->status=status;
    cur->tag_s;//already finish in P

    struct task_struct* next=schedule();
    next->status=RUNNING;
    __list_del((next->tag_s).prev,(next->tag_s).next);
    next->elapsed_ticks=0;

    //lcr3 and update tss
    uint32_t pd_paddr;
    if(next->pd!=NULL){//uprocess
        pd_paddr=(uint32_t)get_phy_addr(next->pd);
    }
    else{
        pd_paddr=0x100000;
    }
    asm volatile("movl %0,%%cr3"::"a"(pd_paddr));
    tss.esp0=(uint32_t)((void*)next+4*KB);
    tss.ss0=SELECTOR_K_DATA;
    
    //protect environment
    asm volatile("pushfl;push %%ds;push %%es;push %%fs;push %%gs;push %%ss;pusha"::);
    switch_to(cur,next);
    //restore environment   <-- back to here after being wake_up and reschdule
    asm volatile("popa;pop %%ss;pop %%gs;pop %%fs;pop %%es;pop %%ds;popfl"::);

    set_intr_status(s);
}

extern struct list_head ready_list;
void wakeup(struct task_struct* p){
    ASSERT(p->status==BLOCKED||p->status==HANGING||p->status==WAITING);
    enum INTR_STATUS s=disable_intr();
    p->status=READY;
    ASSERT(list_find(&ready_list,&(p->tag_s))==NULL);
    __list_add(&(p->tag_s),&ready_list,ready_list.next);//remove from wait_list is done by V

    set_intr_status(s);
}

/*
struct lock{
    struct semaphore sem;
    struct task_struct* holder;
    int nr;
};
*/

void lock(struct lock* loc){
    enum INTR_STATUS s=disable_intr();
    ASSERT(((loc->holder!=NULL)&&(loc->nr>=1)) || ((loc->holder==NULL)&&(loc->nr==0)));
    struct task_struct* cur=get_cur_running();
    if(loc->holder==cur){
        loc->nr++;
    }
    else{
        P(&(loc->sem));
        loc->holder=cur;
        loc->nr=1;
    }

    set_intr_status(s);
}

void unlock(struct lock* loc){
    enum INTR_STATUS s=disable_intr();
    ASSERT(((loc->holder!=NULL)&&(loc->nr>=1)));
    if(loc->nr>1){
        loc->nr--;
    }
    else{
        V(&(loc->sem));
        loc->holder=NULL;
        loc->nr=0;
    }

    set_intr_status(s);
}

void init_lock(struct lock* loc){
    init_sem(&(loc->sem),1);
    loc->holder=NULL;
    loc->nr=0;
}

void init_sem(struct semaphore* semp,int init_val){
    semp->val=init_val;
    INIT_LIST_HEAD(&(semp->wait_task_list));
}

