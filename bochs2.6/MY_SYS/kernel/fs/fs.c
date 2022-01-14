#include "fs.h"
#include "ata.h"
#include "stdio.h"

//step2
struct channel ata0_channel;//only support 1 channel(ata0) for simplity
struct list_head partition_list;
void init_ata_struct(){
    init_lock(&(ata0_channel.lock));
    init_sem(&(ata0_channel.disk_done),0);
    ata0_channel.base_port=0x1f0;

    ata0_channel.disks[0].channel=&ata0_channel;
    ata0_channel.disks[0].master_slave_flag=MASTER;
    ata0_channel.disks[1].channel=&ata0_channel;
    ata0_channel.disks[1].master_slave_flag=SLAVE;
    /*
    struct init can be done automatically:
    1.rd 0x475 
    2.rd every disk's MBR and EBR so as to initialzie struct disk, partition automatically
    */
    //Here we do init by hand for simplity
    struct disk* slave_disk=&(ata0_channel.disks[1]);//ata0.master_disk 是个裸盘，不去管它
    struct partition* slave_disk_prim1=&((slave_disk->prim_partition)[0]);
    struct partition* slave_disk_prim2=&((slave_disk->prim_partition)[1]);//extend parti so don't add into parti_list
    struct partition* slave_disk_logic1=&((slave_disk->logic_partition)[0]);
    struct partition* slave_disk_logic2=&((slave_disk->logic_partition)[1]);

    slave_disk_prim1->start_lba=2048;
    slave_disk_prim1->sec_sz=28192;
    slave_disk_prim1->disk=slave_disk;

    slave_disk_logic1->start_lba=32288;
    slave_disk_logic1->sec_sz=8032;
    slave_disk_logic1->disk=slave_disk;

    slave_disk_logic2->start_lba=42368;
    slave_disk_logic2->sec_sz=8032;
    slave_disk_logic2->disk=slave_disk;

    INIT_LIST_HEAD(&partition_list);
    list_add_tail(&slave_disk_prim1->list_elem,&partition_list);
    list_add_tail(&slave_disk_logic1->list_elem,&partition_list);
    list_add_tail(&slave_disk_logic2->list_elem,&partition_list);
}


/*
#define MAX_FILE_NUM 1024 //文件系统中最多能有 MAX_FILE_NUM 个文件
#define OBR_SECS 1
#define SUPER_BLOCK_SECTS 1
#define SECT_SIZE 512
const static uint32_t inode_per_sect=SECT_SIZE/(sizeof(struct inode));
const static uint32_t inode_sects=DIVUP(MAX_FILE_NUM,inode_per_sect);
const static uint32_t ib_sects=DIVUP(MAX_FILE_NUM/8.0,SECT_SIZE);
*/
//construct a partition'fs
    //OBR S ib db I data
    //root_dir's ino is 0
void format_partition(struct partition* parti){
    //1db可管理512*8=4096个sect。多分配点db，把不能对应sect的位初始为1即可
    const uint32_t db_sects=DIVUP((parti->sec_sz-OBR_SECS-SUPER_BLOCK_SECTS-ib_sects-inode_sects),4097);
    void* buf=sys_malloc(SECT_SIZE*(inode_sects>db_sects?inode_sects:db_sects));

    struct super_block s_b;
    memset((void*)&s_b,0,SUPER_BLOCK_SECTS*SECT_SIZE);
    s_b.ib_start_lba=parti->start_lba+OBR_SECS+SUPER_BLOCK_SECTS;
    s_b.ib_sects=ib_sects;

    s_b.db_start_lba=s_b.ib_start_lba+s_b.ib_sects;
    s_b.db_sects=db_sects;

    s_b.inode_table_start_lba=s_b.db_start_lba+s_b.db_sects;
    s_b.inode_table_sects=inode_sects;

    s_b.data_start_lba=s_b.inode_table_start_lba+s_b.inode_table_sects;
    s_b.data_sects=parti->sec_sz-OBR_SECS-SUPER_BLOCK_SECTS- s_b.ib_sects - s_b.db_sects - s_b.inode_table_sects;
    ASSERT(s_b.data_start_lba<=parti->start_lba+parti->sec_sz);
    ata_write(SUPER_BLOCK_SECTS,parti->start_lba+OBR_SECS,parti->disk,(char*)&s_b);

    struct bitmap ib;
    ib.p=(char*)buf;//1 0 0 0 ... 0 1 1 1
    ib.byte_len=ib_sects*SECT_SIZE;
    memset(ib.p,0,ib.byte_len);
    set_bit_bm(&ib,0);
    for(int i=MAX_FILE_NUM;i<ib.byte_len*8;i++){
        set_bit_bm(&ib,i);
    }
    ata_write(ib_sects,s_b.ib_start_lba,parti->disk,ib.p);

    struct bitmap db;
    db.p=(char*)buf;//1 0 0 0 ... 0 1 1 1
    db.byte_len=db_sects*SECT_SIZE;
    memset(db.p,0,db.byte_len);
    set_bit_bm(&db,0);
    for(int i=s_b.data_sects;i<db.byte_len*8;i++){
        set_bit_bm(&db,i);
    }
    ata_write(db_sects,s_b.db_start_lba,parti->disk,db.p);

    struct inode* root_dir_inode=(struct inode*)buf;
    init_inode(root_dir_inode);
    root_dir_inode->filesz=2*sizeof(struct dir_entry);//. ..
    root_dir_inode->lba[0]=s_b.data_start_lba;
    root_dir_inode->ino=0;
    ata_write(1,s_b.inode_table_start_lba,parti->disk,(char*)root_dir_inode);

    struct dir_entry* root_dir_entry=(struct dir_entry*)buf;
    memset(buf,0,SECT_SIZE);
    strcpy(root_dir_entry[0].filename,".");
    root_dir_entry[0].ino=0;
    root_dir_entry[0].ftype=DIR;

    strcpy(root_dir_entry[1].filename,"..");
    root_dir_entry[1].ino=0;
    root_dir_entry[1].ftype=DIR;
    ata_write(1,s_b.data_start_lba,parti->disk,(char*)root_dir_entry);

    sys_free(buf);
}

