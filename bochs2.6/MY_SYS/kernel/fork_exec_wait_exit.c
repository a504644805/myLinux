#include "list.h"
#include "stdio.h"
#include "lib/print.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "include/global.h"
#include "include/interrupt.h"
#include "include/memory.h"
#include "include/thread.h"
#include "include/lock.h"
#include "include/syscall.h"
#include "ata.h"
#include "fs.h"
#include "fork_exec_wait_exit.h"
//-1, 0, child_pid
extern void exit_intr();
extern struct list_head ready_list;
extern struct list_head all_task_list;
extern struct file sys_open_file[MAX_SYSTEM_OPEN_FILE];
int sys_fork(){
    struct task_struct* child_t_s=(struct task_struct*)malloc_page(K,1);
    ASSERT(child_t_s!=NULL);
    struct task_struct* parent_t_s=get_cur_running();
    ASSERT(parent_t_s->pd!=NULL);

    //child_t_s's esp and 寄存器环境(借助kernel stack的intr_s来恢复环境)
    /*
      | ABI
      | eip
      v intr_s
    */
    void* p_intr_s=(void*)child_t_s+PG_SIZE-sizeof(struct intr_s);
    void* p_eip=p_intr_s-1*4;
    child_t_s->esp=p_intr_s-5*4;
    memcpy(p_intr_s,(((void*)parent_t_s)+PG_SIZE-sizeof(struct intr_s)),sizeof(struct intr_s));
    *(uint32_t*)p_eip=(uint32_t)exit_intr;
    ((struct intr_s*)p_intr_s)->eax=0;

    //
    child_t_s->status=READY;
    list_add_tail(&(child_t_s->tag_s),&ready_list);
    list_add_tail(&(child_t_s->tag_all),&all_task_list);
    child_t_s->prio=parent_t_s->prio;
    child_t_s->ticks=parent_t_s->ticks;
    child_t_s->elapsed_ticks=0;

    //pd,uvpool,cluster
    void* k_buf=malloc_page(K,1);
    child_t_s->pd=prepare_pd();//768-1022,1023
    for (size_t i = 0; i < (parent_t_s->u_vpool.bm.byte_len)*8; i++){
        if(get_bit_bm(&(parent_t_s->u_vpool.bm),i)==1){
            void* vaddr=parent_t_s->u_vpool.s_addr+i*PG_SIZE;
            void* paddr=palloc(U);
            memcpy(k_buf,vaddr,PG_SIZE);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)child_t_s->pd)[1023]));
            build_mapping(vaddr,paddr);
            memcpy(vaddr,k_buf,PG_SIZE);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)parent_t_s->pd)[1023]));
        }
        else{
            continue;
        }
    }
    prepare_u_vpool(&(child_t_s->u_vpool));//bm.p=malloc_page(K, )
    memcpy((void*)child_t_s->u_vpool.bm.p,(void*)parent_t_s->u_vpool.bm.p,DIVUP(parent_t_s->u_vpool.bm.byte_len,PG_SIZE));
    memcpy((void*)child_t_s->u_arena_cluster,(void*)parent_t_s->u_arena_cluster,sizeof(struct arena_cluster)*7);
    for (size_t i = 0; i < CLUSTER_CNT; i++){//把cluster对应双向链表中的内核地址改正确
        if(list_empty(&(parent_t_s->u_arena_cluster[i].lhead))){
            INIT_LIST_HEAD(&(child_t_s->u_arena_cluster[i].lhead));
        }
        else{
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)child_t_s->pd)[1023]));
            (child_t_s->u_arena_cluster[i].lhead).prev->next=&(child_t_s->u_arena_cluster[i].lhead);
            (child_t_s->u_arena_cluster[i].lhead).next->prev=&(child_t_s->u_arena_cluster[i].lhead);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)parent_t_s->pd)[1023]));
        }
    }
    
    //fs
    for (size_t i = 0; i < MAX_PROCESS_OPEN_FILE; i++){
        uint32_t sys_ofile_idx=(child_t_s->process_open_file)[i]=(parent_t_s->process_open_file)[i];
        if(sys_ofile_idx!=-1){
            sys_open_file[sys_ofile_idx].i_p->cnt++;
        }
    }
    child_t_s->cwd_ino=parent_t_s->cwd_ino;

    //
    child_t_s->pid=allocate_pid();
    child_t_s->ppid=parent_t_s->pid;
    child_t_s->stack_overflow_chk=parent_t_s->stack_overflow_chk;

    free_page(k_buf,1);
    return child_t_s->pid;
}