#include "list.h"
list_node* list_find(struct list_head* head,list_node* elem){
    struct list_head* p;
    list_for_each(p,head){
        if(p==elem){
            return elem;
        }
    }
    return 0;
}