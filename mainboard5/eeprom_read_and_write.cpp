/*
   @Author: 无序熵增
   @Date: 2025-07-25 13:26:15
   @LastEditors: 无序熵增
   @LastEditTime: 2025-09-17 20:30:43
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
#include "eeprom_read_and_write.h"

bool EEPROM_CONFIG::begin()
{
  EEPROM.begin(eeprom_space);
#if debug
  Serial1.print("the eeprom flag is : ");
  Serial1.println(setup_flag);
#endif
  if (EEPROM.read(setup_flag_address) == setup_flag)
  {
#if debug
    Serial1.println("not first time read eeprom");
#endif
  }
  else
  {
#if debug
    Serial1.println("first time read eeprom");
#endif
    EEPROM.write(fan_pwm_value_address, fan_pwm_value_default);
    EEPROM.write(backlight_pwm_value_address, backlight_pwm_value_default);
    EEPROM.write(ws2812_func_address[0], ws2812_func[0]);
    EEPROM.write(ws2812_func_address[1], ws2812_func[1]);
    EEPROM.write(ws2812_func_address[2], ws2812_func[2]);
    EEPROM.write(ws2812_func_address[3], ws2812_func[3]);
    EEPROM.write(setup_flag_address, setup_flag);
    EEPROM.commit();
#if debug
    Serial1.println("eeprom first time setup finish");
#endif
  }
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
