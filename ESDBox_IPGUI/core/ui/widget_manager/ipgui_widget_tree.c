#include "ipgui_widget_tree.h"

/*
+------------+ 
| widget_tree_t（root link）|
| (parent=NULL)| 
+------------+  
        |                                    
        | first_child                           
        ↓                                       
    +------------+     +------------+       
    | link A     |<--->| link  C    |     
    +------------+     +------------+    
        |                   |
        | first_child       | first_child
        ↓                   ↓
    +------------+     +------------+
    | link  B    |     | linkD      |
    +------------+     +------------+

*/

#if 0
static struct widget_link_t * detached_monitor = (struct widget_link_t *)0;
#endif

/* 一个节点是不是处于游离态，如果父节点也是处于游离态那么这个节点也处于游离态 */
__IPGUI_API__ ipgui_yes_no_t ipgui_widget_link_is_detached(struct widget_link_t * link)
{
    if (link->parent == (struct widget_link_t *)0) return IPGUI_NO;
    if (link->parent == link) return IPGUI_YES;
    struct widget_link_t * parent = link->parent;

    while (parent) {
        if (parent->parent == parent)
            return IPGUI_YES;
        parent = parent->parent;
    }

    return IPGUI_NO;
}

/* init widget link */
__IPGUI_API__ void ipgui_widget_link_init(struct widget_link_t * link)
{
    link->parent = link;
    link->sib_next = link;
    link->sib_prev = link;
    link->first_child = (struct widget_link_t *)0;
    link->child_num = 0;
}

/* int widget tree */
__IPGUI_API__ void ipgui_widget_tree_init(struct widget_tree_t * tree)
{
    ipgui_widget_link_init(&tree->root);
    tree->root.parent = (struct widget_link_t *)0;
    tree->root.sib_next = &tree->root;
    tree->root.sib_prev = &tree->root;
    tree->root.first_child = (struct widget_link_t *)0;
    tree->root.child_num = 0;
}

/* if link is a root link */
__IPGUI_API__ ipgui_yes_no_t ipgui_widget_link_is_root(struct widget_link_t * link)
{
    if (!link->parent) return IPGUI_YES;
    else return IPGUI_NO;
}

/* get the link root */
__IPGUI_API__ struct widget_link_t * 
ipgui_widget_link_get_root(struct widget_link_t * link)
{
    if (IPGUI_YES == ipgui_widget_link_is_root(link))
        return link;
    struct widget_link_t * parent;
    parent = link->parent;
    while(parent->parent) {
        parent = parent->parent;
    }
    return parent;
}

/* detach the link from it's tree */
__IPGUI_API__ void ipgui_widget_link_detach(struct widget_link_t * link)
{
    if (IPGUI_YES == ipgui_widget_link_is_root(link))
        return;
    
    if (link->parent == link) return;

    link->sib_prev->sib_next = link->sib_next;
    link->sib_next->sib_prev = link->sib_prev;

    if (link->sib_next == link) {
        link->parent->first_child = (struct widget_link_t *)0;
    }
    else if (link->parent->first_child == link) {
        link->parent->first_child = link->sib_next;
    }
    if (link->parent != link) /* the link is in the init state（detached state游离态） */
        link->parent->child_num --;
    link->parent = link;
}

/* set link's parent */
__IPGUI_API__ void ipgui_widget_link_set_parent(
    struct widget_link_t * link,
    struct widget_link_t * parent)
{
    if (IPGUI_YES == ipgui_widget_link_is_root(link))
        return;

    if (link->parent == parent)
        return;

    ipgui_widget_link_detach(link);
    link->parent = parent;

    if (!parent->first_child) {
        link->sib_prev = link;
        link->sib_next = link;
        parent->first_child = link;
    } else {
        link->sib_next = parent->first_child;
        link->sib_prev = parent->first_child->sib_prev;
        parent->first_child->sib_prev->sib_next = link;
        parent->first_child->sib_prev = link;
    }
    parent->child_num ++;
}

/* set link as the last child of it's parent */
__IPGUI_API__ int ipgui_widget_link_set_last(struct widget_link_t * link)
{
    struct widget_link_t * parent = link->parent;
    if (parent && (parent != link))
    {
        if (parent->first_child->sib_prev == link)
            return 0;
        ipgui_widget_link_detach(link);
        
        link->parent = parent;
        if (!parent->first_child) {
            link->sib_prev = link;
            link->sib_next = link;
            parent->first_child = link;
        } else {
            link->sib_next = parent->first_child;
            link->sib_prev = parent->first_child->sib_prev;
            parent->first_child->sib_prev->sib_next = link;
            parent->first_child->sib_prev = link;
        }
        parent->child_num ++;
        return 0;
    } else {
        return 0; /* it is a root link, also ok */
    }
}

