/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 11:43:28
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-10-10 22:39:16
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#ifndef IO_H
#define IO_H

#include <Arduino.h>
#include "RP2040_PWM.h"
#include "config.h"
#include <Wire.h>
#include <SoftwareSerial.h>
// 引脚定义
#define gpio0 0
#define gpio1 1
#define gpio2 2
#define gpio3 3
#define gpio4 4
#define gpio5 5
#define gpio6 6
#define gpio7 7
#define gpio8 8
#define gpio9 9
#define gpio10 10
#define gpio11 11
#define gpio12 12
#define gpio13 13
#define gpio14 14
#define gpio15 15
#define gpio16 16
#define gpio17 17
#define gpio18 18
#define gpio19 19
#define gpio20 20
#define gpio21 21
#define gpio22 22
#define gpio23 23
#define gpio24 24
#define gpio25 25
#define gpio26 A0
#define gpio27 A1
#define gpio28 A2
#define gpio29 A3

#define debug_tx 0
#define debug_rx 1
#define iic1_sda 2
#define iic1_scl 3
#define keyboard_int 4
#define touch_rst 8
#define touch_int 9
#define fan_pwm 10
#define hdmi_active 11
#define led1 12
#define led2 13
#define screen_backlight_pwm 14
#define screen_enable 15
#define spi_miso 16
#define spi_cs 17
#define spi_sck 18
#define spi_mosi 19
#define iic0_sda 20
#define iic0_scl 21
#define charge_status 23
#define charge_done_status 24
#define ws2812 25
#define backlight A0
#define battery A1
#define main_power A2
#define pi_3v3 A3

extern uint32_t pwm_pins[];
extern RP2040_PWM *pwm_instance[2];

class IO_CONFIG
{
public:
  bool begin();
  uint8_t battery_percent_read();
  uint16_t battery_voltage_read();
  uint16_t pi_main_power_read();
  uint16_t backlight_voltage_read();
  uint16_t pi_3v3_voltage_read();
  bool fan_setup(uint8_t fan_duty_cycle);
  bool fan_update(uint8_t fan_duty_cycle);
  //bool fan_speed_increase();
  //bool fan_speed_reduce();
  bool backlight_setup(float backlight_duty_cycle);
  bool backlight_update(float backlight_duty_cycle);
  //bool backlight_increase();
  //bool backlight_reduce();
  uint8_t  fan_report();
  uint8_t backlight_report();
  bool read_hdmi_status();
  bool update_charge_status();
  bool battery_voltage_update();
  void led1_flip();
  void led2_flip();
  void led1_high();
  void led1_low();
  void led2_high();
  void led2_low();

  const uint16_t battery_0_percent = 33000;
  const uint16_t battery_100_percent = 43000;
  uint8_t battery_percent = 0;
  uint16_t battery_voltage = 0;
  uint16_t pi_main_power_voltage = 0;
  uint16_t backlight_voltage = 0;
  uint16_t pi_3v3_voltage = 0;

  const uint16_t backlight_pwm_freq = 10000;
  const uint16_t fan_pwm_freq = 10000;
  uint8_t fan_speed_value = 0;
  uint8_t fan_speed_step = 2;
  const uint8_t fan_speed_max = 100;
  const uint8_t fan_speed_min = 0;
  float backlight_value = 0;
  float backlight_step = 0.2;
  const float backlight_max=100;
  const float backlight_min=80;

  bool screen_enabled = false;

  bool charge_or_not = false;
  bool charge_done_or_not = false;

private:
  bool serial_setup();
  bool iic_setup();
  bool pin_setup();
  bool adc_setup();
  uint8_t battery_count=0;
  uint8_t battery_count_max=10;
  unsigned long battery_long=0;
  const int delay_=500;
};

#endif
