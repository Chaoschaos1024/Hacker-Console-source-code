/*
   @Author: 无序熵增
   @Date: 2025-07-24 17:59:36
   @LastEditors: 无序熵增
   @LastEditTime: 2025-12-17 22:14:01
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
uint8_t loop_count_max = 100;

unsigned int mouse_x = 0;
unsigned int mouse_y = 0;

#if debug
unsigned int delay_time = 0;
int allwaysdelay = 0;
#endif
long long int loop_count_debug = 0;
void setup()
{
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
  io.set_battery_100_percent_value(erom.read_battery_max());
  io.set_battery_0_percent_value(erom.read_battery_min());
  io.charging_battery_max = erom.read_charging_battery_max();
  io.led1_flip();
  io.fan_setup(erom.read_fan_value());
  io.led1_flip();
  io.backlight_setup(erom.read_backlight_value());
  io.led1_flip();
  ws.begin(erom.read_led_function(1), erom.read_led_function(2), erom.read_led_function(3), erom.read_led_function(4));
  io.led1_flip();
  iic_display.begin(erom.device_version);
  while (!io.pi_main_power_read())
  {
    io.pi_main_power_read();
  }
  while (!io.battery_voltage_update())
  {
    io.battery_voltage_update();
  }
  delay(800);
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

void loop()
{
#if debug
  long long int time_now = rp2040.getCycleCount64();
  Serial1.print("loop begin. loop count : ");
  Serial1.print(loop_count);
  Serial1.print("/");
  Serial1.println(loop_count_max);
#endif
  kb.routine(mouse_x, mouse_y); //
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
  if (touch.is_touched)
  {
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
  }
  else
  {
#if debug
    Serial1.println("no touch panel signal");
#endif
  }
  if (kb.need_reduce_fan_speed)
  {
    if (io.fan_speed_value > io.fan_speed_min)
    {
      io.fan_speed_value = io.fan_speed_value - io.fan_speed_step;
    }
    else
    {
      io.fan_speed_value = 0;
    }
    io.fan_update(io.fan_speed_value);
    erom.write_fan_value(io.fan_speed_value);
    kb.need_reduce_fan_speed = false;
  }
  if (kb.need_increase_fan_speed)
  {
    io.fan_speed_value = io.fan_speed_value + io.fan_speed_step;
    if (io.fan_speed_value > io.fan_speed_max)
    {
      io.fan_speed_value = io.fan_speed_max;
    }
    io.fan_update(io.fan_speed_value);
    erom.write_fan_value(io.fan_speed_value);
    kb.need_increase_fan_speed = false;
  }
  if (kb.need_reduce_backlight)
  {
    if (io.backlight_value > io.backlight_min)
    {
      io.backlight_value = io.backlight_value - io.backlight_step;
    }
    else
    {
      io.backlight_value = 0;
    }
    io.backlight_update(io.backlight_value);
    erom.write_backlight_value(io.backlight_value);
    kb.need_reduce_backlight = false;
  }
  if (kb.need_increase_backlight)
  {
    if (io.backlight_value >= io.backlight_max)
    {
      io.backlight_value = io.backlight_max;
    }
    else
    {
      io.backlight_value = io.backlight_value + io.backlight_step;
    }
    io.backlight_update(io.backlight_value);
    erom.write_backlight_value(io.backlight_value);
    kb.need_increase_backlight = false;
  }
  /*
    if (loop_count == 1)
    {
    io.battery_voltage_update();
    if (! io.charge_or_not)
    {
      if (io.battery_voltage > io.battery_100_percent)
      {
        erom.write_battery_max(io.battery_voltage);
        io.battery_100_percent = io.battery_voltage;
      }
      if (io.battery_voltage < io.battery_0_percent && io.battery_voltage > 0)
      {
        if (io.pi_main_power_voltage >= io.pi_main_power_voltage_min && io.pi_main_power_voltage != 0)
        {
          erom.write_battery_min(io.battery_voltage);
          io.battery_0_percent = io.battery_voltage;
        }
      }
    }
    }
    else if (loop_count == 2)
    {
    io.update_charge_status();
    if (io.charge_done_or_not)
    { if (io.battery_voltage > erom.read_charging_battery_max())
      {
        erom.write_charging_battery_max(io.battery_voltage);
      }
    }
    }*/
  if (loop_count == 1)
  {
    io.battery_voltage_update();

    if (!io.charge_or_not)
    {
      /* ===================== 100% 自校准 ===================== */
      if (io.battery_voltage >
          io.battery_100_percent + BATTERY_CALIB_DELTA_MV)
      {
        io.battery_max_stable_cnt++;
        if (io.battery_max_stable_cnt >= BATTERY_CALIB_STABLE_CNT &&
            (loop_count_debug - io.last_battery_max_write_tick) >
            BATTERY_CALIB_WRITE_INTERVAL)
        {
          erom.write_battery_max(io.battery_voltage);
          io.battery_100_percent = io.battery_voltage;
          io.last_battery_max_write_tick = loop_count_debug;
          io.battery_max_stable_cnt = 0;
        }
      }
      else
      {
        io.battery_max_stable_cnt = 0;
      }

      /* ===================== 0% 自校准 ===================== */
      if (io.battery_voltage <
          io.battery_0_percent - BATTERY_CALIB_DELTA_MV &&
          io.battery_voltage > 0 &&
          io.pi_main_power_voltage >= io.pi_main_power_voltage_min)
      {
        io.battery_min_stable_cnt++;
        if (io.battery_min_stable_cnt >= BATTERY_CALIB_STABLE_CNT &&
            (loop_count_debug - io.last_battery_min_write_tick) >
            BATTERY_CALIB_WRITE_INTERVAL)
        {
          erom.write_battery_min(io.battery_voltage);
          io.battery_0_percent = io.battery_voltage;
          io.last_battery_min_write_tick = loop_count_debug;
          io.battery_min_stable_cnt = 0;
        }
      }
      else
      {
        io.battery_min_stable_cnt = 0;
      }
    }
  }
  else if (loop_count == 2)
  {
    io.update_charge_status();

    if (io.charge_done_or_not)
    {
      uint16_t old_max = erom.read_charging_battery_max();

      if (io.battery_voltage > old_max + BATTERY_CALIB_DELTA_MV)
      {
        io.charging_max_stable_cnt++;
        if (io.charging_max_stable_cnt >= BATTERY_CALIB_STABLE_CNT &&
            (loop_count_debug - io.last_charging_max_write_tick) >
            BATTERY_CALIB_WRITE_INTERVAL)
        {
          erom.write_charging_battery_max(io.battery_voltage);
          io.last_charging_max_write_tick = loop_count_debug;
          io.charging_max_stable_cnt = 0;
        }
      }
      else
      {
        io.charging_max_stable_cnt = 0;
      }
    }
  }

  else if (loop_count == 3)
  {
    io.read_hdmi_status();
  }
  else if (loop_count == 4)
  {
    ws.calculate(io.battery_percent, 0, kb.last_key, touch.is_touched, kb.capslock, erom.fan_pwm_value, erom.backlight_pwm_value);
  }
  else if (loop_count == 5)
  {
    ws.update();
  }
  else if (loop_count == 6)
  {
    iic_display.clear();
  }
  else if (loop_count == 7)
  {
    if (touch.is_touched)
    {
      touch.is_touched--;
      iic_display.update(mouse_x, mouse_y, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key, erom.battery_max, erom.battery_min, erom.charging_battery_max, io.pi_main_power_voltage);
    }
    else if (kb.mouse_moved)
    {
      kb.mouse_moved = false;
      iic_display.update(mouse_x, mouse_y, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key, erom.battery_max, erom.battery_min, erom.charging_battery_max, io.pi_main_power_voltage);
    }
    else
    {
      iic_display.update(0, 0, io.charge_or_not, io.charge_done_or_not, io.battery_percent, io.battery_voltage, io.screen_enabled, io.backlight_value, io.fan_speed_value, kb.keyboard_connect, kb.last_key, erom.battery_max, erom.battery_min, erom.charging_battery_max, io.pi_main_power_voltage);
    }
  }
  else if (loop_count == 8)
  {
    iic_display.flash();
  }
  else if (loop_count == 9)
  {
    io.pi_main_power_read();
  }
#if debug
  else if (loop_count == 10)
  {
    erom.eeprom_report();
  }
#endif
  else
  {
    loop_count_max = loop_count;
    loop_count = 0;
#if debug
    Serial1.println("loop count is set to zero!!!");
#endif
  }
  loop_count++;
#if debug
  loop_count_debug++;
  Serial1.print("loop finish.loop count is ");
  Serial1.println(loop_count_debug);
  Serial1.print("This loop use : ");
  Serial1.println((rp2040.getCycleCount64() - time_now) / 100000);
  Serial1.println("_____________________________________________________________________________________");
  // delay(300);
  if (!digitalRead(gpio16))
  {
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
  if (!digitalRead(gpio17))
  {
    delay(1000);
  }
  if (!digitalRead(gpio18))
  {
    delay(3000);
  }
  if (!digitalRead(gpio19))
  {
    delay(5000);
  }
  if (!digitalRead(gpio20))
  {
    delay(8000);
  }
  delay(allwaysdelay + 1);
#endif
  io.led2_flip();
}

/*void setup1() {
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
