#include "ipgui_membox.h"

int main(void)
{
    ipgui_membox_t * box = (ipgui_membox_t *)0;
    box = ipgui_membox_create(8, 6);
    void * p1 = ipgui_membox_alloc( box );
    void * p2 = ipgui_membox_alloc( box );
    void * p3 = ipgui_membox_alloc( box );
    void * p4 = ipgui_membox_alloc( box );
    void * p5 = ipgui_membox_alloc( box );
    void * p6 = ipgui_membox_alloc( box );
    ipgui_membox_free(box, p1);
    void * p7 = ipgui_membox_alloc( box );
    return 0;
}