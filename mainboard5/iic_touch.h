/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 11:46:51
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-11-04 19:34:42
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#ifndef IIC_TOUCH_H
#define IIC_TOUCH_H

#include "GT911.h"
#include "MyMouseAbsolute.h"
#include "io.h"

#define touch_width 800
#define touch_height 480

extern GT911 tp;

class IIC_TOUCH
{
private:
   unsigned int mouse_left_size=40;
   unsigned int mouse_right_size=50;
public:
    bool begin();
    bool detect_touch_screen();
    bool update();
    unsigned int x = 32767 / 2;
    unsigned int y = 32767 / 2;
    bool touch_display_status = false;
    unsigned int is_touched = 0;
    const int is_touched_max = 10;
};

#endif