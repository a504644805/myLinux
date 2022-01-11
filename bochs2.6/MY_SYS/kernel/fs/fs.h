#ifndef FS_H
#define FS_H
#include "global.h"
#include "lock.h"
#include "debug.h"
#include "list.h"
#include "bitmap.h"
#include "memory.h"

struct partition{
    //step1. fdisk
    //step2. get partition list (read disk and init channel,partition,disk.)
    uint32_t start_lba;
    uint32_t sec_sz;

    struct disk* disk;
    list_node list_elem;

    //step3. 构建各分区的fs and save metadata in RAM
    struct super_block* s_b;
    struct bitmap ib;
    struct bitmap db;
    struct list_head inode_list;
};

enum MASTER_SLAVE_FLAG{
    MASTER,SLAVE
};
struct disk{
    struct channel* channel;
    enum MASTER_SLAVE_FLAG master_slave_flag;
    struct partition prim_partition[4];
    struct partition logic_partition[8];//assume miniOS 最多支持8个逻辑分区
};

struct channel{
    struct lock lock;
    struct semaphore disk_done;

    struct disk disks[2];//0:master, 1:slave

    int base_port;//为方便ata_read和write的编写
};

struct super_block{
    uint32_t ib_start_lba;
    uint32_t ib_sects;

    uint32_t db_start_lba;
    uint32_t db_sects;

    uint32_t inode_table_start_lba;
    uint32_t inode_table_sects;

    uint32_t data_start_lba;
    uint32_t data_sects;

    char pad[512-4*8];
}__attribute__((packed));

struct inode{
    //inode读入内存后的管理
    uint32_t cnt;
    list_node inode_elem;

    uint32_t filesz;
    uint32_t lba[12];//一级索引
    uint32_t lba_2;

    
    uint32_t ino;
    //int write_deny;
};


//dir除根目录外并不纳入持久存储
struct dir{
    struct inode* i_p;
};

#define MAX_FILENAME_LEN 10
enum FILE_TYPE{
    UNKNOWN,
    NORMAL,
    DIR
};
struct dir_entry{
    char filename[10];
    uint32_t ino;

    enum FILE_TYPE ftype;//unknown means this entry is empty(invalid)
};

#define MAX_FILE_NUM 1024 //文件系统中最多能有 MAX_FILE_NUM 个文件
#define OBR_SECS 1
#define SUPER_BLOCK_SECTS 1
#define SECT_SIZE 512
const static uint32_t inode_per_sect=SECT_SIZE/(sizeof(struct inode));
const static uint32_t inode_sects=DIVUP(MAX_FILE_NUM,(SECT_SIZE/(sizeof(struct inode))));
const static uint32_t ib_sects=DIVUP(MAX_FILE_NUM/8.0,SECT_SIZE);

void init_ata_struct();
void format_partition(struct partition* parti);
void set_default_parti(struct partition* parti);
void init_fs();

//-----------------inode operation-----------------
struct inode_pos{
    uint32_t sec_no;
    uint32_t byte_offset;
};
void init_inode(struct inode* p);
struct inode* inode_open(struct partition* parti, uint32_t ino);
struct inode_pos locate_inode(struct partition* parti, uint32_t ino);

//-----------------asist function------------------
void format_partition_show_s_b_info(struct partition* parti,struct super_block* s_b);
void set_default_partition_show_parti(struct partition* parti);
void show_init_fs_result(struct partition* parti);

//
/*
The open() system call opens the file specified by pathname.  If the specified file does not exist, it may optionally (if O_CREAT is specified in flags) be created by open(). The return value of open() is a file descriptor */
uint32_t sys_open(const char *pathname, int flags){
    


}









#endif


