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
    root_dir.i_p=inode_open(parti,0);

    default_parti=parti;
    show_init_fs_result(default_parti);
}

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
}

//------------------inode operation--------------------
void init_inode(struct inode* p){
    p->cnt=0;
    p->inode_elem.prev=NULL;
    p->inode_elem.next=NULL;
    
    p->filesz=0;
    for(int i=0;i<12;i++){
        p->lba[i]=0;
    }
    p->lba_2=0;

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
    t_s->pd=NULL;//malloc inode in kernel space so can be shared
    pinode=(struct inode*)sys_malloc(sizeof(struct inode));
    t_s->pd=pd_backup;

    memcpy(pinode,sect+i_p.byte_offset,sizeof(struct inode));
    pinode->cnt=1;
    list_add_tail(&(pinode->inode_elem),&(parti->inode_list));

    return pinode;
}

struct inode_pos locate_inode(struct partition* parti, uint32_t ino){
    struct inode_pos i_p;
    i_p.sec_no=parti->s_b->inode_table_start_lba+ino/inode_per_sect;
    i_p.byte_offset=ino%inode_per_sect*sizeof(struct inode);
    return i_p;
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