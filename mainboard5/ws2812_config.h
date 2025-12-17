/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 13:40:25
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-10-13 20:40:37
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#ifndef WS2812_CONFIG_H
#define WS2812_CONFIG_H
#include <Adafruit_NeoPixel.h>
#include "io.h"
extern Adafruit_NeoPixel pixels;
/*
0 未设置
1 错误指示灯
2 键盘信号指示灯
3 触摸信号指示灯
4 capslock指示灯
5 红色呼吸灯
6 绿色呼吸灯
7 蓝色呼吸灯
8 风扇速度反馈
9 背光电压反馈
10 随机数
11 电量
*/
class WS2812
{
public:
    bool begin(uint8_t func1, uint8_t func2, uint8_t func3, uint8_t func4);
    bool calculate(uint8_t battery_percent, bool error, bool keyboard_signal, bool touch_singnal, bool capslock, uint8_t fan_speed, uint8_t backlight_value);
    bool update();
    bool set_led_function(uint8_t num, uint8_t func);
    bool direct_led_set(uint8_t red[4],uint8_t green[4],uint8_t blue[4]);

private:
    uint8_t led_function[4];
    const uint8_t led_function_default[4] = {11,4,7,8};
    uint8_t rgb[3][12];
    const uint8_t breath_speed = 50;
    uint8_t breath = 0;
    bool breath_up_or_down = true;
};
#endif