/* set link as the first child of it's parent*/
__IPGUI_API__ int ipgui_widget_link_set_first(struct widget_link_t * link)
{
    struct widget_link_t * parent = link->parent;
    if (parent && (parent != link))
    {
        if (parent->first_child == link)
            return 0;
        ipgui_widget_link_detach(link);

        link->parent = parent;
        if (!parent->first_child) {
            link->sib_prev = link;
            link->sib_next = link;
        } else {
            link->sib_next = parent->first_child;
            link->sib_prev = parent->first_child->sib_prev;
            link->sib_prev->sib_next = link;
            link->sib_next->sib_prev = link;
        }
        parent->first_child = link;
        parent->child_num ++;
        return 0;
    } else {
        return 0; /* it is a root link, also ok */
    }
}

/* if the parent is a parent of the child */
__IPGUI_API__ ipgui_yes_no_t ipgui_widget_link_is_parent_of(
    struct widget_link_t * parent,
    struct widget_link_t * child)
{
    struct widget_link_t * iter;
    iter = child ? child->parent : (struct widget_link_t *)0;

    while (iter) {
        if (iter == parent)
            return IPGUI_YES;
        iter = iter->parent;
    }
    return IPGUI_NO;
}

/* if the child is a child of the parent */
__IPGUI_API__ ipgui_yes_no_t ipgui_widget_link_is_child_of(
    struct widget_link_t * child,
    struct widget_link_t * parent)
{
    return ipgui_widget_link_is_parent_of(parent, child);
}

/* insert the link before next, and next is a sibling of link */
__IPGUI_API__ void ipgui_widget_link_insert_prev(
    struct widget_link_t * link,
    struct widget_link_t * next)
{
    if ((IPGUI_YES == ipgui_widget_link_is_root(link)) \
        || (IPGUI_YES == ipgui_widget_link_is_root(next)))
        return;
    
    struct widget_link_t * parent = next->parent;
    
    ipgui_widget_link_detach(link);
    
    /* insert link before next */
    link->parent = parent;
    link->sib_prev = next->sib_prev;
    link->sib_next = next;
    
    next->sib_prev->sib_next = link;
    next->sib_prev = link;
    
    /* if next is the first link，then update first_child */
    if (parent->first_child == next) {
        parent->first_child = link;
    }
    
    parent->child_num ++;
}   

/* insert the link after prev, and prev is a sibling of link */
__IPGUI_API__ void ipgui_widget_link_insert_next(
    struct widget_link_t * link,
    struct widget_link_t * prev)
{
    if ((IPGUI_YES == ipgui_widget_link_is_root(link)) \
        || (IPGUI_YES == ipgui_widget_link_is_root(prev)))
        return;

    struct widget_link_t * parent = prev->parent;
    
    ipgui_widget_link_detach(link);
    
    /* insert link after prev */
    link->parent = parent;
    link->sib_prev = prev;
    link->sib_next = prev->sib_next;
    
    prev->sib_next->sib_prev = link;
    prev->sib_next = link;
    
    parent->child_num ++;
}

/* move the link before the link->sib_prev */
__IPGUI_API__ void ipgui_widget_link_move_before(
    struct widget_link_t * link)
{
    if (IPGUI_YES == ipgui_widget_link_is_root(link))
        return;
    if (link->parent == link)
        return;
    if (link->parent->first_child == link)
        return;
    ipgui_widget_link_insert_prev(link, link->sib_prev);
}

/* move the link after the link->sib_next */
__IPGUI_API__ void ipgui_widget_link_move_after(
    struct widget_link_t * link)
{
    if (IPGUI_YES == ipgui_widget_link_is_root(link))
        return;
    if (link->parent == link)
        return;
    if (link->parent->first_child->sib_prev == link)
        return;
    ipgui_widget_link_insert_next(link, link->sib_next);
}

/* traverse the widget tree in dfs order */
__IPGUI_API__ void ipgui_widget_link_foreach_dfs(
    struct widget_link_t * root,
    widget_ops_t ops, void * args)
{
    if (!root)
        return;
    struct widget_link_t ** child = &root->first_child;
    if (ops)
        ops(root, args);
    while (*child) {
        if ((*child)->first_child) {
            ipgui_widget_link_foreach_dfs(*child, ops, args);
        } else if (ops) {
            ops(*child, args);
        }
        child = &((*child)->sib_next);
        if (*child == root->first_child)
            break;
    }
}