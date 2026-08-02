#include "ipgui_avl.h"

/* avl树初始化 */
void * avl_init(avl_t * tree, compare vcomp, u8_t node_off)
{
    if(( tree == (avl_t *)0 ) || ( vcomp == (compare *)0 )) 
        return (void *)0;
    tree->root = (avl_node_t *)0;
    tree->node_off = node_off;
    tree->vcomp = vcomp;
    tree->count = 0;
	
	return (void *)tree;
}

/* 初始化节点 */
static inline void avl_node_init(avl_node_t * node)
{
    node->height = 1;
    node->lchild = (avl_node_t *)0;
    node->rchild = (avl_node_t *)0;
    node->parent = (avl_node_t *)0;
}

/* 更新树高 */
static inline void avl_node_height_update(avl_node_t * node)
{
    int lheight,rheight;
    if( node == (avl_node_t *)0 ) return;
    lheight = AVL_LCHILD_HEIGHT(node);
    rheight = AVL_RCHILD_HEIGHT(node);
    node->height = AVL_MAX(lheight,rheight) + 1;
}

/* 获取树的第一个叶子节点 */
avl_node_t * avl_find_first_node(avl_t * tree)
{
    avl_node_t * node = tree->root;
    if(node == (avl_node_t *)0) return (avl_node_t *)0;
    while(node->lchild)
        node = node->lchild;
    return node;
}

/* 获取树的最后一个叶子节点 */
avl_node_t * avl_find_last_node(avl_t * tree)
{
    avl_node_t * node = tree->root;
    if(node == (avl_node_t *)0) return (avl_node_t *)0;
    while(node->rchild)
        node = node->rchild;
    return node;
}

/* 获取一个节点的后继节点 */
avl_node_t * avl_next_node(avl_node_t * node)
{
	if (node == (avl_node_t *)0) return (avl_node_t *)0;
	if (node->rchild) {
		node = node->rchild;
		while (node->lchild) 
			node = node->lchild;
	}
	else {
		while (1) {
			avl_node_t *last = node;
			node = node->parent;
			if (node == (avl_node_t *)0) break;
			if (node->lchild == last) break;
		}
	}
	return node;
}

/* 获取一个节点的前驱节点 */
avl_node_t * avl_prev_node(avl_node_t * node)
{
	if (node == (avl_node_t *)0) return (avl_node_t *)0;
	if (node->lchild) {
		node = node->lchild;
		while (node->rchild) 
			node = node->rchild;
	}
	else {
		while (1) {
			avl_node_t *last = node;
			node = node->parent;
			if (node == (avl_node_t *)0) break;
			if (node->rchild == last) break;
		}
	}
	return node;
}

/* 交换节点和它的左孩 */
void avl_node_exchange_with_l(avl_node_t * node, avl_node_t * lchild)
{
    avl_node_t * parent = node->parent;
    lchild->rchild = node;
    lchild->parent = parent;
    node->parent = lchild;
    node->lchild = (avl_node_t *)0;
	if(parent) parent->rchild = lchild;
}

