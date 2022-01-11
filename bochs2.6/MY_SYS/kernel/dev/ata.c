#include "ata.h"
#include "stdio.h"

//only support 1 channel(ata0) for simplity 
//max sec_cnt the function support is 255
extern struct channel ata0_channel;
void ata_read(size_t sec_cnt,uint32_t lba,struct disk* device,char* buf){
    lock(&(device->channel->lock));
    int base_port=device->channel->base_port;
    //sec_cnt
    ASSERT(sec_cnt>0 && sec_cnt<256);
    outb(sec_cnt,base_port+2);
    //lba
    ASSERT(lba<=0xfffffff);
    outb(lba&0xff,base_port+3);
    outb((lba&0xff00)>>8,base_port+4);
    outb((lba&0xff0000)>>16,base_port+5);
    //device
    uint8_t device_reg_content=(device->master_slave_flag==MASTER)?((0xe0)|((lba&0xff000000)>>24)):((0xf0)|((lba&0xff000000)>>24));
    outb(device_reg_content,base_port+6);
    //commond
    outb(0x20,base_port+7);

    P(&(device->channel->disk_done));
    //wait till ready
    //threshold is 10ms*10000=100s
    for(int i=0;i<=10000;i++){
        ASSERT(i!=10000);//time is up
        if(inb(base_port+7)&0x08){
            break;
        }
        else{
            thread_sleep(10);
        }
    }

    //data
    rep_insw(base_port+0,buf,(sec_cnt*512/2));

    unlock(&(device->channel->lock));
}

void ata_write(size_t sec_cnt,uint32_t lba,struct disk* device,char* buf){
    lock(&(device->channel->lock));
    int base_port=device->channel->base_port;
    //sec_cnt
    ASSERT(sec_cnt>0 && sec_cnt<256);
    outb(sec_cnt,base_port+2);
    //lba
    ASSERT(lba<=0xfffffff);
    outb(lba&0xff,base_port+3);
    outb((lba&0xff00)>>8,base_port+4);
    outb((lba&0xff0000)>>16,base_port+5);
    //device
    uint8_t device_reg_content=(device->master_slave_flag==MASTER)?((0xe0)|((lba&0xff000000)>>24)):((0xf0)|((lba&0xff000000)>>24));
    outb(device_reg_content,base_port+6);
    //commond
    outb(0x30,base_port+7);

    //threshold is 10ms*100=1s
    for(int i=0;i<=100;i++){
        ASSERT(i!=100);//time is up
        if(inb(base_port+7)&0x08){
            break;
        }
        else{
            thread_sleep(10);
        }
    }
    rep_outsw(base_port+0,buf,(sec_cnt*512/2));

    P(&(device->channel->disk_done));
    //wait till ready
    //threshold is 10ms*100=1s
    for(int i=0;i<=10000;i++){
        ASSERT(i!=10000);//time is up
        if(inb(base_port+7)&0x40){
            break;
        }
        else{
            thread_sleep(10);
        }
    }

    unlock(&(device->channel->lock));
}

//IRQ14
void ata0_intr_handler(){
    V(&(ata0_channel.disk_done));
    inb(ata0_channel.base_port+7);//let ata0 know we've handle the intr so it will continue
}
