/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#define CANVAS_WIDTH  240
#define CANVAS_HEIGHT  280

static int last_x = -1;
static int last_y = -1; 

//screens
static  lv_obj_t *scr_home;
static  lv_obj_t *scr_left;
static  lv_obj_t *scr_right;
static  lv_obj_t *scr_top;
static  lv_obj_t *scr_bottom;

int create_screen_home();
int create_screen_left();
int create_screen_right();
int create_screen_top();
int create_screen_bottom();
int create_screen_test();

static void lv_btn_click_callback_left(lv_event_t *e);
static void lv_btn_click_callback_right(lv_event_t *e);
static void lv_btn_click_callback_top(lv_event_t *e);
static void lv_btn_click_callback_bottom(lv_event_t *e);

//events
static void on_lvgl_screen_gesture_event_callback(lv_event_t *e);
static void on_input_subsys_callback(struct input_event *evt);

int main(void)
{
	LOG_DBG("Starting lvgl test");
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	//INPUT_CALLBACK_DEFINE(NULL, on_input_subsys_callback);

#if CONFIG_LOGGING_SWIPES_ONLY
	create_screen_test();
#else
	create_screen_home();
#endif
	lv_task_handler();
	display_blanking_off(display_dev);

	
	while (1) {
		lv_task_handler();
		//k_sleep(K_MSEC(10));
		k_sleep(K_MSEC(250));
	}
	
}

static void handle_screen_gesture(lv_dir_t event_code)
{
        switch (event_code) {
            case LV_DIR_LEFT: {
				LOG_DBG("LEFT gesture detected");
				#if LOG_GESTURES_ONLY==1
				create_screen_left();
				#endif
                break;
            }
            case LV_DIR_RIGHT: {
				LOG_DBG("RIGHT gesture detected");
				#if LOG_GESTURES_ONLY==1
				create_screen_right();
				#endif
                break;
            }
            case LV_DIR_TOP: {
				LOG_DBG("TOP gesture detected");
				#if LOG_GESTURES_ONLY==1
				create_screen_top();
				#endif
                break;
            }
            case LV_DIR_BOTTOM: {
				LOG_DBG("BOTTOM gesture detected");
				#if LOG_GESTURES_ONLY==1
      			create_screen_bottom();
				#endif
                break;
            }
            default:
                LOG_ERR("Not a valid gesture code: %d", event_code);
        }

}

static void lv_btn_click_callback_left(lv_event_t *e)
{
	LOG_DBG("Button press left");
	ARG_UNUSED(e);
	lv_obj_del(scr_left);
	create_screen_home();
}

static void lv_btn_click_callback_right(lv_event_t *e)
{
	LOG_DBG("Button press right");
	ARG_UNUSED(e);
	lv_obj_del(scr_right);
	create_screen_home();
}

static void lv_btn_click_callback_top(lv_event_t *e)
{
	LOG_DBG("Button press top");
	ARG_UNUSED(e);
	lv_obj_del(scr_top);
	create_screen_home();
}

static void lv_btn_click_callback_bottom(lv_event_t *e)
{
	LOG_DBG("Button press bottom");
	ARG_UNUSED(e);
	lv_obj_del(scr_bottom);
	create_screen_home();
}

