#include "ipgui_screen.h"

panel_param_t scr1_attr_example = {
    .bpp = 24,
    .fps = 165,
    .ppi = 72,
    .xreso = 800,
    .yreso = 480,
    .pri_data = NULL,
};
void mouse_event_poll(ipgui_input_dev_t * dev, void * args)
{
    ipgui_input_event(dev, IPGUI_EVT_TYPE_REL, IPGUI_EVT_REL_X, 111);
    ipgui_input_sync(dev);
}
void keyboard_event_poll(ipgui_input_dev_t * dev, void * args)
{
    ipgui_input_event(dev, IPGUI_EVT_TYPE_REL, IPGUI_EVT_REL_Y, 121);
    ipgui_input_sync(dev);
}

int main(void)
{
    /* 支持多设备 */
    ipgui_scr_t * scr1 = ipgui_create_screen(&scr1_attr_example);
    scr1->name = "scr_example";
    ipgui_input_dev_t * mousedev = ipgui_input_allocate_device("mouse");
    ipgui_input_dev_t * keyboarddev = ipgui_input_allocate_device("keyboard");
    ipgui_input_set_capability(mousedev, IPGUI_EVT_TYPE_REL, IPGUI_EVT_REL_X);
    ipgui_input_set_capability(mousedev, IPGUI_EVT_TYPE_REL, IPGUI_EVT_REL_Y);
    ipgui_input_set_capability(mousedev, IPGUI_EVT_TYPE_REL, IPGUI_EVT_REL_WHEEL);
    ipgui_register_event_poll(mousedev, mouse_event_poll, NULL);
    ipgui_register_event_poll(keyboarddev, keyboard_event_poll, NULL);
    ipgui_screen_register_input_device(scr1, mousedev);
    ipgui_screen_register_input_device(scr1, keyboarddev);

}