#include "ipgui_conf.h"

#if IPGUI_TICK_PER_SECOND < 1000
#error "IPGUI_TICK_PER_SECOND must can be divided by 1000"
#endif
