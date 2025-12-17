/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 13:40:25
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-10-10 21:36:25
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#include "ws2812_config.h"

Adafruit_NeoPixel pixels(4, ws2812, NEO_GRB + NEO_KHZ800);

bool WS2812::begin(uint8_t func1, uint8_t func2, uint8_t func3, uint8_t func4)
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 12; j++)
    {
      rgb[i][j] = 0;
    }
  }
  if (func1 > 0 && func1 < 12)
  {
    led_function[0] = func1;
#if debug
    Serial1.print("led1 function set to : ");
    Serial1.println(func1);
#endif
  }
  else
  {
    led_function[0] = led_function_default[0];
#if debug
    Serial1.print("led1 function wrong,wrong function is : ");
    Serial1.println(func1);
#endif
    return false;
  }
  if (func2 > 0 && func2 < 12)
  {
    led_function[1] = func2;
#if debug
    Serial1.print("led2 function set to : ");
    Serial1.println(func2);
#endif
  }
  else
  {
    led_function[1] = led_function_default[1];
#if debug
    Serial1.print("led2 function wrong,wrong function is : ");
    Serial1.println(func2);
#endif
    return false;
  }
  if (func3 > 0 && func3 < 12)
  {
    led_function[2] = func3;
#if debug
    Serial1.print("led3 function set to : ");
    Serial1.println(func3);
#endif
  }
  else
  {
    led_function[2] = led_function_default[2];
#if debug
    Serial1.print("led3 function wrong,wrong function is : ");
    Serial1.println(func3);
#endif
    return false;
  }
  if (func4 > 0 && func4 < 12)
  {
    led_function[3] = func4;
#if debug
    Serial1.print("led4 function set to : ");
    Serial1.println(func4);
#endif
  }
  else
  {
    led_function[3] = led_function_default[3];
#if debug
    Serial1.print("led4 function wrong,wrong function is : ");
    Serial1.println(func4);
#endif
    return false;
  }
  pixels.begin();
  return true;
}
bool WS2812::calculate(uint8_t battery_percent, bool error, bool keyboard_signal, bool touch_singnal, bool capslock, uint8_t fan_speed, uint8_t backlight_value)
{

  if (breath_up_or_down)
  {
    if (breath >= breath_speed)
    {
      breath_up_or_down = !breath_up_or_down;
      breath = breath_speed;
    }
    else
    {
      breath++;
    }
  }
  else
  {
    if (breath != 0)
    {
      breath--;
    }
    else
    {
      breath_up_or_down = !breath_up_or_down;
      breath = 0;
    }
  }
  if (led_function[0] == 1 || led_function[1] == 1 || led_function[2] == 1 || led_function[3] == 1)
  {
// 1 错误指示灯
#if debug
    Serial1.println("ws2812 func 1 : error led");
#endif
    if (error)
    {
      rgb[0][1] = 10;
      rgb[1][1] = 0;
      rgb[2][1] = 0;
    }
    else
    {
      rgb[0][1] = 0;
      rgb[1][1] = 10;
      rgb[2][1] = 0;
    }
  }
  if (led_function[0] == 2 || led_function[1] == 2 || led_function[2] == 2 || led_function[3] == 2)
  {
    // 2 键盘信号指示灯
#if debug
    Serial1.println("ws2812 func 2 : keyboard led");
#endif
    if (keyboard_signal)
    {
      rgb[0][2] = 0;
      rgb[1][2] = 0;
      rgb[2][2] = 10;
    }
    else
    {
      if (rgb[2][2] > 0)
      {
        if (breath == breath_speed / 2)
        {
          rgb[0][2] = 0;
          rgb[1][2] = 0;
          rgb[2][2] = rgb[2][2] - 1;
        }
      }
      else
      {
        rgb[0][2] = 0;
        rgb[1][2] = 10;
        rgb[2][2] = 0;
      }
    }
  }
  if (led_function[0] == 3 || led_function[1] == 3 || led_function[2] == 3 || led_function[3] == 3)
  {
    // 3 触摸信号指示灯
#if debug
    Serial1.println("ws2812 func 3 : touch led");
#endif
    if (touch_singnal)
    {
      rgb[0][3] = 0;
      rgb[1][3] = 0;
      rgb[2][3] = 10;
    }
    else
    {
      if (rgb[2][3] > 0)
      {
        if (breath == breath_speed / 2)
        {
          rgb[0][3] = 0;
          rgb[1][3] = 0;
          rgb[2][3] = rgb[2][3] - 1;
        }
      }
      else
      {
        rgb[0][3] = 0;
        rgb[1][3] = 10;
        rgb[2][3] = 0;
      }
    }
  }
  if (led_function[0] == 4 || led_function[1] == 4 || led_function[2] == 4 || led_function[3] == 4)
  {
    // 4 capslock指示灯
#if debug
    Serial1.println("ws2812 func 4 : acapslock led");
#endif
    if (capslock)
    {
      rgb[0][4] = 10;
      rgb[1][4] = 0;
      rgb[2][4] = 0;
    }
    else
    {
      rgb[0][4] = 0;
      rgb[1][4] = 10;
      rgb[2][4] = 0;
    }
  }
  if (led_function[0] == 5 || led_function[1] == 5 || led_function[2] == 5 || led_function[3] == 5)
  {
    // 5 红色呼吸灯
#if debug
    Serial1.println("ws2812 func 5 : red breath led");
#endif
    rgb[0][5] = breath;
    rgb[1][5] = 0;
    rgb[2][5] = 0;
  }
  if (led_function[0] == 6 || led_function[1] == 6 || led_function[2] == 6 || led_function[3] == 6)
  {
    // 6 绿色呼吸灯
#if debug
    Serial1.println("ws2812 func 6 : green breath led");
#endif
    rgb[1][6] = breath;
    rgb[0][6] = 0;
    rgb[2][6] = 0;
  }
  if (led_function[0] == 7 || led_function[1] == 7 || led_function[2] == 7 || led_function[3] == 7)
  {
    // 7 蓝色呼吸灯
#if debug
    Serial1.println("ws2812 func 7 : blue breath led");
#endif
    rgb[2][7] = breath;
    rgb[0][7] = 0;
    rgb[1][7] = 0;
  }
  if (led_function[0] == 8 || led_function[1] == 8 || led_function[2] == 8 || led_function[3] == 8)
  {
    // 8 风扇速度反馈
#if debug
    Serial1.println("ws2812 func 8 : fan speed led");
#endif
    if (fan_speed >= 80)
    {
      rgb[0][8] = 10;
      rgb[1][8] = 0;
      rgb[2][8] = 0;
    }
    else if (fan_speed == 0)
    {
      rgb[0][8] = 0;
      rgb[1][8] = 0;
      rgb[2][8] = 10;
    }
    else
    {
      rgb[0][8] = 0;
      rgb[1][8] = fan_speed / 10;
      rgb[2][8] = 0;
    }
  }
  if (led_function[0] == 9 || led_function[1] == 9 || led_function[2] == 9 || led_function[3] == 9)
  {
    // 9 背光电压反馈
#if debug
    Serial1.println("ws2812 func 9 : backlight led");
#endif
    if (backlight_value >= 80)
    {
      rgb[0][9] = 10;
      rgb[1][9] = 0;
      rgb[2][9] = 0;
    }
    else if (backlight_value == 0)
    {
      rgb[0][9] = 0;
      rgb[1][9] = 0;
      rgb[2][9] = 10;
    }
    else
    {
      rgb[0][9] = 0;
      rgb[1][9] = backlight_value / 10;
      rgb[2][9] = 0;
    }
  }
  if (led_function[0] == 10 || led_function[1] == 10 || led_function[2] == 10 || led_function[3] == 10)
  {
    // 10 随机数
#if debug
    Serial1.println("ws2812 func 10 : random led");
#endif
    rgb[0][10] = random(3);
    rgb[1][10] = random(3);
    rgb[2][10] = random(3);
  }
  if (led_function[0] == 11 || led_function[1] == 11 || led_function[2] == 11 || led_function[3] == 11)
  {
    // 11 电量
#if debug
    Serial1.println("ws2812 func 11 : battery led");
#endif
    if (battery_percent >= 80)
    {
      rgb[0][11] = 0;
      rgb[1][11] = 10;
      rgb[2][11] = 0;
    }
    else if (battery_percent <= 20)
    {
      rgb[0][11] = 10;
      rgb[1][11] = 0;
      rgb[2][11] = 0;
    }
    else
    {
      rgb[0][11] = 0;
      rgb[1][11] = 0;
      rgb[2][11] = battery_percent / 10;
    }
  }
#if debug
  Serial1.println("led value : r/g/b");
  for (int i = 0; i < 12; i++)
  {
    Serial1.print("func ");
    Serial1.print(i);
    Serial1.print(" : ");
    for (int j = 0; j < 3; j++)
    {
      Serial1.print(rgb[j][i]);
      Serial1.print("/");
    }
    Serial1.println(";");
  }
#endif
#if debug
  Serial1.print("led calculate finish");
#endif
  return true;
}
bool WS2812::update()
{
  pixels.clear();
  pixels.setPixelColor(0, rgb[0][led_function[0]], rgb[1][led_function[0]], rgb[2][led_function[0]]);
  pixels.setPixelColor(1, rgb[0][led_function[1]], rgb[1][led_function[1]], rgb[2][led_function[1]]);
  pixels.setPixelColor(2, rgb[0][led_function[2]], rgb[1][led_function[2]], rgb[2][led_function[2]]);
  pixels.setPixelColor(3, rgb[0][led_function[3]], rgb[1][led_function[3]], rgb[2][led_function[3]]);
  pixels.show();
  #if debug
  Serial1.print("led update finish");
#endif
  return true;
}
bool WS2812::set_led_function(uint8_t num, uint8_t func)
{
  if (num > 0 && num < 5)
  {
    if (func > 0 && func < 12)
    {
      led_function[num - 1] = func;
#if debug
      Serial1.print("led ");
      Serial1.print(num);
      Serial1.print(" function set to");
      Serial1.print(func);
#endif
    }
    else
    {
#if debug
      Serial1.print("error function : ");
      Serial.println(func);
#endif
      return false;
    }
  }
  else
  {
#if debug
    Serial1.print("error on led number,number is : ");
    Serial1.println(num);
    return false;
#endif
  }
  return true;
}

bool WS2812::direct_led_set(uint8_t red[4], uint8_t green[4], uint8_t blue[4])
{
  pixels.clear();
  for (int i = 0; i < 4; i++)
  {
    pixels.setPixelColor(i, red[i], green[i], blue[1]);
  }
  pixels.show();
#if debug
  Serial1.print("set led color directly");
#endif
  return 0;
}