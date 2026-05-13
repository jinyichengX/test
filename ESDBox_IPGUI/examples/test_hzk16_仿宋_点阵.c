#include <stdio.h>

/* 编译此程序必须指定编译器的输入输出编码格式！！！ */

int main(void)
{
    /* 测试 */
    int i = 0;
    int j, k, flag;
    unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    unsigned char * chineseword = "中国";

    FILE* fd = fopen("./hzk/HZK16", "rb");

    if( fd == NULL){
        printf("open file error\n");
    }
    unsigned char frame_buffer[1024] = {0};
    while( chineseword[i] ){
        /* 一个区有94个汉字，区号从0xa1开始，位号从0xa1开始 */
        int offset = (94 * (unsigned int)(chineseword[i] - 0xa1) + (chineseword[i + 1] - 0xa0 - 1)) * 32;
        fseek(fd, offset, SEEK_SET);
        fread(frame_buffer, 1, 32, fd);

        for (k = 0; k < 32; k++) {
            printf("%02x ", frame_buffer[k]);
        }
        printf("\n");
    
        for (k = 0; k < 16; k++) {
            for (j = 0; j < 2; j++) {
                for (int u = 0; u < 8; u++) {
                    flag = frame_buffer[k * 2 + j] & key[u];
                    printf("%d", flag ? 1 : 0);
                }
            }
            printf("\n");
        }
        i += 2;
    }
    fclose(fd);
    return 0;
}