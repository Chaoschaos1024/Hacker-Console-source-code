/*
   @Author: 无序熵增
   @Date: 2025-07-25 12:04:40
   @LastEditors: 无序熵增
   @LastEditTime: 2025-12-09 14:07:29
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
#include "ssd1306.h"
#include "config.h"

Adafruit_SSD1306 display(iic_display_width, iic_display_height, &Wire1, iic_display_reset);

bool IIC_DISPLAY::begin()
{
  screen_exist_test();
  if (ssd1306_screen_exist)
  {
    display.begin(SSD1306_SWITCHCAPVCC, iic_display_address);
    display.setRotation(2);
    display.clearDisplay();
    display.display();
    display.drawBitmap(0, 0, bootup_logo, bootup_logo_width, bootup_logo_hight, 1);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);
    display.setCursor(35, 0);
    display.print("HACKER ");
    display.print("COMSOLE");
    display.setCursor(35, 10);
    display.print("Ver:");
    display.print(version);
#if debug
    display.print("(debug)");
#endif
    display.setFont(&front1);
    display.setCursor(35, 24);
    display.print("Compile Date:");
    display.setCursor(35, 30);
    display.print(__DATE__);
    display.print(":");
    display.print(__TIME__);
    display.display();
    display.setRotation(1);
    return true;
  }
  else
  {
    return false;
  }
}
bool IIC_DISPLAY::screen_exist_test()
{
  Wire1.beginTransmission(iic_display_address);
  int error =Wire1.endTransmission();

  if (error == 0)
  {
#if debug
    Serial1.print("SSD1306 screen is connected");
#endif
    ssd1306_screen_exist = true;
    return true;
  }
  else if (error == 4)
  {
#if debug
    Serial1.print("SSD1306 screen is connected");
#endif
    ssd1306_screen_exist = true;
    return true;
  }
  else
  {
#if debug
    Serial1.print("no SSD1306");
#endif
    ssd1306_screen_exist = false;
    return false;
  }
  return false;
}
bool IIC_DISPLAY::update(unsigned int x, unsigned int y, bool charge_or_not, bool charge_done_or_not, unsigned int battery_percent, unsigned int battery_voltage, bool screen_enabled, uint8_t backlight_duty_cycle, uint8_t fan_duty_cycle, unsigned int keyboard_status, unsigned int last_key)
{
  if (ssd1306_screen_exist)
  {
    display.clearDisplay();
    if (x | y)
    {
      real_x = x;
      real_y = y;
      x = map(x, 0, 32767, 0, logo_width);
      y = map(y, 0, 32767, 0, logo_hight);
      display.drawCircle(x, y, 1, SSD1306_WHITE);
    }
    else
    {
      display.drawBitmap(0, 0, heart_beat_wave[heart_beat_frame], logo_width, logo_hight, 1);
#if debug
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 8);
      display.cp437(true);
      display.print(heart_beat_frame);
#endif
      heart_beat_frame++;
      if (heart_beat_frame > heart_beat_frame_max)
      {
        heart_beat_frame = 0;
      }
    }
    if (charge_or_not)
    {
      display.drawBitmap(0, 24, battery_logo_charge, battery_logo_width, battery_logo_hight, 1);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      if (charge_done_or_not)
      {
        display.print("Charge OK");
      }
      else
      {
        display.print("Charging");
      }
    }
    else
    {
      display.drawBitmap(0, 24, battery_logo_empty, battery_logo_width, battery_logo_hight, 1);
      int battery_pixel = map(battery_percent, 0, 100, 2, 28);
      for (int i = 2; i < battery_pixel; i++)
      {
        display.drawPixel(i, 26, SSD1306_WHITE);
        display.drawPixel(i, 27, SSD1306_WHITE);
        display.drawPixel(i, 28, SSD1306_WHITE);
        display.drawPixel(i, 29, SSD1306_WHITE);
      }
      if (battery_percent >= 100)
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print("Full");
      }
      else if (battery_percent >= 75)
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print(battery_percent);
        display.print("%");
      }
      else if (battery_percent >= 50)
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print(battery_percent);
        display.print("%");
      }
      else if (battery_percent >= 25)
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print(battery_percent);
        display.print("%");
      }
      else if (battery_percent >= 10)
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print(battery_percent);
        display.print("%");
      }
      else
      {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 37);
        display.cp437(true);
        display.print(battery_percent);
        display.print("%");
      }
    }
#if debug
    display.print("/");
    display.print(battery_voltage / 10);
#endif
    display.drawLine(0, 40, 31, 40, SSD1306_WHITE);
    display.setCursor(0, 46);
    display.setTextSize(1);
    display.print("HDMI:");
    if (screen_enabled)
    {
      display.print("YES");
    }
    else
    {
      display.print("NO");
    }
    display.setCursor(0, 54);
    display.setTextSize(1);
#if debug
    display.print("L:");
#else
    display.print("Light:");
#endif
    uint8_t i = backlight_duty_cycle;
#if debug
    display.print(i);
    display.print("/");
#endif
    i = map(backlight_duty_cycle, 80, 100, 0, 100);
    display.print(i);
    display.setCursor(0, 62);
    display.setTextSize(1);
    display.print("Fan:");
    i = fan_duty_cycle;
    display.print(i);
    display.drawLine(0, 65, 31, 65, SSD1306_WHITE);
    display.setCursor(0, 71);
    display.setTextSize(1);
    display.print("X:");
    display.println(real_x);
    display.print("Y:");
    display.println(real_y);

    if (keyboard_status)
    {
      display.print("K:");
      display.println(key_massage_to_show[last_key]);
    }
    int display_y = 86;
    display.drawLine(0, display_y, 31, display_y, SSD1306_WHITE);
    display.setCursor(0, display_y + 6);
    display.setTextSize(1);
#if debug
    display.println("kbs=");
    display.println(keyboard_status);
#else
    display.println("TIPS:");
    if (keyboard_status == 1)
    {
      display.println("STICK");
      display.print("+ ");
    }
    else if (keyboard_status > 1)
    {
      display.println("MOU L+R");
      display.print("+ ");
    }
    if (tip_massage < tip_massage_max / 4)
    {
      display.println("F1");
      display.println("=");
      display.println("light up");
    }
    else if (tip_massage < tip_massage_max / 2)
    {
      display.println("F2");
      display.println("=");
      display.println("light down");
    }
    else if (tip_massage < tip_massage_max / 4 * 3)
    {
      display.println("F3");
      display.println("=");
      display.println("fan up");
    }
    else if (tip_massage < tip_massage_max)
    {
      display.println("F4");
      display.println("=");
      display.println("fan down");
    }
    tip_massage++;

    if (tip_massage >= tip_massage_max)
    {
      tip_massage = 0;
    }
#endif
#if debug
    Serial1.println("calculate little diaplay.");
#endif
    return true;
  }
  else
  {
    return false;
  }
  return false;
}
bool IIC_DISPLAY::flash()
{
  if (ssd1306_screen_exist)
  {
    display.display();
    return true;
  }
  else
  {
    return false;
  }
#if debug
  Serial1.println("flash little diaplay.");
#endif
  return false;
}
