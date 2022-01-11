#include "bitmap.h"
/*
struct bitmap{
    char* p;
    size_t byte_len;
};
*/
void set_bit_bm(struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    (bm->p)[byte_idx]|=(1<<(bit_idx%8));
}
void clear_bit_bm(struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    (bm->p)[byte_idx]&=~(1<<(bit_idx%8));
}
int get_bit_bm(const struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    if( (bm->p)[byte_idx] & (1<<(bit_idx%8)) ){
        return 1;
    }
    else{
        return 0;
    }
}
int scan_bm(const struct bitmap* bm,size_t cnt){
    size_t bit_cnt=0;
    int bit_pointer=0;
    //按位进行一遍搜索.
    //Note: get_bit_bm is not efficient so scan_bm is not efficient, but its idea is simple
    for(;bit_cnt<cnt && bit_pointer<((bm->byte_len)*8); bit_pointer++){
        if(get_bit_bm(bm,bit_pointer)==1){
            bit_cnt=0;
        }
        else{
            bit_cnt++;
        }
    }
    
    if(bit_cnt==cnt){
        return bit_pointer-cnt;
    }
    else{
        return -1;
    }
}
