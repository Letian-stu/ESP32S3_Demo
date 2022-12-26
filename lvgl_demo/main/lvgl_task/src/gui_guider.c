/*
 * @Author: StuTian
 * @Date: 2022-09-05 14:07
 * @LastEditors: letian
 * @LastEditTime: 2022-12-26 15:57
 * @FilePath: \lvgl_demo\main\lvgl_task\src\gui_guider.c
 * @Description:
 * Copyright (c) 2022 by StuTian 1656733975@qq.com, All Rights Reserved.
 */
/*
 * Copyright 2022 NXP
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "string.h"
#include "lvgl/lvgl.h"
#include "gui_guider.h"

#define TAG "UI"

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
        printf("CLICKED\n");
        break;
    default:
        break;
    }
}

void setup_scr_screen(lv_ui *ui)
{
    ui->bg = lv_obj_create(NULL);
    lv_obj_set_pos(ui->bg, 0, 0);
    lv_obj_set_size(ui->bg, 240, 240);

    /*Properties to transition*/
    static lv_style_prop_t props[] = {
            LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_TRANSFORM_HEIGHT, LV_STYLE_TEXT_LETTER_SPACE, 0
    };

    /*Transition descriptor when going back to the default state.
     *Add some delay to be sure the press transition is visible even if the press was very short*/
    static lv_style_transition_dsc_t transition_dsc_def;
    lv_style_transition_dsc_init(&transition_dsc_def, props, lv_anim_path_overshoot, 250, 100, NULL);

    /*Transition descriptor when going to pressed state.
     *No delay, go to presses state immediately*/
    static lv_style_transition_dsc_t transition_dsc_pr;
    lv_style_transition_dsc_init(&transition_dsc_pr, props, lv_anim_path_ease_in_out, 250, 0, NULL);

    /*Add only the new transition to he default state*/
    static lv_style_t style_def;
    lv_style_init(&style_def);
    lv_style_set_transition(&style_def, &transition_dsc_def);

    /*Add the transition and some transformation to the presses state.*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_transform_width(&style_pr, 10);
    lv_style_set_transform_height(&style_pr, -10);
    lv_style_set_text_letter_space(&style_pr, 10);
    lv_style_set_transition(&style_pr, &transition_dsc_pr);

    ui->btn1 = lv_btn_create(ui->bg);
    lv_obj_add_event_cb(ui->btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(ui->btn1, LV_ALIGN_CENTER, 0, -70);
    lv_obj_add_style(ui->btn1, &style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(ui->btn1, &style_def, 0);
    ui->label = lv_label_create(ui->btn1);
    lv_label_set_text(ui->label, "Button1");
    lv_obj_center(ui->label);

    ui->btn2 = lv_btn_create(ui->bg);
    lv_obj_add_event_cb(ui->btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(ui->btn2, LV_ALIGN_CENTER, 0, -30);
    lv_obj_add_style(ui->btn2, &style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(ui->btn2, &style_def, 0);
    ui->label = lv_label_create(ui->btn2);
    lv_label_set_text(ui->label, "Button2");
    lv_obj_center(ui->label);

    ui->btn3 = lv_btn_create(ui->bg);
    lv_obj_add_event_cb(ui->btn3, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(ui->btn3, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_style(ui->btn3, &style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(ui->btn3, &style_def, 0);
    ui->label = lv_label_create(ui->btn3);
    lv_label_set_text(ui->label, "Button3");
    lv_obj_center(ui->label);

    ui->btn4 = lv_btn_create(ui->bg);
    lv_obj_add_event_cb(ui->btn4, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(ui->btn4, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_style(ui->btn4, &style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(ui->btn4, &style_def, 0);
    ui->label = lv_label_create(ui->btn4);
    lv_label_set_text(ui->label, "Button4");
    lv_obj_center(ui->label);

    lv_group_add_obj(ui->group, ui->btn1);
    lv_group_add_obj(ui->group, ui->btn2);
    lv_group_add_obj(ui->group, ui->btn3);
    lv_group_add_obj(ui->group, ui->btn4);
}

void setup_ui(lv_ui *ui)
{
    setup_scr_screen(ui);
    lv_scr_load(ui->bg);
}