/* 交换节点和它的右孩 */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
void avl_node_exchange_with_r(avl_node_t * node, avl_node_t * rchild)
{
    avl_node_t * parent = node->parent;
    rchild->lchild = node;
    rchild->parent = parent;
    node->parent = rchild;
    node->rchild = (avl_node_t *)0;
	if(parent) parent->lchild = rchild;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     

/* 右旋节点 */
void avl_node_ratate_right(avl_node_t * node,avl_t * tree)
{
    avl_node_t * lchild = node->lchild;
    avl_node_t * parent = node->parent;

    node->lchild = lchild->rchild;
    node->parent = lchild;
    lchild->parent = parent;
    if(lchild->rchild)
        lchild->rchild->parent = node;
    lchild->rchild = node;
    if(parent != (avl_node_t *)0){
        if(parent->lchild == node) parent->lchild = lchild;
        else parent->rchild = lchild;       
    }
    else tree->root = lchild;
}

/* 左旋节点 */
void avl_node_ratate_left(avl_node_t * node, avl_t * tree)
{
    avl_node_t * rchild = node->rchild;
    avl_node_t * parent = node->parent;

    node->rchild = rchild->lchild;
    node->parent = rchild;
    rchild->parent = parent;
    if(rchild->lchild)
        rchild->lchild->parent = node;
    rchild->lchild = node;
    if(parent != (avl_node_t *)0)
    {
        if(parent->lchild == node) parent->lchild = rchild;
        else parent->rchild = rchild;
    }
    else tree->root = rchild;
}

/* LL/LR左失衡处理 */
void avl_left_unbalan_handle(avl_node_t * subtree, avl_t * tree)
{
    int lh,rh;
    avl_node_t * tree_root = subtree;
    avl_node_t * left = tree_root->lchild;
    lh = AVL_LCHILD_HEIGHT(left);
    rh = AVL_RCHILD_HEIGHT(left);
    if( rh > lh ){
        /* 左旋LR型成LL型 */
        avl_node_ratate_left(left, tree);
        avl_node_height_update(left->lchild);
        avl_node_height_update(left);
    }
    /*右旋LL型*/
    avl_node_ratate_right(tree_root, tree);
    /*调节被旋转节点的右子树的高度*/
    avl_node_height_update(tree_root->rchild);
    avl_node_height_update(tree_root);
}

/* RR/RL右失衡处理 */
void avl_right_unbalan_handle(avl_node_t * subtree, avl_t * tree)
{
    int lh,rh;
    avl_node_t * tree_root = subtree;
    avl_node_t * right = tree_root->rchild;
    lh = AVL_LCHILD_HEIGHT(right);
    rh = AVL_RCHILD_HEIGHT(right);
    if( lh > rh ){
        /* 右旋RL型成RR型 */
        avl_node_ratate_right(right, tree);
        avl_node_height_update(right->rchild);
        avl_node_height_update(right);
    }
    /*左旋RR型*/
    avl_node_ratate_left(tree_root, tree);
    /*调节被旋转节点的左子树的高度*/
    avl_node_height_update(tree_root->lchild);
    avl_node_height_update(tree_root);
}

/* 向上遍历父节点更新树高 */
void avl_node_post_height_updata(avl_node_t * start)
{
    avl_node_t * index = start;
    if( start == (avl_node_t *)0 ) return;
    while(index != (avl_node_t *)0)
    {
        avl_node_height_update(index);
        index = index->parent;
    }
}

/* 处理失衡 */
avl_node_t * avl_node_post_unbalance(avl_node_t * start, avl_t * tree)
{
    avl_node_t * pos = start;
    int balance_factor = 0;
    while((avl_node_t *)0 != pos)
    {
        avl_node_height_update(pos);
        balance_factor = AVL_TREE_BLNFCT(pos);
        if( balance_factor >= 2 )
        {
            /* LL或LR失衡 */
            avl_left_unbalan_handle(pos, tree);
            //break;
        }else if( balance_factor <= -2 )
        {
            /* RR或RL失衡 */
            avl_right_unbalan_handle(pos, tree);
            //break;
        }
        pos = pos->parent;
    }
    return pos;
}

/* 添加叶子节点 */
avl_node_t * avl_node_add_leaf(avl_node_t * node, avl_node_t * parent, avl_node_t ** pos)
{
    (*pos) = node;
    node->parent = parent;
    node->lchild = node->rchild = (avl_node_t *)0;
    node->height = 1;
	
	return node;
}

/* 插入节点(通用方法) */
avl_node_t * g_avl_node_add(void * node_cont, avl_t * tree)
{
    int ret;
    avl_node_t ** pos = &(tree->root);
    avl_node_t * parent = (avl_node_t *)0;
    avl_node_t * node_to_bala;
	avl_node_t * node;
	void * pos_cont;
    if(( node_cont == (void *)0 )||( tree == (avl_t *)0 ))
        return (avl_node_t *)0;
	node = _AVL_CONTAINER2NODE(node_cont, tree->node_off);
    avl_node_init(node);
    while(* pos != (avl_node_t *)0){
		pos_cont = _AVL_NODE2CONTAINER(* pos, tree->node_off);
        ret = tree->vcomp((void *)pos_cont, (void *)node_cont);
        parent = *pos;
        if( ret == -1 ){
            pos = &((*pos)->lchild);
        }else if( ret == 1 ){
            pos = &((*pos)->rchild);
        }else if( ret == 0 ){
            pos = &((*pos)->rchild);
        }
    }
    /* 找到了插入的节点,先插入 */
    avl_node_add_leaf(node, parent, pos);
    /* post后处理,向上平衡最小不平衡子树 */
    node_to_bala = avl_node_post_unbalance(node, tree);
    /* 向上更新树高 */
    avl_node_post_height_updata(node_to_bala);
    tree->count ++;
    return (*pos != (avl_node_t *)0) ? (* pos) : (avl_node_t *)0;  
}

/* 插入节点 */
avl_node_t * avl_node_add(avl_node_t * node, avl_t * tree)
{
    int ret;
    avl_node_t ** pos = &(tree->root);
    avl_node_t * parent = (avl_node_t *)0;
    avl_node_t * node_to_bala;
    if(( node == (avl_node_t *)0 )||( tree == (avl_t *)0 ))
        return (avl_node_t *)0;
    avl_node_init(node);
    while(*pos != (avl_node_t *)0){
        ret = tree->vcomp((void *)&((*pos)->value), (void *)&(node->value));
        parent = *pos;
        if( ret == -1 ){
            pos = &((*pos)->lchild);
        }else if( ret == 1 ){
            pos = &((*pos)->rchild);
        }else if( ret == 0 ){
            pos = &((*pos)->rchild);
        }
    }
    /* 找到了插入的节点,先插入 */
    avl_node_add_leaf(node, parent, pos);
    /* post后处理,向上平衡最小不平衡子树 */
    node_to_bala = avl_node_post_unbalance(node, tree);
    /* 向上更新树高 */
    avl_node_post_height_updata(node_to_bala);
    tree->count ++;
    return (*pos != (avl_node_t *)0) ? (* pos) : (avl_node_t *)0;  
}

/* 查找节点（通用方法） */
avl_node_t * g_avl_node_search(void * node_cont, avl_t * tree)
{
    int ret;
    avl_node_t * pos = tree->root;
	void * pos_cont;
    if(( node_cont == (void *)0 )||( tree == (avl_t *)0 ))
        return (avl_node_t *)0;
    while(pos != (avl_node_t *)0){
		pos_cont = _AVL_NODE2CONTAINER(pos, tree->node_off);
        ret = tree->vcomp((void *)pos_cont, (void *)node_cont);
        if( ret == -1 ){
            pos = pos->lchild;
        }else if( ret == 1 ){
            pos = pos->rchild;
        }else if( ret == 0 ){
            return pos;
        }
    }
    return (avl_node_t *)0;  
}

/* 查找节点 */
avl_node_t * avl_node_search(avl_node_t * node, avl_t * tree)
{
    int ret;
    avl_node_t * pos = tree->root;
    if(( node == (avl_node_t *)0 )||( tree == (avl_t *)0 ))
        return (avl_node_t *)0;
    while(pos != (avl_node_t *)0){
        ret = tree->vcomp((void *)&pos->value, (void *)&node->value);
        if( ret == -1 ){
            pos = pos->lchild;
        }else if( ret == 1 ){
            pos = pos->rchild;
        }else if( ret == 0 ){
            return pos;
        }
    }
    return (avl_node_t *)0;  
}

/* 删除节点（通用方法） */
avl_node_t * avl_node_delete(avl_node_t * node, avl_t * tree)
{
    avl_node_t * parent = (avl_node_t *)0;
    avl_node_t * child = (avl_node_t *)0;
    if( node == (avl_node_t *)0 )
        return (avl_node_t *)0;
    /* case one：被删除节点有左右孩子节点 */
    if((node->lchild)&&(node->rchild))
    {
		avl_node_t * old = node;
		avl_node_t * lchild = (avl_node_t *)0;
        /* 寻找被删除节点的后继节点 */
		node = node->rchild;
		while ((lchild = node->lchild) != (avl_node_t *)0)
			node = lchild;
		child = node->rchild;
		parent = node->parent;
		if (child) {
			child->parent = parent;
		}
        /* 被删除节点的后继节点的父节点安置 */
        if(parent){
            if(parent->lchild == node) parent->lchild = child;
            else parent->rchild = child;
        }else tree->root = child;

		if (node->parent == old)
			parent = node;
        /* 后继节点继承到被删除节点的位置 */
		node->lchild = old->lchild;
		node->rchild = old->rchild;
		node->parent = old->parent;
		node->height = old->height;
        /* 被删除节点的父节点安置 */
        if(old->parent){
            if(old->parent->lchild == old) old->parent->lchild = node;
            else old->parent->rchild = node;
        }else tree->root = node;

		old->lchild->parent = node;
		if (old->rchild) {
			old->rchild->parent = node;
		}
    }
    /* case two：被删除节点只有一个孩子节点或删除的节点是叶子节点 */
    else
    {
        parent = node->parent;
        if(node->lchild != (avl_node_t *)0) child = node->lchild;
        else child = node->rchild;
        if(child) child->parent = parent;
        
        if(parent){
            if(parent->lchild == node) parent->lchild = child;
            else parent->rchild = child;
        }else tree->root = child;
    }
    if(parent)
        avl_node_post_unbalance(parent, tree);
    tree->count --;
	
	return node;
}