// shit AI 写的
// 输入分发系统 - 最终版
// 裸机/RTOS通用，纯C实现，零依赖
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// ==================== 配置参数 ====================
#define EVENT_QUEUE_SIZE    16      // 事件队列大小
#define MAX_DEVICES         4       // 最大支持设备数
#define MAX_SCREENS         2       // 最大支持屏幕数
#define LONG_PRESS_MS       500     // 长按触发时间(毫秒)

// 队列满时策略: 1=丢弃最旧事件 0=丢弃最新事件
#define QUEUE_DROP_OLD      1

// ==================== 错误码定义 ====================
typedef enum {
    INPUT_OK            = 0,
    INPUT_ERR_FULL      = -1,
    INPUT_ERR_NOT_FOUND = -2,
    INPUT_ERR_INVALID_ID= -3,
    INPUT_ERR_NULL_PTR  = -4
} input_err_t;

// ==================== 事件类型 ====================
typedef enum {
    INPUT_EVENT_NONE       = 0xFF,
    INPUT_EVENT_PRESS      = 0,
    INPUT_EVENT_MOVE       = 1,
    INPUT_EVENT_RELEASE    = 2,
    INPUT_EVENT_LONG_PRESS = 3
} input_event_type_t;

// 标准输入事件结构
typedef struct {
    uint8_t device_id;
    input_event_type_t type;
    int16_t x, y;
    uint32_t timestamp;
} input_event_t;

// ==================== 输入设备描述符 ====================
typedef struct {
    uint8_t id;
    bool enabled;
    void (*init)(void);
    void (*read)(input_event_t* ev);
    char name[16];
    bool (*filter)(input_event_t* ev); // 设备级事件过滤
} input_device_t;

// ==================== 设备-屏幕映射表 ====================
typedef struct {
    uint8_t target_screen;
    bool enabled;
} mapping_entry_t;

// ==================== 屏幕事件接口 ====================
typedef struct {
    uint8_t id;
    void (*handle_event)(input_event_t* ev);
    char name[16];
} ui_screen_t;

