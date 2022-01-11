#include "include/thread.h"
#include "stdio.h"
/*
struct task_struct{
    void* esp;

    enum{
        RUNNING,READY,WAITING,HANGING
    }status;
    list_node tag_s;//挂载在和status相关的队列上
    list_node tag_all;
    int prio;
    int ticks;//==prio in our policy
    int elapsed_ticks;

    
    void* pdt;
    struct pool u_vpool;
    

   int stack_overflow_chk;
};
*/

struct list_head ready_list;
struct list_head all_task_list;
//#define MAIN_THREAD_TASK_STRUCT 0xc009e000
void make_main_thread(){
    INIT_LIST_HEAD(&ready_list);
    INIT_LIST_HEAD(&all_task_list);

    struct task_struct* p=(struct task_struct*)MAIN_THREAD_TASK_STRUCT;
    p->pid=allocate_pid();
    p->esp;//no need to save esp because main thread is running now, esp is now somewhat below 0xc009f000
    p->status=RUNNING;
    p->tag_s;
    __list_add(&(p->tag_all),&all_task_list,&all_task_list);
    p->prio=31;
    p->ticks=p->prio;
    p->elapsed_ticks=0;
    p->stack_overflow_chk=0x19980211;

}

//在引入用户态前的线程创建函数(彼时尚未有*pd和u_vpool)，彼时线程切换时不需要考虑cr3的变化
void* start_thread(void (*f)(void*), void* f_arg, int prio){
    struct task_struct* pcb=(struct task_struct*)malloc_page(K,1);
    if(pcb==NULL){
        return NULL;
    }
    pcb->pid=allocate_pid();
    pcb->esp=(void*)pcb+4*KB;
    pcb->status=READY;
    list_add_tail(&(pcb->tag_s),&ready_list);
    list_add_tail(&(pcb->tag_all),&all_task_list);
    pcb->prio=prio;
    pcb->ticks=pcb->prio;
    pcb->elapsed_ticks=0;
    pcb->pd=NULL;
    pcb->u_vpool.bm.p=NULL;
    pcb->stack_overflow_chk=0x19980211;

    //prepare the stack for 初次调用
    struct s{
        //prepare for switch_to
        uint32_t ebp;
        uint32_t ebx;
        uint32_t edi;
        uint32_t esi;
        void* f_wrapper;
        //f_wrapper充分信任caller的工作
        uint32_t unused_4B;
        void* f;
        void* f_arg;
    };
    pcb->esp-=sizeof(struct intr_s);//never use: create_process, fork时才会有此操作，此处只是为了统一(习惯了)
    pcb->esp-=sizeof(struct s);
    struct s* p=(struct s*)(pcb->esp);
    p->f_wrapper=start_kthread;
    p->f=f;
    p->f_arg=f_arg;

    return pcb;
}

//创建用户进程
//DIFF x: difference from start_thread
void* create_process(void (*f)(), int prio){
    struct task_struct* pcb=(struct task_struct*)malloc_page(K,1);
    if(pcb==NULL){
        return NULL;
    }
    pcb->pid=allocate_pid();
    pcb->esp=(void*)pcb+4*KB;
    pcb->status=READY;
    list_add_tail(&(pcb->tag_s),&ready_list);
    list_add_tail(&(pcb->tag_all),&all_task_list);
    pcb->prio=prio;
    pcb->ticks=pcb->prio;
    pcb->elapsed_ticks=0;
    pcb->pd=prepare_pd();//DIFF.1
    prepare_u_vpool(&(pcb->u_vpool));
    prepare_u_arena_cluster(pcb->u_arena_cluster);
    pcb->stack_overflow_chk=0x19980211;

    //prepare the stack for 初次调用
    struct s{
        //prepare for switch_to
        uint32_t ebp;
        uint32_t ebx;
        uint32_t edi;
        uint32_t esi;
        void* f_wrapper;
        //f_wrapper充分信任caller的工作
        uint32_t unused_4B;
        void* f;
        void* f_arg;
    };
    pcb->esp-=sizeof(struct intr_s);
    pcb->esp-=sizeof(struct s);
    struct s* p=(struct s*)(pcb->esp);
    p->f_wrapper=start_uprocess;//DIFF.2
    p->f=NULL;//start_uprocess() has no parameter, it's responsible to alloc u_stack and making sure go to exit_intr correctly
    p->f_arg=NULL;
    
    prepare_intr_s((pcb->esp)+sizeof(struct s),f);//DIFF.3
    return pcb;
}