struct partition* default_parti;
struct dir root_dir;//uniform fs tree
void set_default_parti(struct partition* parti){
    parti->s_b=(struct super_block*)sys_malloc(SUPER_BLOCK_SECTS*SECT_SIZE);
    ata_read(SUPER_BLOCK_SECTS,parti->start_lba+OBR_SECS,parti->disk,(char*)(parti->s_b));
    
    parti->ib.p=(char*)sys_malloc(parti->s_b->ib_sects*SECT_SIZE);
    parti->ib.byte_len=parti->s_b->ib_sects*SECT_SIZE;
    ata_read(parti->s_b->ib_sects,parti->s_b->ib_start_lba,parti->disk,parti->ib.p);

    parti->db.p=(char*)sys_malloc(parti->s_b->db_sects*SECT_SIZE);
    parti->db.byte_len=parti->s_b->db_sects*SECT_SIZE;
    ata_read(parti->s_b->db_sects,parti->s_b->db_start_lba,parti->disk,parti->db.p);

    INIT_LIST_HEAD(&(parti->inode_list));
    root_dir;
    
    default_parti=parti;
}

extern struct file sys_open_file[MAX_SYSTEM_OPEN_FILE];
void init_fs(){
    //step1. fdisk
    //step2. get partition list (read disk and init channel,partition,disk.)
    init_ata_struct();
    void* p = sys_malloc(512*8);

    //step3. 构建各分区的fs and save metadata in RAM
    struct partition* parti=NULL;
    list_for_each_entry(parti,&partition_list,list_elem){
        format_partition(parti);
    }
    set_default_parti(container_of(partition_list.next,struct partition,list_elem));

    for (size_t i = 0; i < MAX_SYSTEM_OPEN_FILE; i++)
        sys_open_file[i].i_p=NULL;

    show_init_fs_result(default_parti);
}

//------------------inode operation--------------------
void init_inode(struct inode* p){
    p->cnt=0;
    p->inode_elem.prev=NULL;
    p->inode_elem.next=NULL;
    
    p->filesz=0;
    for(int i=0;i<12;i++){
        p->lba[i]=-1;
    }
    p->lba_2=-1;

    p->ino=0;
}

struct inode* inode_open(struct partition* parti, uint32_t ino){
    struct inode* pinode=NULL;
    list_for_each_entry(pinode,&(parti->inode_list),inode_elem){
        if(pinode->ino==ino){
            pinode->cnt++;
            return pinode;
        }
    }
    //not found in parti.inode_list
    struct inode_pos i_p=locate_inode(parti,ino);
    void* sect=sys_malloc(SECT_SIZE);
    ASSERT(sect!=NULL);
    ata_read(1,i_p.sec_no,parti->disk,sect);

    struct task_struct* t_s=get_cur_running();
    void* pd_backup=t_s->pd;
    t_s->pd=NULL;//保证在内核空间创建 so can be shared
    pinode=(struct inode*)sys_malloc(sizeof(struct inode));
    t_s->pd=pd_backup;

    memcpy(pinode,sect+i_p.byte_offset,sizeof(struct inode));
    pinode->cnt=1;
    list_add_tail(&(pinode->inode_elem),&(parti->inode_list));

