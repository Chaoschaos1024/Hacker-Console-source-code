/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 11:46:51
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-11-04 19:38:42
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#include "iic_touch.h"
#include "MyMouseAbsolute.h"

GT911 tp = GT911(iic1_sda, iic1_scl, touch_int, touch_rst, touch_width, touch_height);

bool IIC_TOUCH::begin()
{
    tp.begin();
    tp.setRotation(ROTATION_INVERTED);
    tp.setResolution(touch_width, touch_height);
    if (detect_touch_screen())
    {
        return false;
    }
    return true;
}
bool IIC_TOUCH::detect_touch_screen()
{
    Wire1.beginTransmission(0x5D);
    touch_display_status = !Wire1.endTransmission();
    if (touch_display_status)
    {
#if debug
        Serial1.println("touch panel connect!");
#endif
    }
    else
    {
#if debug
        Serial1.println("touch panel not connect!!!!");
#endif
    }
    return touch_display_status;
}
bool IIC_TOUCH::update()
{
    int max_size = 0;
    int whitch_touch_size_is_biggest = 0;
    if (touch_display_status)
    {
        if (!digitalRead(touch_int))
        {
#if debug
            Serial1.println("touch int is enabled!");
#endif
            tp.read();
            if (tp.isTouched)
            {
#if debug
                Serial1.println("gt911 have signel");
#endif
                is_touched = is_touched + 1;
                if (is_touched > is_touched_max)
                {
                    is_touched = is_touched_max;
                }
                if (tp.touches == 15)
                {
                    int bad_point = 0;
                    for (int i = 0; i < tp.touches; i++)
                    {
                        if (tp.points[i].x == 65535 & tp.points[i].y == 65535 & tp.points[i].size == 255)
                        {
                            bad_point = bad_point + 1;
#if debug
                            Serial1.print("bad point : ");
                            Serial1.print(i);
                            Serial1.print("/ x : ");
                            Serial1.print(tp.points[i].x);
                            Serial1.print("/ y : ");
                            Serial1.println(tp.points[i].y);
#endif
                        }
                    }
                    if (bad_point > 10)
                    {
#if debug
                        Serial1.println("touch error.too many bad points");
                        Serial1.print("bad point : ");
                        Serial1.println(bad_point);
#endif
                        touch_display_status = 0;
                        tp.begin();
                        tp.setRotation(ROTATION_INVERTED);
                        tp.setResolution(touch_width, touch_height);
                        return false;
                    }
                }
                for (int i = 0; i < tp.touches; i++)
                {
                    touch_display_status = 1;
                    if (tp.points[i].size > max_size)
                    {
                        max_size = tp.points[i].size;
                        whitch_touch_size_is_biggest = i;
                    }
                }
                x = map(tp.points[whitch_touch_size_is_biggest].x, 0, 800, 0, 32767);
                y = map(tp.points[whitch_touch_size_is_biggest].y, 0, 480, 0, 32767);
                //MouseAbsolute.move(x, y, 0);
                if (max_size > mouse_right_size)
                 {
                     uint8_t k = MOUSE_RIGHT;
                   MyMouseAbsolute.press(x, y, k);
                 }
                 else if (max_size > mouse_left_size)
                 {
                     uint8_t k = MOUSE_LEFT;
                   MyMouseAbsolute.press(x, y, k);
                 }
                 else
                 {
                   MyMouseAbsolute.move(x, y, 0);
                 }
#if debug
                Serial1.print("mouse move ro x : ");
                Serial1.print(x);
                Serial1.print(" y : ");
                Serial1.println(y);
                Serial1.print("touch size : ");
                Serial1.println(max_size);
#endif
            }
        }
        else
        {
            if (is_touched > 0)
            {
                is_touched = is_touched - 1;
            }
            else
            {
                is_touched = 0;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
    return false;
}