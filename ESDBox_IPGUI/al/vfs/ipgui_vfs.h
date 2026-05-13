#ifndef __IPGUI_VFS_H__
#define __IPGUI_VFS_H__

#include "ipgui_utils.h"
#include "ipgui_conf.h"
#include "ipgui_defs.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________


typedef enum
{
    IPGUI_FILE_MODE_READ,//r        /* 遇到的坑，windows中用fopen打开txt文件mode = "r"会读取不完整，改为"rb"就可以了，why? */
    IPGUI_FILE_MODE_WRITE,//w
    IPGUI_FILE_MODE_APPEND_WRITE,//a
    IPGUI_FILE_MODE_APPEND_WRITE_AND_READ,//a+
}ipgui_file_mode_e;

typedef enum
{
    IPGUI_FILE_SEEK_SET,//from beginning
    IPGUI_FILE_SEEK_CUR,//from current position
    IPGUI_FILE_SEEK_END,//from end(参数offset一般为负)
}ipgui_seek_mode_e;

typedef struct ipgui_drv
{
    int (* fs_mount)(struct ipgui_drv *, void * pri_drv);
    int (* fs_init)(struct ipgui_drv *, void * pri_drv);
    int (* fs_make)(unsigned int sec_num, unsigned int clu_size);
    int state;
}ipgui_drv_t;

typedef struct ipgui_file_st ipgui_file_t;

typedef struct ipgui_fs_t
{
    void * pri_drv;
    ipgui_drv_t * drv;

    /* file operations */
    int (* fopen)(struct ipgui_fs_t *, const char * path, void ** pri_file, ipgui_file_mode_e mode);
    int (* fread)(struct ipgui_fs_t *, void * buffer, unsigned int btr, unsigned int * br, void * pri_file);
    unsigned int (* frleft)(struct ipgui_fs_t *, void * pri_file);
    int (* fwrite)(struct ipgui_fs_t *, void * buffer, unsigned int btw, unsigned int * bw, void * pri_file);
    int (* fclose)(struct ipgui_fs_t *, void * pri_file);
    int (* fcreate)(struct ipgui_fs_t *, const char * path);
    unsigned int (* fsize)(struct ipgui_fs_t *, void * pri_file);
    int (* fdelete)(struct ipgui_fs_t *, const char * path);
    int (* frename)(struct ipgui_fs_t *, const char * path, const char * newpath);
    int (* fputs)(struct ipgui_fs_t *, const char * str, unsigned int * bw);
    int (* fseek)(struct ipgui_fs_t *, void * pri_file, ipgui_seek_mode_e mode, int offset);

    /* directory operations */
    int (* enter)(struct ipgui_fs_t *, const char * path, void * pri_dir);
    int (* drename)(struct ipgui_fs_t *, const char * path, const char * newpath, void * pri_dir);
    int (* mkdir)(struct ipgui_fs_t *, const char * path, void * pri_dir);
    int (* closedir)(struct ipgui_fs_t *, void * pri_dir);
}ipgui_fs_ops_t;

typedef struct
{
    void * base;
    unsigned int size;
    unsigned int pos;
    unsigned int end_pos;
}ipgui_file_cache_t;

#define IPGUI_FILE_OPEN_ERR 0x01 /* file open err */

typedef struct ipgui_file_st
{
    void * pri_file;
    unsigned char status;
    ipgui_fs_ops_t * fs;
    ipgui_file_cache_t * rd_cache;/* cache for read */
    ipgui_file_cache_t * wr_cache;/* cache for write */
}ipgui_file_t IPGUI_ST_ALIGN(IPGUI_MEM_ALIGN_SIZE);

typedef struct
{
    void * pri_dir;
    ipgui_fs_ops_t * fs;
}ipgui_dir_t;

extern ipgui_fs_ops_t ipgui_fs_win;
extern ipgui_fs_ops_t ipgui_fs_elos;
extern ipgui_fs_ops_t ipgui_fs_linux;
extern ipgui_fs_ops_t ipgui_fs_mac;

extern __IPGUI_API__ ipgui_err_t ipgui_file_init(ipgui_file_t * file);

extern __IPGUI_API__ ipgui_err_t ipgui_vfs_fopen(ipgui_file_t * file, const char * path, ipgui_file_mode_e mode, unsigned short rcache_size, unsigned int wcache_size);

extern __IPGUI_API__ ipgui_err_t ipgui_link_fs_auto(ipgui_file_t * file);

extern __IPGUI_API__ ipgui_err_t ipgui_vfs_fread(ipgui_file_t * file, void * buffer, unsigned int btr, unsigned int * br);

extern __IPGUI_API__ ipgui_err_t ipgui_vfs_fwrite(ipgui_file_t * file, void * buffer, unsigned int btw, unsigned int * bw);

extern __IPGUI_API__ ipgui_err_t ipgui_vfs_fseek(ipgui_file_t * file, ipgui_seek_mode_e mode, unsigned int offset);

extern __IPGUI_API__ ipgui_err_t ipgui_vfs_fclose(ipgui_file_t * file);

IPGUI_HEADER_END   _______________MARKER_______________
#endif