    return pinode;
}

void inode_close(struct inode* inode, struct partition* parti){
    ASSERT(list_find(&(parti->inode_list),&(inode->inode_elem)) && inode->cnt>=1);
    inode->cnt--;
    if(inode->cnt==0){
        __list_del(inode->inode_elem.prev,inode->inode_elem.next);

        struct task_struct* t_s=get_cur_running();
        void* pd_backup=t_s->pd;
        t_s->pd=NULL;//保证在内核空间释放
        free(inode);
        t_s->pd=pd_backup;
    }
}

struct inode_pos locate_inode(struct partition* parti, uint32_t ino){
    struct inode_pos i_p;
    i_p.sec_no=parti->s_b->inode_table_start_lba+ino/inode_per_sect;
    i_p.inode_offset=ino%inode_per_sect;
    i_p.byte_offset=i_p.inode_offset*sizeof(struct inode);
    return i_p;
}

//将inode写入磁盘中的对应位置（需先读出sect，修改sect，再写回sect
void sync_inode(struct inode inode,struct partition* parti){
    inode.cnt=0;
    inode.inode_elem.next=inode.inode_elem.prev=NULL;

    struct inode_pos i_p=locate_inode(parti,inode.ino);
    struct inode* inode_array=(struct inode*)sys_malloc(SECT_SIZE);
    ata_read(1,i_p.sec_no,parti->disk,(void*)inode_array);
    memcpy((void*)(&(inode_array[i_p.inode_offset])),(void*)(&inode),sizeof(struct inode));
    ata_write(1,i_p.sec_no,parti->disk,(void*)inode_array);
    sys_free((void*)inode_array);
}

//-----------------------asist function-----------------------
void show_init_fs_result(struct partition* parti){
    struct super_block* s_b=parti->s_b;
    printf("\n");
    printf("Default Partition at %d, parti_sect: %d\n",parti->start_lba,parti->sec_sz);
    printf("Default Parti's Super Block Info:\n");
    printf("ib_start_lba: %d, ib_sects: %d\n",s_b->ib_start_lba,s_b->ib_sects);
    printf("db_start_lba: %d, db_sects: %d\n",s_b->db_start_lba,s_b->db_sects);
    printf("inode_table_start_lba: %d, inode_table_sects: %d\n",s_b->inode_table_start_lba,s_b->inode_table_sects);
    printf("data_start_lba: %d, data_sects: %d\n",s_b->data_start_lba,s_b->data_sects);
    
    printf("Default Parti's ram info:\n");
    printf("super_block ram addr: %x\n",(uint32_t)parti->s_b);
    printf("ib ram addr: %x\n",(uint32_t)(parti->ib.p));
    printf("db ram addr: %x\n",(uint32_t)(parti->db.p));
}

//cnt: 显示前cnt个表项
void print_dir(struct dir dir,struct partition* parti,uint32_t cnt){
    ASSERT(cnt<=dir_entry_per_sect);//为简化代码, 暂时只支持显示第一个sect里的dir_entry
    struct dir_entry* dir_entry_array=(struct dir_entry*)sys_malloc(SECT_SIZE);
    ata_read(1,dir.i_p->lba[0],parti->disk,(void*)dir_entry_array);
    printf("dir: \n");
    for (size_t i = 0; i < cnt; i++){
        print_dir_entry(dir_entry_array[i]);
    }
    
    sys_free((void*)dir_entry_array);
}

void print_dir_entry(struct dir_entry dir_entry){
    printf("filename: %s, ino: %d, ftype: %d\n",dir_entry.filename,dir_entry.ino,dir_entry.ftype);
}

void print_inode_list(struct partition* parti){
    struct inode* inode;
    printf("inode list:\n");
    list_for_each_entry(inode,&(parti->inode_list),inode_elem){
        print_inode(inode);
    }
}

//cnt: 要显示前多少个inode
void print_inode_in_disk(struct partition* parti, uint32_t cnt){
    ASSERT(cnt<=inode_per_sect);//为简化代码, 暂时只支持显示第一个inode sect里的inode
    struct inode* inode_array=(struct inode*)sys_malloc(SECT_SIZE);
    ata_read(1,parti->s_b->inode_table_start_lba,parti->disk,(void*)inode_array);
    printf("inode in disk:\n");
    for (size_t i = 0; i < cnt; i++){
        print_inode(&(inode_array[i]));
    }
    sys_free((void*)inode_array);
}


void print_inode(struct inode* inode){
    printf("inode.cnt: %d, inode.filesz: %d, inode.lba[0]: %d, lba[1]: %d, inode.ino: %d\n",\
            inode->cnt,inode->filesz,inode->lba[0],inode->lba[1],inode->ino);
}