int create_screen_home()
{
	LOG_DBG("Creating home screen");
	scr_home = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_home, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_t *hello_world_label;

	lv_obj_set_style_bg_color(lv_scr_act(), lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	hello_world_label = lv_label_create(lv_scr_act());

	lv_label_set_text(hello_world_label, "Swipe to test!");
	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(hello_world_label, &lv_font_montserrat_16, LV_PART_MAIN);


	lv_obj_add_event_cb(lv_scr_act(), on_lvgl_screen_gesture_event_callback, LV_EVENT_GESTURE, NULL);

	return 0;
}

int create_screen_left()
{
	LOG_DBG("Creating left screen");
	scr_left = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_left, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	lv_obj_t *label = lv_label_create(lv_scr_act());

	if (IS_ENABLED(CONFIG_LV_Z_POINTER_KSCAN) || IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
		lv_obj_t *home_button;

		home_button = lv_btn_create(lv_scr_act());
		lv_obj_align(home_button, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(home_button, lv_btn_click_callback_left, LV_EVENT_CLICKED,
						NULL);
		label = lv_label_create(home_button);
	} else {
		label = lv_label_create(lv_scr_act());
	}


	lv_label_set_text(label, "LV_DIR_LEFT (1)");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);

	return 0;
}

int create_screen_right()
{
	LOG_DBG("Creating right screen");
	scr_right = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_right, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	lv_obj_t *label = lv_label_create(lv_scr_act());

	if (IS_ENABLED(CONFIG_LV_Z_POINTER_KSCAN) || IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
		lv_obj_t *home_button;

		home_button = lv_btn_create(lv_scr_act());
		lv_obj_align(home_button, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(home_button, lv_btn_click_callback_right, LV_EVENT_CLICKED,
						NULL);
		label = lv_label_create(home_button);
	} else {
		label = lv_label_create(lv_scr_act());
	}

	lv_label_set_text(label, "LV_DIR_RIGHT (2)");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);

	return 0;
}

int create_screen_top()
{
	LOG_DBG("Creating top screen");
	scr_top = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_top, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	lv_obj_t *label = lv_label_create(lv_scr_act());

	if (IS_ENABLED(CONFIG_LV_Z_POINTER_KSCAN) || IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
		lv_obj_t *home_button;

		home_button = lv_btn_create(lv_scr_act());
		lv_obj_align(home_button, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(home_button, lv_btn_click_callback_top, LV_EVENT_CLICKED,
						NULL);
		label = lv_label_create(home_button);
	} else {
		label = lv_label_create(lv_scr_act());
	}

	lv_label_set_text(label, "LV_DIR_TOP (3)");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);

	return 0;
}

int create_screen_bottom()
{
	LOG_DBG("Creating bottom screen");
	scr_bottom = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_bottom, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	lv_obj_t *label = lv_label_create(lv_scr_act());

	if (IS_ENABLED(CONFIG_LV_Z_POINTER_KSCAN) || IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
		lv_obj_t *home_button;

		home_button = lv_btn_create(lv_scr_act());
		lv_obj_align(home_button, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(home_button, lv_btn_click_callback_bottom, LV_EVENT_CLICKED,
						NULL);
		label = lv_label_create(home_button);
	} else {
		label = lv_label_create(lv_scr_act());
	}

	lv_label_set_text(label, "LV_DIR_BOTTOM (4)");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);

	return 0;
}


static void on_input_subsys_callback(struct input_event *evt)
{
	LOG_DBG("input subsys type=%d,code=%d,value=%d", evt->type, evt->code, evt->value);
	lv_color_t c0;
    lv_color_t c1;

    c0.full = 0;
    c1.full = 1;
	
	//LOG_DBG("input subsys type,code,value,%d,%d,%d", evt->type, evt->code, evt->value);
	if (evt->code == INPUT_ABS_X) {
		last_x = evt->value;
	}
	if (evt->code == INPUT_ABS_Y) {
		last_y = evt->value;
	}
	if (last_x != -1 && last_y != -1) {
		//lv_canvas_set_px_color(canvas, last_x, last_y, c0);
		last_x = -1;
		last_y = -1;
	}
}

static void on_lvgl_screen_gesture_event_callback(lv_event_t *e)
{
	LOG_DBG("Gesture event detected %d", e->code);
	#if CONFIG_LOGGING_SWIPES_ONLY==0
    lv_dir_t  dir;
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_GESTURE) {
        dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        handle_screen_gesture(dir);
    }
	lv_indev_wait_release(lv_indev_get_act()); // Needed otherwise accidental button press will happen
	#endif
}

int create_screen_test()
{
	LOG_DBG("Creating test screen");
	scr_home = lv_obj_create(NULL);
	// Used this function as it automatically deletes previous screen.
	// Should work as you did also I think. But have not used lv_scr_load_xxx before
  	lv_scr_load_anim(scr_home, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

	lv_obj_t *hello_world_label;

	lv_obj_set_style_bg_color(lv_scr_act(), lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

	hello_world_label = lv_label_create(lv_scr_act());

	lv_label_set_text(hello_world_label, "Swipe to test!");
	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(hello_world_label, &lv_font_montserrat_16, LV_PART_MAIN);


	lv_obj_add_event_cb(lv_scr_act(), on_lvgl_screen_gesture_event_callback, LV_EVENT_GESTURE, NULL);

	return 0;

}