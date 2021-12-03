#ifndef _LIST_H
#define _LIST_H
struct list_head {
    struct list_head* next, * prev;
};
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

#endif


