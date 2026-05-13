#include "ipgui_darray.h"
#include <stdint.h>
int main(void)
{
    int index = 123213;
    int pop;
    int number;
    unsigned int data_test[100] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    ipgui_darray_t darray1;
    ipgui_darray_init(&darray1, 4);
    void * pp1 = ipgui_darray_index(&darray1, 0);

    index = ipgui_darray_element_append(&darray1, data_test, 2);
    number = *(unsigned int *)ipgui_darray_last_element(&darray1);
    number = *(uint32_t *)ipgui_darray_first_element(&darray1);
    index = ipgui_darray_element_append(&darray1, &data_test[2], 1);
    index = ipgui_darray_element_append(&darray1, &data_test[3], 1);
    index = ipgui_darray_element_append(&darray1, &data_test[4], 5);
    number = *(unsigned int *)ipgui_darray_last_element(&darray1);
    number = *(uint32_t *)ipgui_darray_first_element(&darray1);
    ipgui_darray_element_updata(&darray1, 8, &data_test[9]);
    number = *(unsigned int *)ipgui_darray_last_element(&darray1);
    number = *(uint32_t *)ipgui_darray_first_element(&darray1);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    // index = ipgui_darray_element_pop(&darray1, &pop);
    // index = ipgui_darray_element_pop(&darray1, &pop);
    // index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_append(&darray1, data_test, 2);
    index = ipgui_darray_element_append(&darray1, &data_test[2], 1);
    index = ipgui_darray_element_append(&darray1, &data_test[3], 1);
    index = ipgui_darray_element_append(&darray1, &data_test[4], 5);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    index = ipgui_darray_element_pop(&darray1, &pop);
    ipgui_darray_deinit(&darray1);
    return 0;
}