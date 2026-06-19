#if 0 /* 测试代码 */
void print_widget_link(struct widget_link_t * link, void * args)
{
    plat_printf("link name: %s, children num:%d\r\n", link->name, link->child_num);
}

__IPGUI_API__ void ipgui_widget_tree_print(struct widget_tree_t * tree)
{
    ipgui_widget_link_foreach_dfs(tree, print_widget_link, NULL);
}

    struct widget_tree_t tree;
    struct widget_link_t link1;
    struct widget_link_t link2;
    struct widget_link_t link3;
    struct widget_link_t link4;
    struct widget_link_t link5;
    struct widget_link_t link6;
    struct widget_link_t link7;
    struct widget_link_t link8;
    struct widget_link_t link9;
    struct widget_link_t link10;
    ipgui_widget_tree_init(&tree);tree.root.name = "root";
    ipgui_widget_link_init(&link1);link1.name = "link1";
    ipgui_widget_link_init(&link2);link2.name = "link2";
    ipgui_widget_link_init(&link3);link3.name = "link3";
    ipgui_widget_link_init(&link4);link4.name = "link4";
    ipgui_widget_link_init(&link5);link5.name = "link5";
    ipgui_widget_link_init(&link6);link6.name = "link6";
    ipgui_widget_link_init(&link7);link7.name = "link7";
    ipgui_widget_link_init(&link8);link8.name = "link8";
    ipgui_widget_link_init(&link9);link9.name = "link9";
    ipgui_widget_link_init(&link10);link10.name = "link10";

    ipgui_widget_link_set_parent(&link1, &tree.root);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link2, &tree.root);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link3, &tree.root);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link4, &link1);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link5, &link1);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link6, &link3);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link7, &link4);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link8, &link4);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link9, &link4);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_link_set_parent(&link10, &link8);ipgui_widget_tree_print(&tree);printf("\r\n");
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_move_after(&link8);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_move_before(&link4);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_move_before(&link5);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_move_before(&link3);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_move_before(&link10);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_set_first(&link3);
    ipgui_widget_link_set_last(&link7);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    // ipgui_widget_link_set_first(&tree.root);
    ipgui_widget_link_detach(&link4);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_insert_prev(&link4,&link1);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_insert_next(&link4,&link5);
    ipgui_widget_tree_print(&tree);
    printf("\r\n");
    ipgui_widget_link_set_parent(&link4, &tree.root);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_detach(&link4);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_detach(&link3);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_set_parent(&link4, &link1);
    ipgui_widget_tree_print(&tree);

    printf("\r\n");
    ipgui_widget_link_set_parent(&link2, &link1);
    ipgui_widget_tree_print(&tree);

    struct ipgui_widget_link_t * root = ipgui_widget_link_get_root(&link1);
    struct ipgui_widget_link_t * root1 = ipgui_widget_link_get_root(&link2);
    struct ipgui_widget_link_t * root2 = ipgui_widget_link_get_root(&link4);
#endif