//cnt: 要显示前多少位
void print_bm_in_parti(struct partition* parti,uint32_t type,uint32_t cnt){
    switch (type){
    case INODE_BITMAP:
        printf("print parti's ib:\n");
        print_bm(&(parti->ib),cnt);
        break;
    case DATA_BITMAP:
        printf("print parti's db:\n");
        print_bm(&(parti->db),cnt);
        break;
    default:
        ASSERT(1==2);
    }
}
void print_bm(struct bitmap* bm,uint32_t cnt){
    for (size_t i = 0; i < cnt; i++){
        printf("%d ",get_bit_bm(bm,i));
    }
    printf("\n");
}
//--------------------------sys_open---------------------------
/*
The open() system call opens the file specified by pathname.  If the specified file does not exist, it may optionally (if O_CREAT is specified in flags) be created by open(). The return value of open() is a file descriptor */
uint32_t sys_open(const char *path, int flags){
    ASSERT(path[0]=='/');
    char filename[MAX_FILENAME_LEN];
    parse_path_lastname(path,filename);
    struct dir_entry dir_entry;
    
    if(!search_file_with_path(path,&dir_entry)){
        if(flags&O_CREAT){
            //创建文件: ib,I,d--dir
            int ib_idx=scan_bm(&(default_parti->ib),1);
            if(ib_idx==-1){
                ASSERT(1==2);
            }
            set_bit_bm(&(default_parti->ib),ib_idx);
            sync_bm(default_parti,ib_idx,INODE_BITMAP);

            struct inode inode;
            init_inode(&inode);
            inode.ino=ib_idx;
            sync_inode(inode,default_parti);

            struct dir dir=dir_open(dir_entry.ino,default_parti);//dir_entry现在是dir2在dir1的dir_entry
            struct dir_entry dir_entry_of_new_file={"",ib_idx,NORMAL};
            strcpy(dir_entry_of_new_file.filename,filename);
            dir_add_dir_entry_and_sync(&dir,&dir_entry_of_new_file,default_parti);
            dir_close(dir,default_parti);
        }
        else{
            ASSERT(1==2);
        }
    }
    else{
    }
    //inode_open, 打开文件表
    ASSERT(search_file_with_path(path,&dir_entry));
    struct inode* i_p=inode_open(default_parti,dir_entry.ino);
    uint32_t sys_ofile_idx=get_free_slot_from_system_ofile_table();
    uint32_t fd=get_free_fd_from_process_ofile_table();
    if(sys_ofile_idx==-1 || fd==-1){
        ASSERT(1==2);//don't come here as we don't have rollback code
    }
    struct task_struct* ts=get_cur_running();
    ts->process_open_file[fd]=sys_ofile_idx;
    sys_open_file[sys_ofile_idx].flags=flags;
    sys_open_file[sys_ofile_idx].i_p=i_p;
    sys_open_file[sys_ofile_idx].offset=0;

    return fd;
}

/*
/dir1/dir2/normal
rt 1, 找到对应的NORMAL file, 并将其对应的dir_entry放入形参dir_entry中
rt 0, 没找到且找至最后一个目录，此时dir_entry为dir2在dir1中对应的dir_entry
	  其他情况暂不考虑（找生路） */
bool search_file_with_path(const char* path,struct dir_entry* dir_entry){//无需parti作形参because of uniform fs tree
    ASSERT(path[0]=='/');
    char stored_name[MAX_FILENAME_LEN];
    uint32_t path_depth=parse_path_depth(path);
    
    struct dir cur_dir=dir_open(0,default_parti);//open root_dir
    strcpy(dir_entry->filename,"/");
    dir_entry->ino=0;
    dir_entry->ftype=DIR;

    for(int i=1; i<=path_depth; i++){
        path=parse_path(path,stored_name);
        if(!search_file_in_dir(stored_name,&cur_dir,default_parti,dir_entry)){
            if(i==path_depth){
                dir_close(cur_dir,default_parti);
                return 0;//
            }
            else{
                ASSERT(1==2);
            }
        }
        else{
            if(dir_entry->ftype==DIR && i<=path_depth-1){
                dir_close(cur_dir,default_parti);
                dir_open(dir_entry->ino,default_parti);//
                continue;
            }
            else if(dir_entry->ftype==NORMAL && i==path_depth){
                dir_close(cur_dir,default_parti);
                return 1;//
            }
            else{
                ASSERT(1==2);
            }
        }
    }

    ASSERT(1==2);
}


