#include "circular_queue.h"
#include "debug.h"
/*
struct circular_queue{ 
    char buf[CIRCULAR_QUEUE_MAXSIZE];
    int head;
    int tail;
};
*/
//we guarantee cq_take_one_elem and cq_put_one_elem is multi-thread safe
void init_cq(struct circular_queue* q,enum CQ_FLAG use_empty){
    q->head=0;
    q->tail=0;
    init_lock(&(q->lock));
    init_sem(&(q->full),0);
    init_sem(&(q->empty),CIRCULAR_QUEUE_MAXSIZE);
    q->use_empty=use_empty;
}

char cq_take_one_elem(struct circular_queue* q){
    if(q->use_empty==CQ_DO_NOT_USE_EMPTY){
        P(&(q->full));
        lock(&(q->lock));
        ASSERT(!cq_is_empty(q));
        char rt=(q->buf)[q->head];
        q->head=(q->head+1)%CIRCULAR_QUEUE_MAXSIZE;
        unlock(&(q->lock));
        return rt;
    }
    else{
        P(&(q->full));
        lock(&(q->lock));
        ASSERT(!cq_is_empty(q));
        char rt=(q->buf)[q->head];
        q->head=(q->head+1)%CIRCULAR_QUEUE_MAXSIZE;
        V(&(q->empty));
        unlock(&(q->lock));
        return rt;
    }
}

void cq_put_one_elem(struct circular_queue* q,char c){
    if(q->use_empty==CQ_DO_NOT_USE_EMPTY){
        lock(&(q->lock));
        if(cq_is_full(q)){
            //nop
        }
        else{
            q->buf[q->tail]=c;
            q->tail=(q->tail+1)%CIRCULAR_QUEUE_MAXSIZE;
            V(&(q->full));
        }
        unlock(&(q->lock));
    }
    else{
        P(&(q->empty));
        lock(&(q->lock));
        ASSERT(!cq_is_full(q));
        q->buf[q->tail]=c;
        q->tail=(q->tail+1)%CIRCULAR_QUEUE_MAXSIZE;
        V(&(q->full));
        unlock(&(q->lock));
    }
}

int cq_is_empty(struct circular_queue* q){
    return (q->head)==(q->tail);
}

int cq_is_full(struct circular_queue* q){
    return ((q->tail+1)%CIRCULAR_QUEUE_MAXSIZE)==(q->head);
}