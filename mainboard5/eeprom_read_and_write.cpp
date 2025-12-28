/*
   @Author: 无序熵增
   @Date: 2025-07-25 13:26:15
   @LastEditors: 无序熵增
   @LastEditTime: 2025-09-17 20:30:43
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
#include "eeprom_read_and_write.h"
#include"config.h"

bool EEPROM_CONFIG::begin()
{
  EEPROM.begin(eeprom_space);
#if debug
  Serial1.print("the eeprom flag is : ");
  Serial1.println(setup_flag);
#endif
  if (EEPROM.read(setup_flag_address) == setup_flag&&EEPROM.read(setup_flag_address+1) == setup_flag_ )
  {
#if debug
    Serial1.println("not first time read eeprom");
#endif
    read_battery_max();
    read_battery_min();
    read_charging_battery_max();
  }
  else
  {
#if debug
    Serial1.println("first time read eeprom");
#endif
    empty_eeprom();
    EEPROM.write(fan_pwm_value_address, fan_pwm_value_default);
    EEPROM.write(backlight_pwm_value_address, backlight_pwm_value_default);
    EEPROM.write(ws2812_func_address[0], ws2812_func[0]);
    EEPROM.write(ws2812_func_address[1], ws2812_func[1]);
    EEPROM.write(ws2812_func_address[2], ws2812_func[2]);
    EEPROM.write(ws2812_func_address[3], ws2812_func[3]);
    write_charging_battery_max(battery_max_default);
    write_battery_max(battery_max_default);
    write_battery_min(battery_min_default);
    EEPROM.write(setup_flag_address, setup_flag);
    EEPROM.write(setup_flag_address + 1, setup_flag_);
    EEPROM.commit();
#if debug
    Serial1.println("eeprom first time setup finish");
#endif
  }
  device_version = (float)EEPROM.read(setup_flag_address) + (float)EEPROM.read(setup_flag_address + 1) / 100;
  return true;
}

uint8_t EEPROM_CONFIG::read_fan_value()
{
  fan_pwm_value = EEPROM.read(fan_pwm_value_address);
  EEPROM.commit();
#if debug
  Serial1.print("read fan speed from eeprom.fan speed:");
  Serial1.println(fan_pwm_value);
#endif
  return fan_pwm_value;
}

float EEPROM_CONFIG::read_backlight_value()
{
  backlight_pwm_value = EEPROM.read(backlight_pwm_value_address) ;
  EEPROM.commit();
#if debug
  Serial1.print("read backlight value from eeprom.backlight value:");
  Serial1.println(backlight_pwm_value);
#endif
  return backlight_pwm_value;
}
bool EEPROM_CONFIG::write_fan_value(uint8_t value)
{
#if debug
  Serial1.println("writing fan value to eeprom");
#endif
  EEPROM.write(fan_pwm_value_address, value);
  fan_pwm_value = value;
  EEPROM.commit();
#if debug
  Serial1.print("write fan value to:");
  Serial1.println(value);
#endif
  return true;
}
bool EEPROM_CONFIG::write_backlight_value(uint8_t value)
{
#if debug
  Serial1.println("writing backlight value to eeprom");
#endif
  EEPROM.write(backlight_pwm_value_address, value);
  backlight_pwm_value = value;
  EEPROM.commit();
#if debug
  Serial1.print("write backlight value to:");
  Serial1.println(value);
#endif
  return true;
}
// 正确读取 uint16_t 数据（2字节）
uint16_t EEPROM_CONFIG::read_uint16_t(unsigned int addr) {
  uint16_t value = 0;

  // 读取低字节（小端序）或高字节（大端序），根据你的硬件决定
  // 常见的是小端序（低位在前）

  // 方法1：直接读取（小端序）
  byte lowByte = EEPROM.read(addr);
  byte highByte = EEPROM.read(addr + 1);
  value = (highByte << 8) | lowByte;

  // 方法2：使用指针（更清晰）
  // byte* p = (byte*)&value;
  // p[0] = EEPROM.read(addr);     // 低字节
  // p[1] = EEPROM.read(addr + 1); // 高字节

  return value;
}

// 正确写入 uint16_t 数据（2字节）
bool EEPROM_CONFIG::write_uint16_t(uint16_t num, unsigned int addr) {
  // 分解为2个字节（小端序）
  byte lowByte = num & 0xFF;        // 低8位
  byte highByte = (num >> 8) & 0xFF; // 高8位

  EEPROM.write(addr, lowByte);
  EEPROM.write(addr + 1, highByte);

  // 根据Arduino平台处理commit
#if defined(ESP8266) || defined(ESP32)
  // ESP系列需要commit
  EEPROM.commit();
#endif

  return true;
}
// 读取充电最大电池值
uint16_t EEPROM_CONFIG::read_charging_battery_max() {
  charging_battery_max = read_uint16_t(charging_battery_max_address);
  return charging_battery_max;
}

// 写入充电最大电池值
bool EEPROM_CONFIG::write_charging_battery_max(uint16_t battery) {
  charging_battery_max = battery;
  return write_uint16_t(battery, charging_battery_max_address);
}

// 读取最大电池值
uint16_t EEPROM_CONFIG::read_battery_max() {
  battery_max = read_uint16_t(battery_max_address);
  return battery_max;
}

// 写入最大电池值
bool EEPROM_CONFIG::write_battery_max(uint16_t battery) {
  battery_max = battery;
  return write_uint16_t(battery, battery_max_address);
}

// 读取最小电池值
uint16_t EEPROM_CONFIG::read_battery_min() {

  battery_min = read_uint16_t(battery_min_address);
  return battery_min;
}

// 写入最小电池值
bool EEPROM_CONFIG::write_battery_min(uint16_t battery) {
  battery_min = battery;
  return write_uint16_t(battery, battery_min_address);
}
uint8_t EEPROM_CONFIG::read_led_function(uint8_t num)
{
#if debug
  Serial1.println("reading led function from eeprom");
  Serial1.print("led ");
  Serial1.print(num);
  Serial1.print(" function is :");
#endif
  if (num > 4)
  {
#if debug
    Serial1.println(" ");
    Serial1.println("error on led num:");
    Serial1.println(num);
#endif
    return false;
  }
  num = num - 1;

  uint8_t func = EEPROM.read(ws2812_func_address[num]);
#if debug
  Serial1.println(func);
#endif
  EEPROM.commit();
  if (func == 0 || func > 11)
  {
#if debug
    Serial1.print("error on led func:");
    Serial1.println(func);
#endif
    write_led_function(num, ws2812_func[num]);
#if debug
    Serial1.print("write led ");
    Serial1.print(num);
    Serial1.print(" function ");
    Serial1.print(ws2812_func[num]);
    Serial1.println();
#endif
    return 0;
  }
  return func;
}
bool EEPROM_CONFIG::write_led_function(uint8_t num, uint8_t func)
{
  if (num > 0 && num < 5)
  {
    if (func > 0 && func < 12)
    {

      num = num - 1;
      EEPROM.write(ws2812_func_address[num], func);
      EEPROM.commit();
#if debug
      Serial1.print("write led ");
      Serial1.print(num + 1);
      Serial1.print(" function ");
      Serial1.println(func);
#endif
      return true;
    }
    else
    {
#if debug
      Serial1.print("wrong led function.function:");
      Serial1.println(func);
#endif
      return false;
    }
  }
  else
  {
#if debug
    Serial1.print("wrong led number.number:");
    Serial1.println(num);
#endif
    return false;
  }
  return false;
}
bool EEPROM_CONFIG::empty_eeprom()
{
#if debug
  Serial1.println("start to empty eeprom");
#endif
  for (int i = 0; i < eeprom_space; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
#if debug
  Serial1.println("eeprom now is empty");
#endif
  return true;
}

#if debug
bool EEPROM_CONFIG::eeprom_report()
{
  Serial1.print("version : ");
  Serial1.println( EEPROM.read(setup_flag_address) * 100 + EEPROM.read(setup_flag_address + 1));
  uint16_t valtage = read_charging_battery_max();
  Serial1.print("charging_battery_max : ");
  delay(1);
  Serial1.println(valtage);
  delay(1);
  Serial1.print("read_battery_max : ");
  delay(1);
  valtage = read_battery_max();
  Serial1.println(valtage);
  delay(1);
  Serial1.print("read_battery_min : ");
  delay(1);
  valtage =  read_battery_max();
  Serial1.println(valtage);
  delay(1);
  return true;
}
#endif