// ==================== 分发器全局上下文 ====================
typedef struct {
    // 环形事件队列
    input_event_t queue[EVENT_QUEUE_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    uint32_t overflow_count;

    // 设备/映射/屏幕表 (ID直接索引，O(1)查找)
    input_device_t devices[256];
    mapping_entry_t mappings[256];
    ui_screen_t screens[256];

    uint8_t device_count;
    uint8_t screen_count;

    // 全局控制
    bool global_enable;
    bool (*global_filter)(input_event_t* ev);
    void (*event_notify)(void); // RTOS事件通知回调

    // 长按检测状态
    uint8_t pressed_device;
    int16_t pressed_x;
    int16_t pressed_y;
    uint32_t pressed_timestamp;
    bool long_press_triggered;
} input_dispatcher_t;

static input_dispatcher_t dispatcher;

// ==================== 系统时钟接口 (用户必须重写) ====================
__attribute__((weak)) uint32_t input_get_tick_ms(void) {
    // 请替换为你的系统毫秒级时钟
    // 例如: return HAL_GetTick();
    return 0;
}

// ==================== 内部私有函数 ====================
static input_err_t enqueue_event(input_event_t* ev) {
    uint8_t next_head = (dispatcher.head + 1) % EVENT_QUEUE_SIZE;

    if (next_head == dispatcher.tail) {
        dispatcher.overflow_count++;
#if QUEUE_DROP_OLD
        dispatcher.tail = (dispatcher.tail + 1) % EVENT_QUEUE_SIZE;
#else
        return INPUT_ERR_FULL;
#endif
    }

    dispatcher.queue[dispatcher.head] = *ev;
    dispatcher.head = next_head;

    if (dispatcher.event_notify) {
        dispatcher.event_notify();
    }

    return INPUT_OK;
}

static bool dequeue_event(input_event_t* ev) {
    if (dispatcher.head == dispatcher.tail) {
        return false;
    }

    *ev = dispatcher.queue[dispatcher.tail];
    dispatcher.tail = (dispatcher.tail + 1) % EVENT_QUEUE_SIZE;
    return true;
}

static void process_long_press(void) {
    if (dispatcher.pressed_device == 0xFF || dispatcher.long_press_triggered) {
        return;
    }

    uint32_t now = input_get_tick_ms();
    if (now - dispatcher.pressed_timestamp >= LONG_PRESS_MS) {
        input_event_t long_ev = {
            .device_id = dispatcher.pressed_device,
            .type = INPUT_EVENT_LONG_PRESS,
            .x = dispatcher.pressed_x,
            .y = dispatcher.pressed_y,
            .timestamp = now
        };
        enqueue_event(&long_ev);
        dispatcher.long_press_triggered = true;
    }
}

// ==================== 对外公共API ====================

/**
 * @brief 初始化输入分发器
 * @note 必须在任何其他API调用之前执行
 */
void input_dispatcher_init(void) {
    memset(&dispatcher, 0, sizeof(input_dispatcher_t));
    dispatcher.pressed_device = 0xFF;
}

/**
 * @brief 注册输入设备
 * @param dev 输入设备描述符指针
 * @return 错误码
 */
input_err_t input_register_device(input_device_t* dev) {
    if (!dev) return INPUT_ERR_NULL_PTR;
    if (dev->id == 0xFF) return INPUT_ERR_INVALID_ID;
    if (dispatcher.devices[dev->id].id == dev->id) return INPUT_ERR_INVALID_ID;
    if (dispatcher.device_count >= MAX_DEVICES) return INPUT_ERR_FULL;

    dispatcher.devices[dev->id] = *dev;
    dispatcher.device_count++;

    if (dev->init) {
        dev->init();
    }

    // 自动创建默认映射(设备->屏幕0)
    dispatcher.mappings[dev->id].target_screen = 0;
    dispatcher.mappings[dev->id].enabled = true;

    return INPUT_OK;
}

/**
 * @brief 注册UI屏幕
 * @param screen 屏幕描述符指针
 * @return 错误码
 */
input_err_t input_register_screen(ui_screen_t* screen) {
    if (!screen) return INPUT_ERR_NULL_PTR;
    if (screen->id == 0xFF) return INPUT_ERR_INVALID_ID;
    if (dispatcher.screens[screen->id].id == screen->id) return INPUT_ERR_INVALID_ID;
    if (dispatcher.screen_count >= MAX_SCREENS) return INPUT_ERR_FULL;

    dispatcher.screens[screen->id] = *screen;
    dispatcher.screen_count++;
    return INPUT_OK;
}

/**
 * @brief 设置设备到屏幕的映射关系
 * @param dev_id 设备ID
 * @param scr_id 目标屏幕ID
 * @return 错误码
 */
input_err_t input_set_mapping(uint8_t dev_id, uint8_t scr_id) {
    if (dev_id == 0xFF || scr_id == 0xFF) return INPUT_ERR_INVALID_ID;
    if (dispatcher.devices[dev_id].id != dev_id) return INPUT_ERR_NOT_FOUND;
    if (dispatcher.screens[scr_id].id != scr_id) return INPUT_ERR_NOT_FOUND;

    dispatcher.mappings[dev_id].target_screen = scr_id;
    dispatcher.mappings[dev_id].enabled = true;
    return INPUT_OK;
}

/**
 * @brief 启用/禁用设备映射
 * @param dev_id 设备ID
 * @param enable true=启用 false=禁用
 * @return 错误码
 */
input_err_t input_enable_mapping(uint8_t dev_id, bool enable) {
    if (dev_id == 0xFF) return INPUT_ERR_INVALID_ID;
    if (dispatcher.devices[dev_id].id != dev_id) return INPUT_ERR_NOT_FOUND;

    dispatcher.mappings[dev_id].enabled = enable;
    return INPUT_OK;
}

/**
 * @brief 上报原始输入事件
 * @param dev_id 设备ID
 * @param type 事件类型
 * @param x X坐标/按键值/增量
 * @param y Y坐标/保留
 * @return 错误码
 * @note 可在中断中直接调用
 */
input_err_t input_report(uint8_t dev_id, input_event_type_t type, int16_t x, int16_t y) {
    if (!dispatcher.global_enable || type == INPUT_EVENT_NONE) return INPUT_OK;
    if (dispatcher.devices[dev_id].id != dev_id) return INPUT_ERR_NOT_FOUND;

    input_event_t ev = {
        .device_id = dev_id,
        .type = type,
        .x = x,
        .y = y,
        .timestamp = input_get_tick_ms()
    };

    // 事件过滤链
    if (dispatcher.devices[dev_id].filter && dispatcher.devices[dev_id].filter(&ev)) {
        return INPUT_OK;
    }
    if (dispatcher.global_filter && dispatcher.global_filter(&ev)) {
        return INPUT_OK;
    }

    // 更新长按检测状态
    if (type == INPUT_EVENT_PRESS) {
        dispatcher.pressed_device = dev_id;
        dispatcher.pressed_x = x;
        dispatcher.pressed_y = y;
        dispatcher.pressed_timestamp = ev.timestamp;
        dispatcher.long_press_triggered = false;
    } else if (type == INPUT_EVENT_RELEASE) {
        dispatcher.pressed_device = 0xFF;
    }

    return enqueue_event(&ev);
}

/**
 * @brief 输入事件分发主函数
 * @note 在UI线程主循环中调用
 */
void input_dispatch(void) {
    if (!dispatcher.global_enable) return;

    process_long_press();

    input_event_t ev;
    while (dequeue_event(&ev)) {
        mapping_entry_t* map = &dispatcher.mappings[ev.device_id];
        if (!map->enabled) continue;

        ui_screen_t* screen = &dispatcher.screens[map->target_screen];
        if (screen->handle_event) {
            screen->handle_event(&ev);
        }
    }
}

/**
 * @brief 轮询所有已注册的输入设备
 * @note 裸机环境在主循环中调用，RTOS可单独开线程
 */
void input_poll_devices(void) {
    for (int i = 0; i < 256; i++) {
        input_device_t* dev = &dispatcher.devices[i];
        if (dev->id != i || !dev->enabled || !dev->read) continue;

        input_event_t ev = {.type = INPUT_EVENT_NONE};
        dev->read(&ev);
        if (ev.type != INPUT_EVENT_NONE) {
            input_report(dev->id, ev.type, ev.x, ev.y);
        }
    }
}

/**
 * @brief 全局启用/禁用输入分发器
 * @param enable true=启用 false=禁用
 */
void input_set_enable(bool enable) {
    dispatcher.global_enable = enable;
}

/**
 * @brief 设置全局事件过滤函数
 * @param filter 过滤函数指针，返回true则丢弃事件
 */
void input_set_global_filter(bool (*filter)(input_event_t* ev)) {
    dispatcher.global_filter = filter;
}

/**
 * @brief 设置事件通知回调
 * @param notify 通知函数指针，有新事件时调用
 * @note RTOS环境使用，用于唤醒UI线程
 */
void input_set_event_notify(void (*notify)(void)) {
    dispatcher.event_notify = notify;
}

/**
 * @brief 获取队列溢出次数
 * @return 溢出次数
 */
uint32_t input_get_overflow_count(void) {
    return dispatcher.overflow_count;
}

/**
 * @brief 清空事件队列
 */
void input_clear_queue(void) {
    dispatcher.head = dispatcher.tail = 0;
    dispatcher.overflow_count = 0;
}

// ==================== 使用示例 ====================
/*
// 1. 实现触摸驱动
static void touch_init(void) {
    // 触摸屏硬件初始化
}

static void touch_read(input_event_t* ev) {
    if (touch_is_pressed()) {
        ev->type = INPUT_EVENT_PRESS;
        ev->x = touch_get_x();
        ev->y = touch_get_y();
    } else if (touch_is_released()) {
        ev->type = INPUT_EVENT_RELEASE;
    } else {
        ev->type = INPUT_EVENT_NONE;
    }
}

static input_device_t touch_dev = {
    .id = 0,
    .enabled = true,
    .init = touch_init,
    .read = touch_read,
    .name = "touch0"
};

// 2. 实现屏幕事件处理
static void main_screen_handler(input_event_t* ev) {
    // 你的UI事件处理逻辑
    switch (ev->type) {
        case INPUT_EVENT_PRESS:
            // 处理按下
            break;
        case INPUT_EVENT_MOVE:
            // 处理移动
            break;
        case INPUT_EVENT_RELEASE:
            // 处理抬起
            break;
        case INPUT_EVENT_LONG_PRESS:
            // 处理长按
            break;
        default:
            break;
    }
}

static ui_screen_t main_screen = {
    .id = 0,
    .handle_event = main_screen_handler,
    .name = "main"
};

// 3. 主函数
int main(void) {
    // 系统初始化
    SystemClock_Config();

    // 初始化输入系统
    input_dispatcher_init();
    input_register_device(&touch_dev);
    input_register_screen(&main_screen);
    input_set_enable(true);

    // 主循环
    while (1) {
        input_poll_devices();  // 轮询硬件
        input_dispatch();      // 分发事件
        // 你的UI渲染逻辑
    }
}
*/