//非递归搜索. 遍历140个盘块直到成功或失败
//若未找到，rt 0，rt_dir_entry内容不变
bool search_file_in_dir(char* filename,struct dir* dir,struct partition* parti,struct dir_entry* rt_dir_entry){
    struct dir_entry* dir_entry_array=(struct dir_entry*)sys_malloc(SECT_SIZE);
    ASSERT(dir_entry_array!=NULL);

    //先找12个一级索引
    for (size_t i = 0; i < LEVEL1_REFERENCE; i++){
        if(dir->i_p->lba[i]==-1){
            continue;
        }
        else{
            ata_read(1,dir->i_p->lba[i],parti->disk,(char*)dir_entry_array);
            for (size_t j = 0; j < dir_entry_per_sect; j++){
                if(dir_entry_array[j].ftype!=UNKNOWN && strcmp(dir_entry_array[j].filename,filename)==0){
                    memcpy((void*)rt_dir_entry,(void*)(&(dir_entry_array[j])),sizeof(struct dir_entry));
                    sys_free((void*)dir_entry_array);
                    return 1;
                }
                else{
                    continue;
                }
            }
        }
    }
    //二级索引对应的128块
    ASSERT(dir->i_p->lba_2==-1);//累，先假设目录不超过12块
    
    sys_free((void*)dir_entry_array);
    return 0;
}

//若输入/a/b/c，则stored_name为a，rt指向/b/c的指针用以更新path。
//若path==NULL表示读完了。
const char* parse_path(const char* path,char* stored_name){
    ASSERT(*path=='/');
    path++;
    while(*(path) && *(path)!='/'){
        *stored_name=*path;
        stored_name++;
        path++;
    }

    *stored_name=0;
    if(*path==0){
        path=NULL;
    }
    return path;
}

uint32_t parse_path_depth(const char* path){
    uint32_t cnt=0;
    char stored_name[MAX_FILENAME_LEN];
    while (path) {
        cnt++;
        path=parse_path(path, stored_name);
    }
    return cnt;
}

void parse_path_lastname(const char* path,char* lastname){
    char stored_name[MAX_FILENAME_LEN];
    uint32_t depth = parse_path_depth(path);
    for (size_t i = 1; i <= depth-1; i++){
        path=parse_path(path,stored_name);
    }
    parse_path(path,lastname);
}

// input:  dir1/dir2/file
// output: dir1/dir2
void parse_path_dir_path(const char* path,char* dir_path){
    char stored_name[MAX_FILENAME_LEN];
    const char* path_backup=path;
    uint32_t depth = parse_path_depth(path);
    for (size_t i = 1; i <= depth-1; i++){
        path=parse_path(path,stored_name);
    }
    strcpy(dir_path,path);

    // dir1/dir2/file
    //          |
    // path point to here now
    dir_path[(uint32_t)path-(uint32_t)path_backup]=0;
}


/*
enum FS_BM_TYPE{
    INODE_BITMAP,
    DATA_BITMAP
};*/
//将存于RAM中的bm第idx位对应的sect sync到磁盘中去. 该bm是parti中的bm
void sync_bm(struct partition* parti, uint32_t idx, uint32_t type){
    uint32_t sec_off=idx/bits_per_sect;
    uint32_t bytes_off=sec_off*SECT_SIZE;
    switch (type){
    case INODE_BITMAP:
        ata_write(1,parti->s_b->ib_start_lba,parti->disk,parti->ib.p+bytes_off);
        break;
    case DATA_BITMAP:
        ata_write(1,parti->s_b->db_start_lba,parti->disk,parti->db.p+bytes_off);
        break;
    default:
        ASSERT(1==2);
    }
}

