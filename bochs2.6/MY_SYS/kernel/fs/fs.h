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

#define LEVEL1_REFERENCE 12
#define LEVEL2_REFERENCE 1
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

//普通文件对应打开文件表
//目录对应struct dir
struct dir{
    struct inode* i_p;
};

#define MAX_FILENAME_LEN 12
#define MAX_PATH_LEN MAX_FILENAME_LEN*4
enum FILE_TYPE{
    UNKNOWN,
    NORMAL,
    DIR
};
struct dir_entry{
    char filename[MAX_FILENAME_LEN];
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
const static uint32_t bits_per_sect=SECT_SIZE*8;
const static uint32_t dir_entry_per_sect=SECT_SIZE/(sizeof(struct dir_entry));

void init_ata_struct();
void format_partition(struct partition* parti);
void set_default_parti(struct partition* parti);
void init_fs();

//-----------------inode operation-----------------
struct inode_pos{
    uint32_t sec_no;
    uint32_t byte_offset;
    uint32_t inode_offset;
};
void init_inode(struct inode* p);
struct inode* inode_open(struct partition* parti, uint32_t ino);
void inode_close(struct inode* inode, struct partition* parti);
struct inode_pos locate_inode(struct partition* parti, uint32_t ino);
void sync_inode(struct inode inode,struct partition* parti);

//-------------------asist function-----------------
void show_init_fs_result(struct partition* parti);
void print_dir(struct dir dir,struct partition* parti,uint32_t cnt);
void sys_print_dir(char* path);
void print_dir_entry(struct dir_entry dir_entry);
void print_inode_list(struct partition* parti);
void print_inode_in_disk(struct partition* parti, uint32_t cnt);
void print_inode(struct inode* inode);
void print_bm_in_parti(struct partition* parti,uint32_t type,uint32_t cnt);
void print_bm(struct bitmap* bm,uint32_t cnt);

//--------------------sys_open----------------------
enum OFLAGS{
    O_RDONLY,O_WRONLY,O_RDWR,O_CREAT = 4
};
uint32_t sys_open(const char *path, int flags);
bool search_file_with_path(const char* path,struct dir_entry* dir_entry);
bool search_file_in_dir(char* filename,struct dir* dir,struct partition* parti,struct dir_entry* rt_dir_entry);

const char* parse_path(const char* path,char* stored_name);
uint32_t parse_path_depth(const char* path);
void parse_path_lastname(const char* path,char* lastname);

enum FS_BM_TYPE{
    INODE_BITMAP,DATA_BITMAP
};
void sync_bm(struct partition* parti, uint32_t idx, uint32_t type);

//------------------dir operation-------------------
struct dir dir_open(uint32_t ino,struct partition* parti);
void dir_close(struct dir dir, struct partition* parti);
void dir_add_dir_entry_and_sync(struct dir* dir,struct dir_entry* dir_entry,struct partition* parti);
void dir_delete_dir_entry_and_sync(struct dir* dir,char* filename,struct partition* parti);

void sys_mkdir(const char *pathname);
void sys_rmdir(const char *pathname);
//getcwd和chdir都是围绕task_struct.cwd_ino实现的
void sys_getcwd(char *buf,size_t size);
void sys_chdir(const char *path);

//---------------打开文件表 operation-----------------
//#define MAX_PROCESS_OPEN_FILE 8 ---defined in thread.h
#define MAX_SYSTEM_OPEN_FILE 32
struct file{
    uint32_t cnt;//not used yet
    uint32_t flags;
    struct inode* i_p;
    uint32_t offset;
};
uint32_t get_free_fd_from_process_ofile_table();
uint32_t get_free_slot_from_system_ofile_table();

//---------file operation(再加上上面的sys_open)----------
#define EOF -1
enum FD_TYPE{
    STD_IN,STD_OUT,STD_ERROUT
};
enum WHENCE_TYPE{
    SEEK_SET,SEEK_CUR,SEEK_END
};
void sys_close(uint32_t fd);
int sys_write_new(int fd, const void *buf, size_t count);
int sys_read(int fd, void *buf, size_t count);
uint32_t sys_lseek(int fd, int offset, enum WHENCE_TYPE whence);
void sys_unlink(const char *pathname);

#endif


