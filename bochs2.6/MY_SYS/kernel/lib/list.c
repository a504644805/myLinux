#include "list.h"
#include "debug.h"
list_node* list_find(struct list_head* head,list_node* elem){
    struct list_head* p;
    list_for_each(p,head){
        if(p==elem){
            return elem;
        }
    }
    return 0;
}

list_node* list_pop(struct list_head* head){
    ASSERT(!list_empty(head));
    list_node* r=head->next;
    __list_del(head,r->next);
    return r;
}