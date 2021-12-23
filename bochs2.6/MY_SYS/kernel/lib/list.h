#ifndef _LIST_H
#define _LIST_H
struct list_head {
    struct list_head* next, * prev;
};
typedef struct list_head list_node;
static inline void INIT_LIST_HEAD(struct list_head* list)
{
    list->next = list->prev = list;
}

static inline void __list_add(struct list_head* New, struct list_head* prev, struct list_head* next)
{
    next->prev = New;
    New->next = next;
    New->prev = prev;
    prev->next = New;
}

static inline void __list_del(struct list_head* prev, struct list_head* next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_add_tail(struct list_head* New, struct list_head* head)
{
    __list_add(New, head->prev, head);
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define container_of(ptr, type, member) (type *)((char *)(ptr) - (char *) &((type *)0)->member)

#define __container_of(ptr, sample, member)     (void *)container_of((ptr), typeof(*(sample)), member)
#define list_for_each_entry(pos, head, member)                                  \
    for (pos = __container_of((head)->next, pos, member);               \
         &pos->member != (head);                                                                        \
         pos = __container_of(pos->member.next, pos, member))
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

static inline int list_empty(struct list_head *head){
    return head->next == head;
}


list_node* list_find(struct list_head* head,list_node* elem);
list_node* list_pop(struct list_head* head);
#endif


