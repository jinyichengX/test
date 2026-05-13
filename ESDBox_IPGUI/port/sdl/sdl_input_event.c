#include "sdl_input_event.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_input.h"
__IPGUI_API__ void ipgui_sdl_mouse_event_poll(ipgui_input_dev_t * dev, void * args);
__IPGUI_API__ void ipgui_sdl_keyboard_event_poll(ipgui_input_dev_t * dev, void * args);
extern ipgui_input_drv_t drv;
unsigned int sdl_event_mapping[][2] =
{
    SDL_QUIT,IPGUI_EVT_BTN_LEFT,
    SDL_KEYDOWN,    IPGUI_EVT_BTN_LEFT,
    SDL_KEYUP,       IPGUI_EVT_BTN_LEFT,
    SDL_MOUSEMOTION,IPGUI_EVT_BTN_LEFT,
    SDL_MOUSEBUTTONDOWN, IPGUI_EVT_BTN_LEFT,
    SDL_MOUSEBUTTONUP, IPGUI_EVT_BTN_LEFT,
    SDL_MOUSEWHEEL, IPGUI_EVT_BTN_LEFT,
    SDL_FINGERDOWN,IPGUI_EVT_BTN_LEFT,
    SDL_FINGERUP,IPGUI_EVT_BTN_LEFT,
    SDL_FINGERMOTION,IPGUI_EVT_BTN_LEFT,
};

void sdl_event_print(ipgui_input_dev_t * dev)
{
    if( test_bit(IPGUI_EVT_TYPE_SYNC, (void *)dev->evtbits) )
    {
        ipgui_dbg_info("device : %s support sync event\n", dev->name);
    }
    if( test_bit(IPGUI_EVT_TYPE_KEY, (void *)dev->evtbits) ){
        ipgui_dbg_info("device : %s support key event\n", dev->name);

        for( int i = 0; i < IPGUI_EVT_KEY_CNT; ++ i )
        {
            if( test_bit(i, (void *)dev->keybits) )
                ipgui_dbg_info("key code : %d\n", i);
        }
    }
    if( test_bit(IPGUI_EVT_TYPE_ABS, (void *)dev->evtbits) ){
        ipgui_dbg_info("device : %s support abs event\n", dev->name);

        for( int i = 0; i < IPGUI_EVT_ABS_CNT; ++ i )
        {
            if( test_bit(i, (void *)dev->absbits) )
                ipgui_dbg_info("abs code : %d\n", i);
        }
    }
    if( test_bit(IPGUI_EVT_TYPE_REL, (void *)dev->evtbits) ){
        ipgui_dbg_info("device : %s support rel event\n", dev->name);

        for( int i = 0; i < IPGUI_EVT_REL_CNT; ++ i )
        {
            if( test_bit(i, (void *)dev->relbits) )
                ipgui_dbg_info("rel code : %d\n", i);
        }
    }
}

/* 初始化鼠标设备 鼠标设备上传REL坐标，但是ipgui只接收ABS坐标 */
__IPGUI_API__ ipgui_input_dev_t * ipgui_sdl_mouse_create_init(void)
{
    ipgui_input_dev_t * sdl_mouse_dev = ipgui_input_allocate_device("sdl_mouse");
    if( NULL == sdl_mouse_dev )
    {
        ipgui_mem_free(ipgui_smem, sdl_mouse_dev);
        return IPGUI_ERR_NOMEM;
    }

    ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_BTN_LEFT);
    ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_BTN_RIGHT);
    //ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_BTN_MIDDLE);

    /* register partial mouse code */
    ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_ABS, IPGUI_EVT_ABS_X);
    ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_ABS, IPGUI_EVT_ABS_Y);
    //ipgui_input_set_capability(sdl_mouse_dev, IPGUI_EVT_TYPE_ABS, IPGUI_EVT_ABS_WHEEL);

    ipgui_register_event_poll(sdl_mouse_dev, ipgui_sdl_mouse_event_poll, NULL);

    // sdl_event_print(sdl_mouse_dev);
    return sdl_mouse_dev;
}

/* 初始化键盘设备 */
__IPGUI_API__ ipgui_input_dev_t * ipgui_sdl_create_keyboard_init(void)
{
    ipgui_input_dev_t * sdl_keyboard_dev = ipgui_input_allocate_device("sdl_keyboard");
    if( NULL == sdl_keyboard_dev )
    {
        ipgui_mem_free(ipgui_smem, sdl_keyboard_dev);
        return IPGUI_ERR_NOMEM;
    }

    /* register all key code */
    for(int i = 0; i < IPGUI_EVT_KEY_CNT; i ++)
    {
        ipgui_input_set_capability(sdl_keyboard_dev, IPGUI_EVT_TYPE_KEY, i);
    }

    ipgui_register_event_poll(sdl_keyboard_dev, ipgui_sdl_keyboard_event_poll, NULL);

    // sdl_event_print(sdl_keyboard_dev);
    return sdl_keyboard_dev;
}

int g_sdl_mouse_x = 0, g_sdl_mouse_y = 0;
int g_pressed = 0, g_last_pressed = 0;

int sdl_mouse_read(struct ipgui_input_drv_t * dev, ipgui_input_data_t * data)
{
    data->data.pid.x = g_sdl_mouse_x;
    data->data.pid.y = g_sdl_mouse_y;
    data->data.pid.state = g_pressed ? IPGUI_PRESS_STATE_DOWN : IPGUI_PRESS_STATE_UP;
    return 0;
}

/* must be called periodically */
__IPGUI_API__ void ipgui_sdl_mouse_event_poll(ipgui_input_dev_t * dev, void * args)
{
    SDL_Event event;

    ipgui_input_dev_t * sdl_input_dev = dev;
    
    if (SDL_PollEvent(&event))
    {
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            g_pressed = g_last_pressed = 1;
            g_sdl_mouse_x = event.button.x;
            g_sdl_mouse_y = event.button.y;
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            g_pressed = g_last_pressed = 0;
            g_sdl_mouse_x = event.button.x;
            g_sdl_mouse_y = event.button.y;
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            g_pressed = g_last_pressed ? 1 : 0;
            g_sdl_mouse_x = event.motion.x;
            g_sdl_mouse_y = event.motion.y;
        }
    }
}

/* must be called periodically */
__IPGUI_API__ void ipgui_sdl_keyboard_event_poll(ipgui_input_dev_t * dev, void * args)
{
    SDL_Event event;

    ipgui_input_dev_t * sdl_input_dev = dev;
    
    if( SDL_PollEvent( &event ) )
    {
        if( event.type == SDL_KEYDOWN )
        {
            ipgui_input_event(dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_KEY_1, 1);
            ipgui_input_sync(dev);
        }
        else if( event.type == SDL_KEYUP )
        {
            ipgui_input_event(dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_KEY_1, 0);
            ipgui_input_sync(dev);
        }
    }
}