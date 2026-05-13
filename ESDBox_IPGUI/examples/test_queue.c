#include "ipgui_queue.h"
unsigned char data1[10] = {1,1,1,1,1,1};
unsigned char data2[10] = {2,2,2,2,2,2};
unsigned char data3[10] = {3,3,3,3,3,3};
unsigned char data4[10] = {4,4,4,4,4,4};
unsigned char data5[10] = {5,5,5,5,5,5};
unsigned char data6[10] = {6,6,6,6,6,6};
unsigned char data7[10] = {7,7,7,7,7,7};
unsigned char data8[10] = {8,8,8,8,8,8};
unsigned char data9[10] = {9,9,9,9,9,9};
unsigned char data10[10] = {10,10,10,10,10,10};
ipgui_queue_t * queue;
    // uint32_t aaa1 = 64;
    // uint32_t aaa2 = 12;
    // uint32_t aaa3 = 56;
    // uint32_t aaa4 = 1;
    // uint32_t aaa5 = 46;
    // uint32_t aaa6 = 17;
    // uint32_t aaa7 = 22;
    // uint32_t aaa8 = 0;
    // uint32_t aaa9 = 84;
    // uint32_t aaa10 = 9;
int main(void)
{
    ipgui_queue_create(&queue, sizeof(int), 10);
    ipgui_queue_publish(queue, data1, sizeof(int), 64);
    ipgui_queue_publish(queue, data2, sizeof(int), 12);
    ipgui_queue_publish(queue, data3, sizeof(int), 56);
    ipgui_queue_publish(queue, data4, sizeof(int), 1);
    ipgui_queue_publish(queue, data5, sizeof(int), 46);
    ipgui_queue_publish(queue, data6, sizeof(int), 17);
    ipgui_queue_publish(queue, data7, sizeof(int), 22);
    ipgui_queue_publish(queue, data8, sizeof(int), 0);
    ipgui_queue_publish(queue, data9, sizeof(int), 84);
    ipgui_queue_publish(queue, data10, sizeof(int), 9);

    ipgui_queue_subscribe(queue, data1, sizeof(int));
    ipgui_queue_subscribe(queue, data2, sizeof(int));
    ipgui_queue_subscribe(queue, data3, sizeof(int));
    ipgui_queue_subscribe(queue, data4, sizeof(int));
    ipgui_queue_subscribe(queue, data5, sizeof(int));
    ipgui_queue_subscribe(queue, data6, sizeof(int));
    ipgui_queue_subscribe(queue, data7, sizeof(int));
    ipgui_queue_subscribe(queue, data8, sizeof(int));
    ipgui_queue_subscribe(queue, data9, sizeof(int));
    ipgui_queue_subscribe(queue, data10, sizeof(int));

    ipgui_queue_publish(queue, data1, sizeof(int), 64);
    ipgui_queue_publish(queue, data2, sizeof(int), 12);
    ipgui_queue_publish(queue, data3, sizeof(int), 56);
    ipgui_queue_publish(queue, data4, sizeof(int), 1);
    ipgui_queue_publish(queue, data5, sizeof(int), 46);
    ipgui_queue_publish(queue, data6, sizeof(int), 17);
    ipgui_queue_publish(queue, data7, sizeof(int), 22);
    ipgui_queue_publish(queue, data8, sizeof(int), 0);
    ipgui_queue_publish(queue, data9, sizeof(int), 84);
    ipgui_queue_publish(queue, data10, sizeof(int), 9);
    return 0;
}