//-----------------------dir operation------------------------
//将dir元信息读入
struct dir dir_open(uint32_t ino,struct partition* parti){
    struct inode* inode=inode_open(default_parti,ino);
    return (struct dir){inode};
}
void dir_close(struct dir dir, struct partition* parti){
    if(dir.i_p->ino==0){
        dir.i_p->cnt--;
        return;
    }
    else{
        inode_close(dir.i_p,parti);
    }
}
//找有没有现成的空的entry, 没有现成的但是lba[]还有余量则从磁盘分配一个sect
void dir_add_dir_entry_and_sync(struct dir* dir,struct dir_entry* dir_entry,struct partition* parti){
    ASSERT(!search_file_in_dir(dir_entry->filename,dir,parti,dir_entry));
    //找有没有现成的空的entry
    struct dir_entry* dir_entry_array=(struct dir_entry*)sys_malloc(SECT_SIZE);
    ASSERT(dir_entry_array!=NULL);
    for (size_t i = 0; i < LEVEL1_REFERENCE; i++){//先找12个一级索引
        if(dir->i_p->lba[i]==-1){
            continue;
        }
        else{
            ata_read(1,dir->i_p->lba[i],parti->disk,(char*)dir_entry_array);
            for (size_t j = 0; j < dir_entry_per_sect; j++){
                if(dir_entry_array[j].ftype==UNKNOWN){
                    memcpy((void*)(&(dir_entry_array[j])),(void*)dir_entry,sizeof(struct dir_entry));
                    ata_write(1,dir->i_p->lba[i],parti->disk,(char*)dir_entry_array);
                    sys_free((void*)dir_entry_array);
                    return;
                }
                else{
                    continue;
                }                   
            }
        }
    }
    //没有现成的但是lba[]还有余量则从磁盘分配一个sect
    uint32_t first_empty_slot=0;
    for (; first_empty_slot < LEVEL1_REFERENCE; first_empty_slot++){
        if(dir->i_p->lba[first_empty_slot]==-1){   
            break;
        }
    }
    ASSERT(first_empty_slot!=LEVEL1_REFERENCE);

    uint32_t db_idx=scan_bm(&(parti->db),1);
    ASSERT(db_idx!=-1);
    set_bit_bm(&(parti->db),db_idx);
    sync_bm(parti,db_idx,DATA_BITMAP);
    dir->i_p->lba[first_empty_slot]=parti->s_b->data_start_lba+db_idx;
    dir->i_p->filesz+=sizeof(dir_entry);
    sync_inode(*(dir->i_p),parti);

    memset((void*)dir_entry_array,0,SECT_SIZE);
    memcpy((void*)(&(dir_entry_array[0])),(void*)dir_entry,sizeof(struct dir_entry));
    ata_write(1,dir->i_p->lba[first_empty_slot],parti->disk,(char*)dir_entry_array);

    //累，暂不考虑二级索引
    sys_free((void*)dir_entry_array);
    return;
}
void dir_delete_dir_entry_and_sync(uint32_t dir_ino,char* filename,struct partition* parti){
    //缺点:  1.由于search_file_in_dir未返回查找到的dir_entry的位置信息(第几块，第几个表项)，因此没能复用
    //      2.未考虑目录所占块的回收
    struct dir dir=dir_open(dir_ino,parti);
    ASSERT(dir.i_p->cnt==1);//assert no one else has open the dir
    struct dir_entry* dir_entry_array=(struct dir_entry*)sys_malloc(SECT_SIZE);
    ASSERT(dir_entry_array!=NULL);

    //先找12个一级索引
    for (size_t i = 0; i < LEVEL1_REFERENCE; i++){
        if(dir.i_p->lba[i]==-1){
            continue;
        }
        else{
            ata_read(1,dir.i_p->lba[i],parti->disk,(char*)dir_entry_array);
            for (size_t j = 0; j < dir_entry_per_sect; j++){
                if(dir_entry_array[j].ftype!=UNKNOWN && strcmp(dir_entry_array[j].filename,filename)==0){
                    memset((void*)(&(dir_entry_array[j])),0,sizeof(struct dir_entry));//delete the dir_entry
                    ata_write(1,dir.i_p->lba[i],parti->disk,(char*)dir_entry_array);//sync
                    sys_free((void*)dir_entry_array);
                    return;
                }
                else{
                    continue;
                }
            }
        }
    }
    //二级索引对应的128块
    //累，先假设目录不超过12块
    dir_close(dir,parti);
    ASSERT(1==2);//查找失败
}
//----------------------打开文件表 operation---------------------
struct file sys_open_file[MAX_SYSTEM_OPEN_FILE];
uint32_t get_free_fd_from_process_ofile_table(){
    struct task_struct* t_s=get_cur_running();
    for (size_t i = 3; i < MAX_PROCESS_OPEN_FILE; i++){//skip 0 1 2
        if(t_s->process_open_file[i]==-1){
            return i;
        }
    }
    return -1;
}
uint32_t get_free_slot_from_system_ofile_table(){
    for (size_t i = 3; i < MAX_SYSTEM_OPEN_FILE; i++){//skip 0 1 2
        if(sys_open_file[i].i_p==NULL){
            return i;
        }
    }
    return -1;
}

//----------------------file operation(再加上上面的sys_open)----------------------
void sys_close(uint32_t fd){
    ASSERT(fd>=3);
    struct task_struct* t_s=get_cur_running();
    ASSERT(t_s->process_open_file[fd]!=-1);
    uint32_t sys_ofile_idx=t_s->process_open_file[fd];
    ASSERT(sys_open_file[sys_ofile_idx].i_p!=NULL);
    //打开文件表,inode_close
    struct inode* i_p=sys_open_file[sys_ofile_idx].i_p;
    inode_close(i_p,default_parti);
    sys_open_file[sys_ofile_idx].flags=0;
    sys_open_file[sys_ofile_idx].i_p=NULL;
    sys_open_file[sys_ofile_idx].offset=0;
    t_s->process_open_file[fd]=-1;
}

