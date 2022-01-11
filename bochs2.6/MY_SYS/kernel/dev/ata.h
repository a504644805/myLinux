#ifndef ATA_H
#define ATA_H
#include "global.h"
#include "debug.h"
#include "lock.h"
#include "fs.h"

void ata_read(size_t sec_cnt,uint32_t lba,struct disk* device,char* buf);
void ata_write(size_t sec_cnt,uint32_t lba,struct disk* device,char* buf);
void ata0_intr_handler();

#endif