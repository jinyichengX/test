#ifndef IPGUI_WIDGET_TREE_H
#define IPGUI_WIDGET_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"

struct widget_link_t{
    struct widget_link_t * parent;
    struct widget_link_t * sib_next; /* next sibling */
    struct widget_link_t * sib_prev; /* previous sibling */
    struct widget_link_t * first_child;
    int child_num;

    const char * name;/* 测试用 */
};

struct widget_tree_t{
    struct widget_link_t root;
};

typedef int (* widget_ops_t)(struct widget_link_t *, void *);

extern __IPGUI_API__ ipgui_yes_no_t ipgui_widget_link_is_detached(struct widget_link_t * link);
extern __IPGUI_API__ void ipgui_widget_tree_init(struct widget_tree_t * tree);
extern __IPGUI_API__ void ipgui_widget_link_init(struct widget_link_t * link);
extern __IPGUI_API__ ipgui_yes_no_t  ipgui_widget_link_is_root(struct widget_link_t * link);
extern __IPGUI_API__ struct widget_link_t * ipgui_widget_link_get_root(struct widget_link_t * link);
extern __IPGUI_API__ void ipgui_widget_link_detach(struct widget_link_t * link);
extern __IPGUI_API__ void ipgui_widget_link_set_parent(struct widget_link_t * link, struct widget_link_t * parent);
extern __IPGUI_API__ int  ipgui_widget_link_set_first(struct widget_link_t * link);
extern __IPGUI_API__ int  ipgui_widget_link_set_last(struct widget_link_t * link);
extern __IPGUI_API__ ipgui_yes_no_t  ipgui_widget_link_is_parent_of(struct widget_link_t * parent, struct widget_link_t * child);
extern __IPGUI_API__ ipgui_yes_no_t  ipgui_widget_link_is_child_of(struct widget_link_t * child, struct widget_link_t * parent);
extern __IPGUI_API__ void ipgui_widget_link_insert_prev(struct widget_link_t * link, struct widget_link_t * next);
extern __IPGUI_API__ void ipgui_widget_link_insert_next(struct widget_link_t * link, struct widget_link_t * prev);
extern __IPGUI_API__ void ipgui_widget_link_move_before(struct widget_link_t * link);
extern __IPGUI_API__ void ipgui_widget_link_move_after(struct widget_link_t * link);
extern __IPGUI_API__ void ipgui_widget_link_foreach_dfs(struct widget_link_t * root, widget_ops_t ops, void * args);

#ifdef __cplusplus
}
#endif

#endif