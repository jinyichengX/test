/* 最小化 list_head 定义（替代 elNET 的 el_nlist.h） */
#ifndef EL_NLIST_H
#define EL_NLIST_H

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct list_head {
    struct list_head * next;
    struct list_head * prev;
};

#define LIST_POISON1 ((void *)0x00100100)
#define LIST_POISON2 ((void *)0x00200200)

static inline void __list_add(struct list_head * new_node,
                              struct list_head * prev,
                              struct list_head * next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

static inline void list_add(struct list_head * new_node, struct list_head * head)
{
    __list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_head * new_node, struct list_head * head)
{
    __list_add(new_node, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head * entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (struct list_head *)LIST_POISON1;
    entry->prev = (struct list_head *)LIST_POISON2;
}

static inline void list_del_init(struct list_head * entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = entry;
    entry->prev = entry;
}

static inline int list_empty(const struct list_head * head)
{
    return head->next == head;
}

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#endif
