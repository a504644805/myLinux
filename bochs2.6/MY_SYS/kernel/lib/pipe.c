#include "pipe.h"
#include "thread.h"
#include "fs.h"
#include "circular_queue.h"
extern struct file sys_open_file[MAX_SYSTEM_OPEN_FILE];
void sys_pipe(int pipefd[2]){
    struct task_struct* t_s=get_cur_running();
    struct circular_queue* cq=(struct circular_queue*)malloc_page(K,1);
    ASSERT(cq!=NULL);
    init_cq(cq,CQ_USE_EMPTY);
    //打开文件表
    //0
    uint32_t sys_ofile_idx0=get_free_slot_from_system_ofile_table();
    pipefd[0]=get_free_fd_from_process_ofile_table();
    ASSERT(sys_ofile_idx0!=-1 && pipefd[0]!=-1);

    t_s->process_open_file[pipefd[0]]=sys_ofile_idx0;
    
    sys_open_file[sys_ofile_idx0].cnt=1;
    sys_open_file[sys_ofile_idx0].flags=PIPE|O_RDONLY;
    sys_open_file[sys_ofile_idx0].i_p=(void*)cq;
    sys_open_file[sys_ofile_idx0].offset;//
    //1
    uint32_t sys_ofile_idx1=get_free_slot_from_system_ofile_table();
    pipefd[1]=get_free_fd_from_process_ofile_table();
    ASSERT(sys_ofile_idx1!=-1 && pipefd[1]!=-1);
    
    t_s->process_open_file[pipefd[1]]=sys_ofile_idx1;
    
    sys_open_file[sys_ofile_idx1].cnt=1;
    sys_open_file[sys_ofile_idx1].flags=PIPE|O_WRONLY;
    sys_open_file[sys_ofile_idx1].i_p=(void*)cq;
    sys_open_file[sys_ofile_idx1].offset=sys_ofile_idx0;
    sys_open_file[sys_ofile_idx0].offset=sys_ofile_idx1;//
}