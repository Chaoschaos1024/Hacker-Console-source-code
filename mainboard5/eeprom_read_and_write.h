/*
   @Author: 无序熵增
   @Date: 2025-07-25 11:23:42
   @LastEditors: 无序熵增
   @LastEditTime: 2025-09-18 17:25:57
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
#ifndef EEPROM_READ_AND_WRITE
#define EEPROM_READ_AND_WRITE
#include <EEPROM.h>
#include "SoftwareSerial.h"
#include "config.h"

#define COMPILE_FLAG ((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'))

class EEPROM_CONFIG
{
  public:
    bool begin();
    uint8_t read_fan_value();
    float read_backlight_value();
    bool write_fan_value(uint8_t value);
    bool write_backlight_value(uint8_t value);
    uint8_t read_led_function(uint8_t num);
    bool write_led_function(uint8_t num, uint8_t func);
    uint16_t charging_battery_max=0;
    uint16_t read_charging_battery_max();
    bool write_charging_battery_max(uint16_t battery);
    uint16_t battery_max=0;
    uint16_t read_battery_max();
    bool write_battery_max(uint16_t battery);
    uint16_t battery_min=0;
    uint16_t read_battery_min();
    bool write_battery_min(uint16_t battery);
    float device_version=0;
#if debug
    bool eeprom_report();
#endif

    uint8_t fan_pwm_value = 0;
    uint8_t backlight_pwm_value = 0;

  private:
    const int eeprom_space = 4096;
    const uint8_t fan_pwm_value_address = 1;
    const uint8_t backlight_pwm_value_address = 3;
    const uint8_t fan_pwm_value_default = 50;
    const float backlight_pwm_value_default = 95;
    const uint8_t setup_flag = version_major;
    const uint8_t setup_flag_ = version_minor;

    const uint16_t setup_flag_address = 666;

    bool write_uint16_t(uint16_t num, unsigned int addr);
    uint16_t read_uint16_t(unsigned int addr);
    const unsigned int charging_battery_max_address = setup_flag_address + 10;
    const unsigned int battery_max_address = setup_flag_address + 20;
    const unsigned int battery_min_address = setup_flag_address + 30;

    const uint16_t battery_min_default=40000;
    const uint16_t battery_max_default=40000;

    bool empty_eeprom();


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
    uint8_t ws2812_func[4] = {11, 4, 7, 8};
    uint8_t ws2812_func_address[4] = {10, 20, 30, 40};
};
#endif
