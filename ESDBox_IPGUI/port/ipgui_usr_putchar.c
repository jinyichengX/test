#include <stdio.h>

/* 
 * Windows 版本：重定向 ipgui_putck() 到控制台输出。
 * 编译器会自动链接这个强符号而非 ipgui_debug.c 中的 __WEAK__ 版本。
 */
void ipgui_putck(char c)
{
    putchar(c);
}
