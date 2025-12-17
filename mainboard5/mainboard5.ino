/*
   @Author: 无序熵增
   @Date: 2025-07-24 17:59:36
   @LastEditors: 无序熵增
   @LastEditTime: 2025-11-03 16:04:14
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
#include "config.h"
#include "io.h"
#include "eeprom_read_and_write.h"
#include "iic_touch.h"
#include "iic_keyboard.h"
#include "ssd1306.h"
#include "ws2812_config.h"

IO_CONFIG io;
EEPROM_CONFIG erom;
WS2812 ws;
IIC_KEYBOARD kb;
IIC_TOUCH touch;
IIC_DISPLAY iic_display;

uint8_t loop_count = 0;
#define loop_count_max 10

unsigned int mouse_x = 0;
unsigned int mouse_y = 0;

#if debug
unsigned int delay_time = 0;
long long int loop_count_debug = 0;
int allwaysdelay = 0;
#endif
void setup() {
  io.begin();
#if debug
  Serial1.println("-----------------------------------------------------------------------------------------------");
  Serial1.println("-----------------------------------------------------------------------------------------------");
  Serial1.println("-----------------------------------------------------------------------------------------------");
  Serial1.println("-----------------------------------------------------------------------------------------------");
  Serial1.println("██   ██  █████   ██████ ██   ██  ██████  ██████  ███    ██ ███████  ██████  ██      ███████  ");
  Serial1.println("██   ██ ██   ██ ██      ██  ██  ██      ██    ██ ████   ██ ██      ██    ██ ██      ██ ");
  Serial1.println("███████ ███████ ██      █████   ██      ██    ██ ██ ██  ██ ███████ ██    ██ ██      █████ ");
  Serial1.println("██   ██ ██   ██ ██      ██  ██  ██      ██    ██ ██  ██ ██      ██ ██    ██ ██      ██ ");
  Serial1.println("██   ██ ██   ██  ██████ ██   ██  ██████  ██████  ██   ████ ███████  ██████  ███████ ███████ ");
#endif
#if debug
  pinMode(gpio16, INPUT_PULLUP);
  pinMode(gpio17, INPUT_PULLUP);
  pinMode(gpio18, INPUT_PULLUP);
  pinMode(gpio19, INPUT_PULLUP);
  pinMode(gpio20, INPUT_PULLUP);
  pinMode(gpio21, INPUT_PULLUP);
  pinMode(gpio22, INPUT_PULLUP);
#endif
  io.led1_flip();
  erom.begin();
  io.led1_flip();
  io.fan_setup(erom.read_fan_value());
  io.led1_flip();
  io.backlight_setup(erom.read_backlight_value());
  io.led1_flip();
  ws.begin(erom.read_led_function(1), erom.read_led_function(2), erom.read_led_function(3), erom.read_led_function(4));
  io.led1_flip();
  iic_display.begin();
  delay(1000);
  io.led1_flip();
  kb.begin();
  io.led1_flip();
  touch.begin();
  mouse_x = touch.x;
  mouse_y = touch.y;
  io.led1_flip();
#if debug
  Serial1.println("setup finish");
#endif
  io.led1_high();
#if debug
  delay(800);
#endif
}

void loop() {
#if debug
  long long int time_now = rp2040.getCycleCount64();
  Serial1.print("loop begin. loop count : ");
  Serial1.print(loop_count);
  Serial1.print("/");
  Serial1.println(loop_count_max);
#endif
  kb.routine(mouse_x, mouse_y);  //
  mouse_x = mouse_x + kb.mouse_x_ * kb.stick_speed - 128 * kb.stick_speed;
  mouse_y = mouse_y + kb.mouse_y_ * kb.stick_speed - 128 * kb.stick_speed;
  kb.mouse_x_ = 128;
  kb.mouse_y_ = 128;
#if debug
  Serial1.println("keyboard mouse : ");
  Serial1.print("mouse x : ");
  Serial1.println(mouse_x);
  Serial1.print("mouse y : ");
  Serial1.println(mouse_y);
#endif
  touch.update();
  if (touch.is_touched) {
#if debug
    Serial1.println("touch panel connected!!");
#endif
    mouse_x = touch.x;
    mouse_y = touch.y;
#if debug
    Serial1.println("touch : ");
    Serial1.print("mouse x : ");
    Serial1.println(mouse_x);
    Serial1.print("mouse y : ");
    Serial1.println(mouse_y);
#endif
  } else {
#if debug
    Serial1.println("no touch panel signal");
#endif
  }
  if (kb.need_reduce_fan_speed) {
    if (io.fan_speed_value > io.fan_speed_min) {
      io.fan_speed_value = io.fan_speed_value - io.fan_speed_step;
    } else {
      io.fan_speed_value = 0;
    }
    io.fan_update(io.fan_speed_value);
    erom.write_fan_value(io.fan_speed_value);
    kb.need_reduce_fan_speed = false;
  }
  if (kb.need_increase_fan_speed) {
    io.fan_speed_value = io.fan_speed_value + io.fan_speed_step;
    if (io.fan_speed_value > io.fan_speed_max) {
      io.fan_speed_value = io.fan_speed_max;
    }
    io.fan_update(io.fan_speed_value);
    erom.write_fan_value(io.fan_speed_value);
    kb.need_increase_fan_speed = false;
  }
  if (kb.need_reduce_backlight) {
    if (io.backlight_value > io.backlight_min) {
      io.backlight_value = io.backlight_value - io.backlight_step;
    } else {
      io.backlight_value = 0;
    }
    io.backlight_update(io.backlight_value);
    erom.write_backlight_value(io.backlight_value);
    kb.need_reduce_backlight = false;
  }
  if (kb.need_increase_backlight) {
    if (io.backlight_value >= io.backlight_max) {
      io.backlight_value = io.backlight_max;
    } else {
      io.backlight_value = io.backlight_value + io.backlight_step;
    }
    io.backlight_update(io.backlight_value);
    erom.write_backlight_value(io.backlight_value);
    kb.need_increase_backlight = false;
  }
  if (loop_count == 1) {
    io.battery_voltage_update();
  } else if (loop_count == 2) {
    io.update_charge_status();
  } else if (loop_count == 3) {
    io.read_hdmi_status();
  } else if (loop_count == 6) {
    if (touch.is_touched) {
  } else if (loop_count == 4) {
    ws.calculate(io.battery_percent, 0, kb.last_key, touch.is_touched, kb.capslock, erom.fan_pwm_value, erom.backlight_pwm_value);
  } else if (loop_count == 5) {
    ws.update();
      touch.is_touched--;
      iic_display.update(mouse_x, mouse_y, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key);
    } else if (kb.mouse_moved) {
      kb.mouse_moved = false;
      iic_display.update(mouse_x, mouse_y, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key);
    } else {
      iic_display.update(0, 0, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key);
    }
  } else if (loop_count == 7) {
    iic_display.flash();
  }
  loop_count++;
  if (loop_count >= loop_count_max) {
    loop_count = 0;
  }
#if debug
  loop_count_debug++;
  Serial1.print("loop finish.loop count is ");
  Serial1.println(loop_count_debug);
  Serial1.print("This loop use : ");
  Serial1.println((rp2040.getCycleCount64() - time_now) / 100000);
  Serial1.println("_____________________________________________________________________________________");
  // delay(300);
  if (!digitalRead(gpio16)) {
    if (allwaysdelay)
    {
      allwaysdelay = 0;
    }
    else
    {
      allwaysdelay = 1000;
    }
    while (!digitalRead(gpio16))
    {
      delay(10);
    }
    Serial1.print("loop delay is : ");
    Serial1.println(allwaysdelay);
    delay(1000);
  }
  if (!digitalRead(gpio17)) {
    delay(1000);
  }
  if (!digitalRead(gpio18)) {
    delay(3000);
  }
  if (!digitalRead(gpio19)) {
    delay(5000);
  }
  if (!digitalRead(gpio20)) {
    delay(8000);
  }
  delay(allwaysdelay + 1);
#endif
  io.led2_flip();
}
/*
  void setup1() {
  delay(1000);
  rp2040.enableDoubleResetBootloader();
  }
  void loop1() {
  if (BOOTSEL) {
    delay(500);
    rp2040.reboot();
  }
  }
*/