/*
enum FD_TYPE{
    STD_IN,STD_OUT,STD_ERROUT
};*/
//从offset开始写内容，对于超过混合索引所支持最大容量的情况assert掉, 若成功返回的一定是count
int sys_write_new(int fd, const void *buf, size_t count){//用于取代syscall.c的sys_write，修改调用了sys_write地方
    if(count==0) return 0;
    ASSERT(fd>=0 && fd!=STD_IN && fd!=STD_ERROUT);
    if(fd==STD_OUT){
        ASSERT(strlen((char*)buf)<=count);
        char* p=(char*)sys_malloc(count);
        ASSERT(p!=NULL);
        memcpy((void*)p,(void*)buf,count);
        put_str(p);//
        sys_free(p);
    }
    else{
        uint32_t sys_ofile_idx,offset;
        struct inode* i_p;
        struct task_struct* t_s=get_cur_running();
        ASSERT(t_s->process_open_file[fd]!=-1);
        sys_ofile_idx=t_s->process_open_file[fd];
        ASSERT(sys_open_file[sys_ofile_idx].i_p!=NULL);
        ASSERT((sys_open_file[sys_ofile_idx].flags&O_RDWR) || (sys_open_file[sys_ofile_idx].flags&O_WRONLY));
        i_p=sys_open_file[sys_ofile_idx].i_p;
        offset=sys_open_file[sys_ofile_idx].offset;
        ASSERT(offset+count<=LEVEL1_REFERENCE*SECT_SIZE);//暂不考虑二级索引
    
        /*
        lba[]的盘块号不一定连续，但有数据的情况是连续的:
        lba[0]=123  [1]=42  [2]=311  [3]=-1  [4]=-1
        有数据........................无数据........
        */
        const uint32_t total_sect=DIVUP((offset+count),SECT_SIZE);
        const uint32_t already_allocated_sect_num=DIVUP((i_p->filesz),SECT_SIZE);
        uint32_t first_negtive_1=0;
        for (;first_negtive_1 < LEVEL1_REFERENCE; first_negtive_1++)
            if(i_p->lba[first_negtive_1]==-1)
                break;
        ASSERT(first_negtive_1==already_allocated_sect_num);

        const uint32_t malloc_sect=total_sect-offset/SECT_SIZE;
        const uint32_t s_idx=offset/SECT_SIZE;//读出来，改好，写回去
        const uint32_t e_idx=s_idx+malloc_sect-1;
        //db
        if(total_sect>already_allocated_sect_num){
            for (size_t i = first_negtive_1; i < total_sect; i++){
                uint32_t db_idx=scan_bm(&(default_parti->db),1);
                ASSERT(db_idx!=-1);
                set_bit_bm(&(default_parti->db),db_idx);
                sync_bm(default_parti,db_idx,DATA_BITMAP);
                i_p->lba[i]=db_idx+default_parti->s_b->data_start_lba;
            }
        }
        //D 核心内容
        void* _buf_=sys_malloc(malloc_sect*SECT_SIZE);
        ASSERT(_buf_!=NULL);
        ata_read(1,i_p->lba[s_idx],default_parti->disk,_buf_);
        ata_read(1,i_p->lba[e_idx],default_parti->disk,_buf_+(e_idx-s_idx)*SECT_SIZE);
        memcpy(_buf_+offset%SECT_SIZE,buf,count);
        for (size_t i = 0; i < malloc_sect; i++){
            ata_write(1,i_p->lba[s_idx+i],default_parti->disk,_buf_+i*SECT_SIZE);
        }
        sys_free(_buf_);

        //打开文件表 I
        sys_open_file[sys_ofile_idx].offset+=count;

        i_p->filesz=((offset+count)>(i_p->filesz))?(offset+count):(i_p->filesz);
        i_p->lba;//已在上面更新
        sync_inode(*i_p,default_parti);
    }

    return count;
}