struct TSS tss;
extern uint32_t SYS_ELAPSED_TIME;
void time_intr_handler(){
    SYS_ELAPSED_TIME++;
    struct task_struct* cur=get_cur_running();
    ASSERT((cur->stack_overflow_chk)==0x19980211);
    if(++(cur->elapsed_ticks) == (cur->ticks)){
        struct task_struct* next=schedule();
        if(next==NULL){
            return;
        }
        else{
            cur->esp;//saved via switch_to
            cur->status=READY;
            ASSERT(list_find(&ready_list,&(cur->tag_s))==0);
            list_add_tail(&(cur->tag_s),&ready_list);
            cur->elapsed_ticks=0;

            //restore next->esp  done by switch_to
            next->status=RUNNING;
            __list_del(next->tag_s.prev,next->tag_s.next);
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

            switch_to(cur,next);
            //                  <--back to here when cur is rescheduled. won't back to here if 初次被调度
            
        }
    }
    else{
    }
}

struct task_struct* schedule(){
    if(list_empty(&ready_list)){
        return NULL;
    }
    else{
        return container_of(ready_list.next,struct task_struct,tag_s);
    }
}

struct task_struct* get_cur_running(){
    uint32_t esp;
    asm volatile("mov %%esp,%0":"=a"(esp));
    return (struct task_struct*)(esp&0xfffff000);
}

void start_kthread(void (*f)(void*),void* f_arg){
    enable_intr();//
    f(f_arg);
}

/*
typedef unsigned int pte_t;
typedef unsigned int pde_t;
*/
void* prepare_pd(){
    pde_t* p=malloc_page(K,1);//pde_t p[1024]
    ASSERT(p!=NULL);
    memcpy(&p[768],&(((pde_t*)PAGE_DIR_TABLE_POS)[768]),4*(1022-768+1));
    p[1023]=((unsigned int)(get_phy_addr((void*)p))&0xfffff000)|PTE_U|PTE_RW|PTE_P;
    return p;
}

void prepare_u_vpool(struct pool* p){
    p->bm.byte_len=(0xc0000000-0x8048000)/(4*KB)/8;//luckily 是整除
    p->bm.p=malloc_page(K,((p->bm.byte_len)/(4*KB))+1);//plus 1 because 不是整除
    ASSERT(p!=NULL);
    p->s_addr=(void*)0x8048000;

    p->type=Virtual;
    p->man_sz=0;
}

void prepare_u_arena_cluster(struct arena_cluster* u_arena_cluster){
    size_t block_sz=16;
    for(int i=0;i<CLUSTER_CNT;i++,block_sz*=2){
        u_arena_cluster[i].block_sz=block_sz;
        INIT_LIST_HEAD(&(u_arena_cluster[i].lhead));
        u_arena_cluster[i].block_per_page=(PG_SIZE-sizeof(struct arena))/block_sz;
    }
}

void prepare_intr_s(struct intr_s* p,void(*f)()){
    p->intrNo;
    p->edi;
    p->esi;
    p->ebp;
    p->esp_dummy;//popad will omit esp
    p->ebx;
    p->edx;
    p->ecx;
    p->eax;

    p->gs;//used in print.S as video segment whose DPL is 0.即使我们在此对其进行赋值，iret时CPU也会进行相应检查并置0以期发生错误
    p->fs=\
    p->es=\
    p->ds=DATA3_SELECTOR;

    p->errNo;
    p->eip=(unsigned int)f;
    p->cs=CODE3_SELECTOR;
    p->eflags=EFLAGS_IOPL_0|EFLAGS_IF_1|EFLAGS_MBS_L;
    p->esp3=0xc0000000;//build_mapping shall be done after uprocess's cr3 is loaded, so we alloc ustack  in start_uprocess
    p->ss3=DATA3_SELECTOR;
}

