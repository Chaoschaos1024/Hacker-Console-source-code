/*
 * @Author: 无序熵增
 * @Date: 2024-01-14 21:39:41
 * @LastEditors: 无序熵增
 * @LastEditTime: 2024-06-19 12:24:42
 * @Description: 
 * 
 * Copyright (c) 2024 by 无序熵增, All Rights Reserved. 
 */
//版本信息
#define version 3.0
//debug功能
#define debug 0

//debug delay 
#define debug_delay 1
#define debug_delay_short 100
#define debug_delay_long 1000
//杂务效率
#define other_issues 12
//以下按键在调试板上
#define key_enable 1
//开启键盘功能
#define keyboard_enable 1
//开启触摸功能
#define touch_enable 0
//灯珠功能定义
/*
0 电量
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
*/
#define ws2812_enable 1
#define ws2812_0 4
#define ws2812_1 5
#define ws2812_2 8
#define ws2812_3 10
#define breath_speed 1//数越高，呼吸灯的速度就越慢
//风扇和背光的参数
#define backlight_pwm_freq 10000
#define fan_pwm_freq 10000
#define backlight_duty_cycle_max 100
#define backlight_duty_cycle_min 80
#define fan_duty_cycle_max 100.0
#define fan_duty_cycle_min 0.0
//adc
#define backlight_adc_times 11
#define battery_adc_times 11
#define main_power_adc_times 2
#define pi_3v3_adc_times 2
#define battery_adc_count 10
//电池电量参数
#define battery_voltage_max 43000
#define battery_voltage_min 33000
//摇杆速度
#define stick_times 0.6
//指令模式延时时间
#define command_delay 30
//风扇设置步进值
#define fan_step 3
