#ifndef BITMAP_H
#define BITMAP_H

#include "global.h"
struct bitmap{
    char* p;
    size_t byte_len;
};
void set_bit_bm(struct bitmap* bm,int bit_idx);
void clear_bit_bm(struct bitmap* bm,int bit_idx);
int get_bit_bm(const struct bitmap* bm,int bit_idx);
int scan_bm(const struct bitmap* bm,size_t cnt);

#endif