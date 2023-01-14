/*
 * @Author: StuTian
 * @Date: 2022-09-05 14:07
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 21:47
 * @FilePath: \take_photo_to_lvgl\main\lvgl_task\src\gui_guider.c
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

// extern lv_img_dsc_t cam_img_dsc;

void setup_scr_screen(lv_ui *ui)
{
    ui->bg = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(ui->bg, 0, 0);
    lv_obj_set_size(ui->bg, 320, 240);

    lv_obj_t * label = lv_label_create(ui->bg);
    lv_obj_set_width(label, 20);
    lv_obj_set_pos(label, 260, 50);
    lv_label_set_text(label, "h\ne\nl\nl\n0");

	ui->objpg = lv_img_create(ui->bg);	
    lv_obj_set_pos(ui->objpg, 0, 0);			
    lv_img_set_src(ui->objpg, "S:/icon.png");
}

void setup_ui(lv_ui *ui)
{
    setup_scr_screen(ui);
    lv_scr_load(ui->bg);
}
