#ifndef IPGUI_DEBUG_H
#define IPGUI_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#define IPGUI_DEBUG_LEVEL_0       0
#define IPGUI_DEBUG_LEVEL_1       1
#define IPGUI_DEBUG_LEVEL_2       2
#define IPGUI_DEBUG_LEVEL_3       3

#define IPGUI_DEBUG_NONE     IPGUI_DEBUG_LEVEL_0
#define IPGUI_DEBUG_INFO     IPGUI_DEBUG_LEVEL_1
#define IPGUI_DEBUG_ERROR    IPGUI_DEBUG_LEVEL_2
#define IPGUI_DEBUG_WARNING  IPGUI_DEBUG_LEVEL_3

#define ipgui_dbg_info      ipgui_printk
#define ipgui_dbg_warning   ipgui_printk
#define ipgui_dbg_error     ipgui_printk

#define ipgui_dbg_assert

extern void ipgui_printk(char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif