#include "ipgui_vfs.h"
#include <stdio.h>
ipgui_file_t file;
unsigned char buffer1[100] = {1,2,2};
unsigned char buffer2[100] = {1,2,2};
int br,bw;
FILE * fp;
int main(void)
{
    ipgui_link_fs_auto(&file);
    ipgui_vfs_fopen(&file, "M:/ESDBox_IPGUI/aaa.txt", IPGUI_FILE_MODE_APPEND_WRITE_AND_READ);
    ipgui_vfs_fread(&file, (void * )buffer1, 4, &br);
    ipgui_vfs_fread(&file, (void * )buffer1, 7, &br);
    ipgui_vfs_fwrite(&file, "154", 3, &bw);
    ipgui_vfs_fseek(&file, 0);
    ipgui_vfs_fread(&file, (void * )buffer1, 16, &br);
    ipgui_vfs_fseek(&file, 0);
    ipgui_vfs_fwrite(&file, "154", 3, &bw);
    ipgui_vfs_fclose(&file);
    //ipgui_vfs_fread(&file, "123", 3, &br);
    return 0;
}