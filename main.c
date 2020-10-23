#include "lvgl/lvgl.h"

#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <SDL2/SDL.h>
#include <lv_examples/lv_apps/demo/demo.h>
//#include <lv_examples/src/lv_demo_widgets/lv_demo_widgets.h>
//#include <lv_examples/study/005_tasks/task.h>

#define DISP_BUF_SIZE (10 * LV_HOR_RES_MAX)

static int tick_thread(void * data);
static void memory_monitor(lv_task_t * param);

int main(void) {
    /*LittlevGL init*/
    lv_init();

    monitor_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf1[DISP_BUF_SIZE];
    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf1, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.flush_cb = monitor_flush;
    lv_disp_drv_register(&disp_drv);

    //鼠标输入
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    //键盘输入
    keyboard_init();
    lv_indev_drv_t sdl_kb_drv;
    lv_indev_drv_init(&sdl_kb_drv);
    sdl_kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    //sdl_kb_drv.read_cb = keyboard_read; //每个gui时间片轮询都会调用此回调函数
    lv_indev_t * sdl_kb_indev = lv_indev_drv_register(&sdl_kb_drv);

    /*Create a Demo*/
    demo_create();

    SDL_CreateThread(tick_thread, "tick", NULL);

    //lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);

    while(1) {
        lv_task_handler();

        usleep(5 * 1000);

#ifdef SDL_APPLE
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
#if USE_MOUSE != 0
            mouse_handler(&event);
#endif
#if USE_KEYBOARD
            keyboard_handler(&event);
#endif
        }
#endif        
    }

    return 0;
}

static int tick_thread(void * data) {
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}