//若已读完(offset==filesz)则返回-1
//若剩余可读字节数＜count则读完并返回所读字节数
int sys_read(int fd, void *buf, size_t count){
    ASSERT(fd>=3);
    uint32_t sys_ofile_idx,offset;
    struct inode* i_p;
    struct task_struct* t_s=get_cur_running();
    ASSERT(t_s->process_open_file[fd]!=-1);
    sys_ofile_idx=t_s->process_open_file[fd];
    ASSERT(sys_open_file[sys_ofile_idx].i_p!=NULL);
    ASSERT(!(sys_open_file[sys_ofile_idx].flags&O_WRONLY));
    i_p=sys_open_file[sys_ofile_idx].i_p;
    offset=sys_open_file[sys_ofile_idx].offset;
    ASSERT(offset<=i_p->filesz);
    
    if(offset==i_p->filesz){
        return EOF;
    }
    else{
        if((offset+count)>(i_p->filesz)){
            count=i_p->filesz-offset;
        }
        else{
        }
    }
    
    /*
    lba[]的盘块号不一定连续，但有数据的情况是连续的:
    lba[0]=123  [1]=42  [2]=311  [3]=-1  [4]=-1
    有数据........................无数据........
    */
    const uint32_t total_sect=DIVUP((offset+count),SECT_SIZE);
    const uint32_t already_allocated_sect_num=DIVUP((i_p->filesz),SECT_SIZE);
    
    const uint32_t malloc_sect=total_sect-offset/SECT_SIZE;
    const uint32_t s_idx=offset/SECT_SIZE;
    const uint32_t e_idx=s_idx+malloc_sect-1;

    //核心内容
    void* _buf_=sys_malloc(malloc_sect*SECT_SIZE);
    ASSERT(_buf_!=NULL);
    for (size_t i = 0; i < malloc_sect; i++){
        ata_read(1,i_p->lba[s_idx+i],default_parti->disk,_buf_+i*SECT_SIZE);
    }
    memcpy(buf,_buf_+offset%SECT_SIZE,count);
    sys_free(_buf_);

    //打开文件表
    sys_open_file[sys_ofile_idx].offset+=count;

    return count;
}

/*
enum WHENCE_TYPE{
    SEEK_SET,SEEK_CUR,SEEK_END
};*/
// whence: from where
// set sys_ofile's offset and rt the new offset
uint32_t sys_lseek(int fd, int offset, enum WHENCE_TYPE whence){
    ASSERT(fd>=3);
    uint32_t sys_ofile_idx,cur_offset;
    struct inode* i_p;
    struct task_struct* t_s=get_cur_running();
    ASSERT(t_s->process_open_file[fd]!=-1);
    sys_ofile_idx=t_s->process_open_file[fd];
    ASSERT(sys_open_file[sys_ofile_idx].i_p!=NULL);
    i_p=sys_open_file[sys_ofile_idx].i_p;
    cur_offset=sys_open_file[sys_ofile_idx].offset;

    int new_offset;
    switch (whence){
    case SEEK_SET:
        new_offset=offset;
        break;
    case SEEK_CUR:
        new_offset=cur_offset+offset;
        break;
    case SEEK_END:
        new_offset=i_p->filesz+offset;
        break;
    default:
        ASSERT(1==2);
    }
    ASSERT(new_offset>=0 && new_offset<=i_p->filesz)
    sys_open_file[sys_ofile_idx].offset=new_offset;
    return new_offset;
}

//sys_open和sys_unlink只能创和删普通文件，涉及目录有专门操作
//和Linux不同，miniOS的inode中无硬链接数，直接删
// dir1/dir2/file1
void sys_unlink(const char *pathname){
    struct dir_entry dir_entry;
    ASSERT(search_file_with_path(pathname,&dir_entry));
    struct inode* inode=inode_open(default_parti,dir_entry.ino);
    //打开文件表
    ASSERT(inode->cnt==1);//ASSERT no one else has open this file
    //ib
    clear_bit_bm(&(default_parti->ib),inode->ino);
    sync_bm(default_parti,inode->ino,INODE_BITMAP);
    //db
    for (size_t i = 0; i < LEVEL1_REFERENCE; i++){
        if(inode->lba[i]!=-1){
            uint32_t db_idx=(inode->lba[i])-(default_parti->s_b->data_start_lba);
            clear_bit_bm(&(default_parti->db),db_idx);
            sync_bm(default_parti,db_idx,DATA_BITMAP);
        }
        else{
        }
    }
    //D--dir
    char filename[MAX_FILENAME_LEN];
    parse_path_lastname(pathname,filename);
    uint32_t dir_ino=({
        //利用search_file_with_path rt 0的情况获得父目录的ino.  not beautiful though 
        char un_exist_filename[MAX_FILENAME_LEN]="__UNEXIST__";
        char search_path[MAX_PATH_LEN];
        parse_path_dir_path(pathname,search_path);
        strcat(search_path,"/");
        strcat(search_path,un_exist_filename);
        struct dir_entry dir_entry;
        ASSERT(!search_file_with_path(search_path,&dir_entry));
        dir_entry.ino;
    });

    dir_delete_dir_entry_and_sync(dir_ino,filename,default_parti);
    
    inode_close(inode,default_parti);
}

