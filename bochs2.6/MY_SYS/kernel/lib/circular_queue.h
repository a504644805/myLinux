//和keyboard中的circular_queue不同，此循环队列还包含有empty
#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H
#include "lock.h"
#define CIRCULAR_QUEUE_MAXSIZE 100
enum CQ_FLAG{
    CQ_DO_NOT_USE_EMPTY,CQ_USE_EMPTY
};

struct circular_queue{ 
    char buf[CIRCULAR_QUEUE_MAXSIZE];
    int head;
    int tail;
    struct lock lock;
    struct semaphore full;
    struct semaphore empty;
    boolean use_empty;//0 for kbd, 1 for pipe. shouldn't change after init
};
void init_cq(struct circular_queue* q,enum CQ_FLAG use_empty);
char cq_take_one_elem(struct circular_queue* q);
void cq_put_one_elem(struct circular_queue* q,char c);
int cq_is_empty(struct circular_queue* q);
int cq_is_full(struct circular_queue* q);


#endif