//#define USTACK_PAGE_SIZE 1
void start_uprocess(){
    /*alloc ustack*/
    void* vaddr_start=(void*)(0xc0000000-USTACK_PAGE_SIZE*4*KB);
    int bit_idx=((uint32_t)vaddr_start-0x8048000)/(4*KB);
    struct task_struct* pcb=get_cur_running();
    //code from malloc_page and valloc. not beautiful :(
    for(size_t i=0;i<USTACK_PAGE_SIZE;i++){
        set_bit_bm(&((pcb->u_vpool).bm),bit_idx+i);
    }
    void* vaddr=vaddr_start;
    for (size_t i = 0; i < USTACK_PAGE_SIZE; i++){
        void* paddr=palloc(U);
        if(paddr==NULL){
            /*
                回退
            */
           ASSERT(paddr!=NULL);
        }
        else{
            build_mapping(vaddr,paddr);
            vaddr+=4*KB;// no need to "/4"
        }
    }
    pcb->esp=(void*)pcb+4*KB;
    asm volatile("movl %0,%%esp; jmp exit_intr"::"a"((pcb->esp)-sizeof(struct intr_s)));
}

void update_gdt(){
    unsigned int DESC_DATA3_HIGH4=(0x00<<24)+DESC_G_4KB+DESC_D_32+DESC_L_0+DESC_AVL_0+\
    DESC_LIMIT2_DATA+DESC_P_1+DESC_DPL_3+DESC_S_D+DESC_TYPE_DATA+0x00;
    
    unsigned int DESC_DATA3_LOW4=0x0000ffff;

    unsigned int DESC_CODE3_HIGH4=(0x00<<24)+DESC_G_4KB+DESC_D_32+DESC_L_0+DESC_AVL_0+\
    DESC_LIMIT2_CODE+DESC_P_1+DESC_DPL_3+DESC_S_D+DESC_TYPE_CODE+0x00;

    unsigned int DESC_CODE3_LOW4=0x0000ffff;

    unsigned int tss_addr=(uint32_t)&tss;
    unsigned int tss_sz=sizeof(struct TSS);
    unsigned int DESC_TSS_HIGH4=(tss_addr&0xff000000)+DESC_G_1B+DESC_D_32+DESC_L_0+DESC_AVL_0+\
    DESC_LIMIT2_TSS+DESC_P_1+DESC_DPL_0+DESC_S_S+DESC_TYPE_TSS+((tss_addr&0x00ff0000)>>16);

    unsigned int DESC_TSS_LOW4=((tss_addr&0x0000ffff)<<16)+tss_sz;
    
    /*
    data3
    code3
    tss     <--GDT_BASE+4*8=0xc0000920
    video
    data
    code
    0       <--GDT_BASE=0xc0000900
    */
    memcpy((void*)0xc0000920,&DESC_TSS_LOW4,4);
    memcpy((void*)0xc0000924,&DESC_TSS_HIGH4,4);
    memcpy((void*)0xc0000928,&DESC_CODE3_LOW4,4);
    memcpy((void*)0xc000092c,&DESC_CODE3_HIGH4,4);
    memcpy((void*)0xc0000930,&DESC_DATA3_LOW4,4);
    memcpy((void*)0xc0000934,&DESC_DATA3_HIGH4,4);

    uint64_t t=(((uint64_t)((uint32_t)GDT_BASE_ADDR))<<16)+(uint16_t)(7*8-1);
	asm volatile ("lgdt %0"::"m"(t));
    uint32_t tss_selector=TSS_SELECTOR;
    asm volatile ("ltr %0"::"m"(tss_selector));
}

static uint32_t next_pid=0;
//暂时不用去考虑pid的回收，uint32_t够我们用的了
uint32_t allocate_pid(){
    return next_pid++;
}

void thread_yeild(){
    enum INTR_STATUS s=disable_intr();
    struct task_struct* cur=get_cur_running();
    cur->status=READY;
    ASSERT(list_find(&ready_list,&(cur->tag_s))==0);
    list_add_tail(&(cur->tag_s),&ready_list);

    //same as block
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

//if ms<10ms, thread won't call thread_yeild
void thread_sleep(size_t ms){
    uint32_t t=SYS_ELAPSED_TIME;
    uint32_t tick=OUTPUT_FREQUENCY/1000*ms;
    while(t+tick>SYS_ELAPSED_TIME){
        thread_yeild();
    }
}