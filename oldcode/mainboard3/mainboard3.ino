#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>
#if touch_enable
#include <MouseAbsolute.h>
#include <RP2040USB.h>
#include "tusb.h"
#include "class/hid/hid_device.h"
#else
#include <Mouse.h>
#endif
#include "GT911.h"
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RP2040_PWM.h"
#include "logo.h"
#include <Fonts/Picopixel.h>
#include "eeprom_address.h"
#include "short.h"
#include "io.h"
#include "config.h"
#include "keyboard_keymap.h"
#include "error.h"

// 小屏幕
#define iic_display_enable 1
#define iic_display_width 128
#define iic_display_height 32
#define iic_display_reset -1
#define iic_display_address 0x3c
#define front1 Picopixel
int iic_screen_status = 0;
bool iic_display_pixel_map[32][128];
int tip_massage = 0;
#define tip_massage_max 200
String parameter1_s = "";  // 和下方参数一起最多8个字符
int parameter1_i = 0;      // 和上方参数一起最多8个字符
String parameter2_s = "";  // 和下方参数一起最多8个字符
int parameter2_i = 0;      // 和上方参数一起最多8个字符
Adafruit_SSD1306 display(iic_display_width, iic_display_height, &Wire1, iic_display_reset);

// pwm
uint32_t pwm_pins[] = { fan_pwm, screen_backlight_pwm };
RP2040_PWM *pwm_instance[2];

// ws2812
Adafruit_NeoPixel pixels(4, ws2812, NEO_GRB + NEO_KHZ800);
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
int led_count = 0;
int rgb[3][30];

//风扇和背光
bool need_to_update_backlight = true;
float backlight_duty_cycle = 40.0;
float backlight_duty_cycle_new = 80.0;
bool need_to_update_fan = true;
float fan_duty_cycle = 50.0;
float fan_duty_cycle_new = 50.0;
bool screen_enabled = false;

//电量相关
bool charge_or_not = false;
bool charge_done_or_not = false;
unsigned long backlight_voltage = 0;
unsigned long battery_voltage = 0;
unsigned long main_voltage = 0;
unsigned long pi_3v3_voltage = 0;
#define battery_count_max 10
unsigned long long int battery_voltage_[battery_count_max];
int battery_count = 0;
int battery_percent = 0;

// 触摸屏
#if touch_enable
#define mouse_enable 1
#define touch_width 800
#define touch_height 480
#define x_times 41
#define y_times 68
GT911 tp = GT911(iic1_sda, iic1_scl, touch_int, touch_rst, touch_width, touch_height);
#endif
long mouse_x = 32767 / 2;
long mouse_y = 32767 / 2;
unsigned int touch_x = 0;
unsigned int touch_y = 0;
unsigned int touched = 0;
int touch_signal = 0;
bool last_time_touch_status = 0;
int touch_panel_status = 0;


//键盘相关
uint8_t keyboard_status = 0;  //没键盘为0，最初的键盘为1，新版键盘为2
uint8_t last_key = 0;
uint8_t last_key_x = 0;
uint8_t last_key_y = 0;
unsigned int keyboard_signal = 0;
int key_pressed = 0;

//错误测试
int any_error = 0;
int error_code[100];

//机器锁定
bool unlock = true;

//全局变量
int loop_count = 0;

void setup() {
  pinmode(screen_enable, output);
  digitalwrite(screen_enable, low);

#if debug
  Serial1.println("------------------------------------------------");
  Serial1.println("██   ██  █████   ██████ ██   ██  ██████  ██████  ███    ██ ███████  ██████  ██      ███████  ");
  Serial1.println("██   ██ ██   ██ ██      ██  ██  ██      ██    ██ ████   ██ ██      ██    ██ ██      ██ ");
  Serial1.println("███████ ███████ ██      █████   ██      ██    ██ ██ ██  ██ ███████ ██    ██ ██      █████ ");
  Serial1.println("██   ██ ██   ██ ██      ██  ██  ██      ██    ██ ██  ██ ██      ██ ██    ██ ██      ██ ");
  Serial1.println("██   ██ ██   ██  ██████ ██   ██  ██████  ██████  ██   ████ ███████  ██████  ███████ ███████ ");
  Serial1.print("设备版本号：");
  Serial1.println(version);
#endif
#if debug
  Serial1.println("iic初始化开始");
#endif
  wire1.setSDA(iic1_sda);
  wire1.setSCL(iic1_scl);
  wire1.begin();
#if debug
  Serial1.println("iic初始化完成");
#endif
#if debug
  Serial1.println("开始载入配置");
#endif
  eeprom.begin(eeprom_space);
  int first_time_setup = eeprom.read(setup_or_not_address);
  if (first_time_setup == setup_or_not_default_value) {

  } else {
#if debug
    Serial1.println("eeprom初次配置");
#endif
    eeprom.write(fan_duty_cycle_address, 0);
    eeprom.write(fan_duty_cycle_address + 1, 0);
    eeprom.write(fan_duty_cycle_address + 2, 0);
    eeprom.write(backlight_duty_cycle_address, 9);
    eeprom.write(backlight_duty_cycle_address + 1, 0);
    eeprom.write(backlight_duty_cycle_address + 2, 0);
    eeprom.write(setup_or_not_address, setup_or_not_default_value);
  }
  fan_duty_cycle_new = eeprom.read(fan_duty_cycle_address) * 10 + eeprom.read(fan_duty_cycle_address + 1) + eeprom.read(fan_duty_cycle_address + 2) * 0.1;
  backlight_duty_cycle_new = eeprom.read(backlight_duty_cycle_address) * 10 + eeprom.read(backlight_duty_cycle_address + 1) + eeprom.read(backlight_duty_cycle_address + 2) * 0.1;
  fan_duty_cycle = fan_duty_cycle_new;
  backlight_duty_cycle = backlight_duty_cycle_new;
#if debug
  Serial1.print("风扇转速百分比:");
  Serial1.println(fan_duty_cycle);
  Serial1.print("背光百分比:");
  Serial1.println(backlight_duty_cycle);
#endif
  eeprom.end();
#if debug
  Serial1.println("载入配置完成");
#endif
  pinmode(keyboard_int, input_pullup);
  pinmode(hdmi_active, input);
  pinmode(led1, output);
  pinmode(led2, output);
  pinmode(screen_enable, output);
  pinmode(charge_status, input_pullup);
  pinmode(charge_done_status, input_pullup);
  analogReadResolution(12);
#if debug
  Serial1.println("IO初始化完成");
#endif
  backlight_duty_cycle = backlight_duty_cycle_new;
  pwm_instance[0] = new RP2040_PWM(fan_pwm, fan_pwm_freq, fan_duty_cycle);
  pwm_instance[1] = new RP2040_PWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_duty_cycle);
  need_to_update_backlight = true;
  need_to_update_fan = true;
  if (digitalread(hdmi_active)) {
    if (screen_enabled != true) {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_duty_cycle);
      screen_enabled = true;
    }
  } else {
    if (screen_enabled != false) {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 0.0);
      screen_enabled = false;
    }
  }
#if debug
  Serial1.println("背光和屏幕初始化完成");
#endif
  wire1.beginTransmission(keyboard_iic_address);
  wire1.endTransmission(false);
  wire1.requestFrom(keyboard_iic_address, keyboard_iic_length);
  uint8_t iic_keyboard_report[keyboard_iic_length];
  for (int i = 0; i < keyboard_iic_length; i++) {
    iic_keyboard_report[i] = wire1.read();
  }
#if debug
  Serial1.println("键盘iic回复：");
  for (int i = 0; i < keyboard_iic_length; i++) {
    Serial1.print(i);
    Serial1.print(" : ");
    Serial1.println(iic_keyboard_report[i], HEX);
  }
  Serial1.println(" ");
#endif
  if (iic_keyboard_report[keyboard_iic_length - 1] == keyboard_iic_crc) {
    keyboard_status = 1;
    kb.begin();
#if debug
    Serial1.println("初始键盘已经安装");
#endif
  } else if (iic_keyboard_report[keyboard_iic_length - 1] == keyboard_iic_crc_new) {
    keyboard_status = 2;
    kb.begin();
#if debug
    Serial1.println("新版键盘已经安装");
#endif
  } else {
    keyboard_status = 0;
#if debug
    Serial1.println("未安装键盘");
#endif
  }
#if debug
  Serial1.println("键盘初始化完成");
#endif
#if touch_enable
  ma.begin();
  tp.begin();
  tp.setRotation(ROTATION_INVERTED);
  tp.setResolution(touch_width, touch_height);
#if debug
  Serial1.println("触摸屏初始化完成");
#endif
#else
  mo.begin();
#if debug
  Serial1.println("不处理触摸屏，使用常规键盘操作");
#endif
#endif
#if debug
  Serial1.println("初始化ws2812");
#endif
  pixels.begin();
  pixels.clear();
  pixels.show();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 30; j++) {
      rgb[i][j] = 0;
    }
  }
#if debug
  Serial1.println("ws2812初始化完成");
#endif
#if debug
  Serial1.println("副屏初始化");
#endif
  wire1.beginTransmission(iic_display_address);
  int iic_screen_status = !wire1.endTransmission();
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
  unsigned long long a = 0;
  unsigned long long j = 0;
  for (int i = 0; i < 100; i++) {
    j = analogread(battery) * 2 * 33000 / 4095;
    a = a + j;
    delay(20);
  }
  a = a / 100;
  battery_percent = map(a, battery_voltage_min, battery_voltage_max, 0, 100);
  if (battery >= 100) {
    battery_percent = 100;
  } else if (battery_percent <= 0) {
    battery_percent = 0;
  }
  display.setRotation(1);

#if debug
  Serial1.println("副屏初始化完成");
#endif
  for (int i = 0; i < 100; i++) {
    error_code[i] = 0;
  }
  for (int j = 0; j < battery_count_max; j++) {
    battery_voltage_[j] = 0;
  }
  digitalwrite(led1, HIGH);
}


void loop() {
#if touch_enable
#if debug
  Serial1.println("触摸程序启动");
#endif
  touch();
#if debug
  Serial1.println("触摸程序结束");
#endif
#endif

#if touch_enable
#if debug
  Serial1.println("使用触摸做鼠标映射的键盘程序启动");
#endif
  keyboard_with_touch();
#if debug
  Serial1.println("使用触摸做鼠标映射的键盘程序结束");
#endif
#else
#if debug
  Serial1.println("使用单独键盘的键盘键盘程序开始");
#endif
  Keyboard_with_out_touch();
#if debug
  Serial1.println("使用单独键盘的键盘键盘程序结束");
#endif
#endif
  update_fan();
  update_backlight();
  if (loop_count > other_issues) {
    hdmi_update();
    adc();
    set_ws2812();
    set_iic_screen();
    find_any_error();
    lock_up();
    loop_count = 0;
    digitalwrite(led2, !digitalread(led2));
  }
  loop_count++;
#if debug
  Serial1.println("---------------------------------------");
#endif
}

#if touch_enable
bool touch() {
#if debug
  Serial1.println("触摸程序开始运行");
#endif
  if (!digitalread(touch_int)) {
    tp.read();
    if (tp.isTouched) {
      touched = touched + 10;
      int max_size = 0;
      int whitch_touch_size_is_biggest = 0;
      if (tp.touches == 15) {
        int bad_point = 0;
        for (int i = 0; i < tp.touches; i++) {
          if (tp.points[i].x == 65535 & tp.points[i].y == 65535 & tp.points[i].size == 255) {
            bad_point = bad_point + 1;
          }
        }
        if (bad_point > 10) {
          touch_panel_status == 0;
          tp.begin();
          tp.setRotation(ROTATION_INVERTED);
          tp.setResolution(touch_width, touch_height);
          error_code[no_touch_panel] = no_touch_panel;
          return 1;
        }
      }
      for (int i = 0; i <= tp.touches; i++) {
        touch_panel_status == 1;
        if (tp.points[i].size > max_size) {
          max_size = tp.points[i].size;
          whitch_touch_size_is_biggest = i;
        }
      }
#if debug
      Serial1.print("touch point max size:");
      Serial1.println(max_size);
#endif
      touch_x = tp.points[whitch_touch_size_is_biggest].x;
      touch_y = tp.points[whitch_touch_size_is_biggest].y;
      mouse_x = touch_x * x_times;
      mouse_y = touch_y * y_times;
      ma.move(mouse_x, mouse_y, 0);
    } else {
#if debug
      Serial1.println("无触摸信号");
#endif
    }
  } else {
  }

#if debug
  Serial1.println("触摸程序运行完成");
#endif
  return 0;
}

bool keyboard_with_touch() {
  wire1.beginTransmission(keyboard_iic_address);
  wire1.endTransmission(false);
  wire1.requestFrom(keyboard_iic_address, keyboard_iic_length);
  uint8_t iic_report[keyboard_iic_length];
  for (int i = 0; i < keyboard_iic_length; i++) {
    iic_report[i] = wire1.read();
  }
  if (iic_report[keyboard_iic_length - 1] == keyboard_iic_crc) {
#if debug
    Serial1.println("使用旧键盘");
#endif

    if (iic_report[6] == 1 & iic_report[9] == 8) {
      unlock = false;
      return 0;
    }
    int mouse_dx = iic_report[0];
    int mouse_dy = iic_report[1];
    int command_key = iic_report[2];
    bool keyboard_matrix[7][11] = {
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    for (int i = 0; i < 11; i++) {
      keyboard_matrix[0][i] = iic_report[i + 3] & 0b01000000;
      keyboard_matrix[1][i] = iic_report[i + 3] & 0b00100000;
      keyboard_matrix[2][i] = iic_report[i + 3] & 0b00010000;
      keyboard_matrix[3][i] = iic_report[i + 3] & 0b00001000;
      keyboard_matrix[4][i] = iic_report[i + 3] & 0b00000100;
      keyboard_matrix[5][i] = iic_report[i + 3] & 0b00000010;
      keyboard_matrix[6][i] = iic_report[i + 3] & 0b00000001;
    }
    if (iic_report[2] == 66) {
      if (keyboard_matrix[6][0] != keyboard_matrix_last_time[6][0]) {
        if (keyboard_matrix[6][0] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new + 1;
          if (backlight_duty_cycle_new > backlight_duty_cycle_max) {
            backlight_duty_cycle = backlight_duty_cycle_max;
          }
          need_to_update_backlight = true;
          last_key = light_up;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[5][0] != keyboard_matrix_last_time[5][0]) {
        if (keyboard_matrix[5][0] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new - 1;
          if (backlight_duty_cycle_new < backlight_duty_cycle_min) {
            backlight_duty_cycle_new = backlight_duty_cycle_min;
          }
          need_to_update_backlight = true;
          last_key = light_down;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[4][0] != keyboard_matrix_last_time[4][0]) {
        if (keyboard_matrix[4][0] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new + fan_step;
          if (fan_duty_cycle_new > fan_duty_cycle_max) {
            fan_duty_cycle_new = fan_duty_cycle_max;
          }
          need_to_update_fan = true;
          last_key = fan_up;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[3][0] != keyboard_matrix_last_time[3][0]) {
        if (keyboard_matrix[3][0] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new - fan_step;
          if (fan_duty_cycle_new < fan_duty_cycle_min) {
            fan_duty_cycle_new = fan_duty_cycle_min;
          }
          need_to_update_fan = true;
          last_key = fan_down;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      delay(command_delay);
      return 0;
    }
    if (mouse_dx != 128 || mouse_dy != 128) {
      key_pressed = key_pressed + 10;
      mouse_x = mouse_x + mouse_dx * stick_times - 128 * stick_times;
      mouse_y = mouse_y + mouse_dy * stick_times - 128 * stick_times;
      if (mouse_x < 0) {
        mouse_x = 1;
      }
      if (mouse_x > 32767) {
        mouse_x = 32766;
      }
      if (mouse_y < 0) {
        mouse_y = 1;
      }
      if (mouse_y > 32767) {
        mouse_y = 32766;
      }
      ma.move(mouse_x, mouse_y, 0);
      keyboard_signal = keyboard_signal + 1;
    }
    for (int i = 0; i < 11; i++) {
      for (int j = 0; j < 7; j++) {
        if (keyboard_matrix[j][i] != keyboard_matrix_last_time[j][i]) {
          bool press_or_release = false;
          if (keyboard_matrix[j][i]) {
            press_or_release = true;
            key_pressed = key_pressed + 10;
          } else {
            press_or_release = false;
          }
          if (mouse_or_keyboard[j][i]) {
            if (press_or_release == true) {
              if (key_map[j][i] == MOUSE_LEFT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), MOUSE_LEFT, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
                last_key = MOUSE_LEFT;
              } else if (key_map[j][i] == MOUSE_RIGHT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), MOUSE_RIGHT, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
                last_key = MOUSE_RIGHT;
              } else if (key_map[j][i] == WHEEL_UP) {
                ma.move(mouse_x, mouse_y, 50);
                last_key = WHEEL_UP;
              } else if (key_map[j][i] == WHEEL_DOWN) {
                ma.move(mouse_x, mouse_y, -50);
                last_key = WHEEL_DOWN;
              }
            } else {
              if (key_map[j][i] == MOUSE_LEFT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
              } else if (key_map[j][i] == MOUSE_RIGHT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
              } else if (key_map[j][i] == WHEEL_UP) {
                // ma.move(mouse_x, mouse_y, 50);
              } else if (key_map[j][i] == WHEEL_DOWN) {
                // ma.move(mouse_x, mouse_y, -50);
              }
            }
          } else {
            if (key_map[j][i] == KEY_LEFT_SHIFT || key_map[j][i] == KEY_RIGHT_SHIFT) {
              if (press_or_release) {
                shift = true;
              } else {
                shift = false;
              }
            }
            if (key_map[j][i] == KEY_CAPS_LOCK) {
              if (press_or_release) {
                capslock = !capslock;
              }
            }
            if (capslock) {
              if (press_or_release) {
                kb.press(key_map_with_capslock[j][i]);
                last_key = key_map_with_capslock[j][i];
                keyboard_signal = keyboard_signal + 100;
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map_with_capslock[j][i]);
                last_key = key_map_with_capslock[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else if (shift) {
              if (press_or_release) {
                kb.press(key_map_with_shift[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map_with_shift[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else {
              if (press_or_release) {
                kb.press(key_map[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            }
          }
          keyboard_matrix_last_time[j][i] = keyboard_matrix[j][i];
        }
      }
    }

#if debug
    Serial1.print("iic_report-d-: ");
    for (int l = 0; l < 14; l++) {
      Serial1.print(iic_report[l]);
      Serial1.print("/");
    }
    Serial1.println("");
    Serial1.print("iic_report-b-: ");
    for (int l = 0; l < 14; l++) {
      Serial1.print(iic_report[l], BIN);
      Serial1.print("/");
    }
    Serial1.println("");
    Serial1.println("keyboard_matrix: ");
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 11; j++) {
        Serial1.print(keyboard_matrix[i][j]);
        Serial1.print("/");
      }
      Serial1.println("");
    }
    Serial1.println("");
#endif


  } else if (iic_report[keyboard_iic_length - 1] == keyboard_iic_crc_new) {
#if debug
    Serial1.println("使用新键盘");
#endif
    bool keyboard_matrix[7][12] = {
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    for (int i = 0; i < 12; i++) {
      keyboard_matrix[0][i] = iic_report[i + 2] & 0b01000000;
      keyboard_matrix[1][i] = iic_report[i + 2] & 0b00100000;
      keyboard_matrix[2][i] = iic_report[i + 2] & 0b00010000;
      keyboard_matrix[3][i] = iic_report[i + 2] & 0b00001000;
      keyboard_matrix[4][i] = iic_report[i + 2] & 0b00000100;
      keyboard_matrix[5][i] = iic_report[i + 2] & 0b00000010;
      keyboard_matrix[6][i] = iic_report[i + 2] & 0b00000001;
    }


#if debug
    Serial1.print("iic_report : ");
    for (int l = 0; l < keyboard_iic_length; l++) {
      Serial1.print(iic_report[l]);
      Serial1.print("/");
    }
    Serial1.println("");

    for (int a = 0; a < 7; a++) {
      for (int b = 0; b < (keyboard_iic_length - 2); b++) {
        if (keyboard_matrix[a][b]) {
          Serial1.print("shift : ");
          Serial1.println(shift);
          Serial1.print("capslock : ");
          Serial1.println(capslock);
          Serial1.print("char(shift) : \" ");
          Serial1.print(new_key_map_with_shift[a][b]);
          uint8_t c = new_key_map_with_shift[a][b];  //- '0';
          Serial1.println(" \"");
          Serial1.print("int : \" 0x");
          Serial1.print(c, HEX);
          Serial1.println(" \"");
          Serial1.print("翻译 : ");
          Serial1.println(key_massage_to_show[c]);
          Serial1.print("lastkey : ");
          Serial1.println(key_massage_to_show[last_key]);
          for (int d = 0; d < 9; d++) {
            // Serial1.print(key_massage_to_show[c][d]);
          }
          Serial1.print(" \"");
        } else {
        }
      }
    }
    Serial1.println();
#endif


    if (iic_report[1] == 66) {
#if debug
      Serial1.println("有特殊指令");
#endif
      if (keyboard_matrix[5][11] != new_keyboard_matrix_last_time[5][11]) {
        if (keyboard_matrix[5][11]) {
          backlight_duty_cycle_new = backlight_duty_cycle_new + 1;
          if (backlight_duty_cycle_new > backlight_duty_cycle_max) {
            backlight_duty_cycle = backlight_duty_cycle_max;
          }
          need_to_update_backlight = true;
          last_key = light_up;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[4][11] != keyboard_matrix_last_time[4][11]) {
        if (keyboard_matrix[4][11] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new - 1;
          if (backlight_duty_cycle_new < backlight_duty_cycle_min) {
            backlight_duty_cycle_new = backlight_duty_cycle_min;
          }
          need_to_update_backlight = true;
          last_key = light_down;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[3][11] != keyboard_matrix_last_time[3][11]) {
        if (keyboard_matrix[3][11] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new + fan_step;
          if (fan_duty_cycle_new > fan_duty_cycle_max) {
            fan_duty_cycle_new = fan_duty_cycle_max;
          }
          need_to_update_fan = true;
          last_key = fan_up;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[2][11] != keyboard_matrix_last_time[2][11]) {
        if (keyboard_matrix[2][11] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new - fan_step;
          if (fan_duty_cycle_new < fan_duty_cycle_min) {
            fan_duty_cycle_new = fan_duty_cycle_min;
          }
          need_to_update_fan = true;
          last_key = fan_down;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      delay(command_delay);
      return 0;
    } else if (iic_report[1] == 33) {
#if debug
      Serial1.println("没有特殊指令");
#endif
    }
    /*
    if (iic_report[0] == 0) {
#if debug
      Serial1.println("摇杆居中");
#endif
    } else if (iic_report[0] == 1) {
#if debug
      Serial1.println("摇杆右上");
#endif
      mouse_dx = 128 + 45;
      mouse_dy = 128 - 45;
    } else if (iic_report[0] == 2) {
#if debug
      Serial1.println("摇杆右下");
#endif
      mouse_dx = 128 + 45;
      mouse_dy = 128 + 45;
    } else if (iic_report[0] == 3) {
#if debug
      Serial1.println("摇杆左下");
#endif
      mouse_dx = 128 - 45;
      mouse_dy = 128 + 45;
    } else if (iic_report[0] == 4) {
#if debug
      Serial1.println("摇杆左上");
#endif
      mouse_dx = 128 - 45;
      mouse_dy = 128 - 45;
    } else if (iic_report[0] == 5) {
#if debug
      Serial1.println("摇杆向右");
#endif
      mouse_dx = 128 + 64;
      mouse_dy = 128;
    } else if (iic_report[0] == 6) {
#if debug
      Serial1.println("摇杆向左");
#endif
      mouse_dx = 64;
      mouse_dy = 128;
    } else if (iic_report[0] == 7) {
#if debug
      Serial1.println("摇杆向下");
#endif
      mouse_dx = 128;
      mouse_dy = 128 + 64;
    } else if (iic_report[0] == 8) {
#if debug
      Serial1.println("摇杆向上");
#endif
      mouse_dx = 128;
      mouse_dy = 64;
    } else {
#if debug
      Serial1.print("第一位错误");
#endif
    }
    */
    bool direction[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int direction_count = 0;
    int dx = 0;
    int dy = 0;
    if (iic_report != 0) {
      direction[0] = iic_report[0] & 0b10000000;  //上
      direction[1] = iic_report[0] & 0b01000000;  //右上
      direction[2] = iic_report[0] & 0b00100000;  //右
      direction[3] = iic_report[0] & 0b00010000;  //右下
      direction[4] = iic_report[0] & 0b00001000;  //下
      direction[5] = iic_report[0] & 0b00000100;  //左下
      direction[6] = iic_report[0] & 0b00000010;  //左
      direction[7] = iic_report[0] & 0b00000001;  //左上
      direction_count = direction[0] + direction[1] + direction[2] + direction[3] + direction[4] + direction[5] + direction[6] + direction[7];
#if debug
      Serial1.print("方向 ： ");
      Serial1.print(direction[0]);
      Serial1.print("/");
      Serial1.print(direction[1]);
      Serial1.print("/");
      Serial1.print(direction[2]);
      Serial1.print("/");
      Serial1.print(direction[3]);
      Serial1.print("/");
      Serial1.print(direction[4]);
      Serial1.print("/");
      Serial1.print(direction[5]);
      Serial1.print("/");
      Serial1.print(direction[6]);
      Serial1.print("/");
      Serial1.print(direction[7]);
      Serial1.print("/");

      Serial1.println();
      Serial1.print("方向计数 ： ");
      Serial1.println(direction_count);
#endif

      if (direction[0]) {
        dy = dy - 64;
#if debug
        Serial1.println("上");
#endif
      }
      if (direction[1]) {
        dx = dx + 45;
        dy = dy - 45;
#if debug
        Serial1.println(" 右上");
#endif
      }
      if (direction[2]) {
        dx = dx + 64;
#if debug
        Serial1.println("右");
#endif
      }
      if (direction[3]) {
        dx = dx + 45;
        dy = dy + 45;
#if debug
        Serial1.println("右下");
#endif
      }
      if (direction[4]) {
        dy = dy + 64;
#if debug
        Serial1.println("下");
#endif
      }
      if (direction[5]) {
        dx = dx - 45;
        dy = dy + 45;
#if debug
        Serial1.println("左下");
#endif
      }
      if (direction[6]) {
        dx = dx - 64;
#if debug
        Serial1.println("左");
#endif
      }
      if (direction[7]) {
        dx = dx - 45;
        dy = dy - 45;
#if debug
        Serial1.println("左上");
#endif
      }
      if (direction_count > 1) {
        dx = dx * 2 / 3;
        dy = dy * 2 / 3;
      }

#if debug
      Serial1.print("dx: ");
      Serial1.println(dx);
      Serial1.print("dy: ");
      Serial1.println(dy);
#endif
    }


    int mouse_dx = 128 + dx;
    int mouse_dy = 128 + dy;
#if debug
    Serial1.print("mouse_dx : ");
    Serial1.println(mouse_dx);
    Serial1.print("mouse_dy: ");
    Serial1.println(mouse_dy);
#endif
    if (mouse_dx != 128 || mouse_dy != 128) {
#if debug
      delay(1000);
#endif
      key_pressed = key_pressed + 10;
      mouse_x = mouse_x + mouse_dx * stick_times - 128 * stick_times;
      mouse_y = mouse_y + mouse_dy * stick_times - 128 * stick_times;
      if (mouse_x < 0) {
        mouse_x = 1;
      }
      if (mouse_x > 32767) {
        mouse_x = 32766;
      }
      if (mouse_y < 0) {
        mouse_y = 1;
      }
      if (mouse_y > 32767) {
        mouse_y = 32766;
      }
      ma.move(mouse_x, mouse_y, 0);
      keyboard_signal = keyboard_signal + 1;
    }




    for (int i = 0; i < 12; i++) {
      for (int j = 0; j < 7; j++) {
        if (keyboard_matrix[j][i] != new_keyboard_matrix_last_time[j][i]) {
          bool press_or_release = false;
          if (keyboard_matrix[j][i]) {
            press_or_release = true;
            key_pressed = key_pressed + 10;
          } else {
            press_or_release = false;
          }
          if (new_mouse_or_keyboard[j][i]) {
            if (press_or_release == true) {
              if (new_key_map[j][i] == MOUSE_LEFT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), MOUSE_LEFT, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
                last_key = MOUSE_LEFT;
              } else if (new_key_map[j][i] == MOUSE_RIGHT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), MOUSE_RIGHT, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
                last_key = MOUSE_RIGHT;
              } else if (new_key_map[j][i] == WHEEL_UP) {
                ma.move(mouse_x, mouse_y, 50);
                last_key = WHEEL_UP;
              } else if (new_key_map[j][i] == WHEEL_DOWN) {
                ma.move(mouse_x, mouse_y, -50);
                last_key = WHEEL_DOWN;
              }
            } else {
              if (new_key_map[j][i] == MOUSE_LEFT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
              } else if (new_key_map[j][i] == MOUSE_RIGHT) {
                CoreMutex m(&__usb_mutex);
                tud_task();
                if (tud_hid_ready()) {
                  tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, mouse_x, mouse_y, 0, 0);
                }
                tud_task();
              } else if (new_key_map[j][i] == WHEEL_UP) {
                // ma.move(mouse_x, mouse_y, 50);
              } else if (new_key_map[j][i] == WHEEL_DOWN) {
                // ma.move(mouse_x, mouse_y, -50);
              }
            }

          } else {
            if (new_key_map[j][i] == KEY_LEFT_SHIFT || new_key_map[j][i] == KEY_RIGHT_SHIFT) {
              if (press_or_release) {
                shift = true;
              } else {
                shift = false;
              }
            }
            if (new_key_map[j][i] == KEY_CAPS_LOCK) {
              if (press_or_release) {
                capslock = !capslock;
              }
            }
            if (capslock) {
              if (press_or_release) {
                kb.press(new_key_map_with_capslock[j][i]);
                last_key = new_key_map_with_capslock[j][i];
                keyboard_signal = keyboard_signal + 100;
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map_with_capslock[j][i]);
                last_key = new_key_map_with_capslock[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else if (shift) {
              if (press_or_release) {
                kb.press(new_key_map_with_shift[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map_with_shift[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else {
              if (press_or_release) {
                kb.press(new_key_map[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            }
          }
          new_keyboard_matrix_last_time[j][i] = keyboard_matrix[j][i];
        }
      }
    }
  }
  return 0;
}
#else
bool Keyboard_with_out_touch() {
  wire1.beginTransmission(keyboard_iic_address);
  wire1.endTransmission(false);
  wire1.requestFrom(keyboard_iic_address, keyboard_iic_length);
  uint8_t iic_report[keyboard_iic_length];
  for (int i = 0; i < keyboard_iic_length; i++) {
    iic_report[i] = wire1.read();
  }
  if (iic_report[keyboard_iic_length - 1] == keyboard_iic_crc) {
#if debug
    Serial1.println("使用旧键盘");
#endif

    if (iic_report[6] == 1 & iic_report[9] == 8) {
      unlock = false;
      return 0;
    }
    int mouse_dx = iic_report[0];
    int mouse_dy = iic_report[1];
    int command_key = iic_report[2];
    bool keyboard_matrix[7][11] = {
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    for (int i = 0; i < 11; i++) {
      keyboard_matrix[0][i] = iic_report[i + 3] & 0b01000000;
      keyboard_matrix[1][i] = iic_report[i + 3] & 0b00100000;
      keyboard_matrix[2][i] = iic_report[i + 3] & 0b00010000;
      keyboard_matrix[3][i] = iic_report[i + 3] & 0b00001000;
      keyboard_matrix[4][i] = iic_report[i + 3] & 0b00000100;
      keyboard_matrix[5][i] = iic_report[i + 3] & 0b00000010;
      keyboard_matrix[6][i] = iic_report[i + 3] & 0b00000001;
    }
    if (iic_report[2] == 66) {
      if (keyboard_matrix[6][0] != keyboard_matrix_last_time[6][0]) {
        if (keyboard_matrix[6][0] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new + 1;
          if (backlight_duty_cycle_new > backlight_duty_cycle_max) {
            backlight_duty_cycle = backlight_duty_cycle_max;
          }
          need_to_update_backlight = true;
          last_key = light_up;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[5][0] != keyboard_matrix_last_time[5][0]) {
        if (keyboard_matrix[5][0] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new - 1;
          if (backlight_duty_cycle_new < backlight_duty_cycle_min) {
            backlight_duty_cycle_new = backlight_duty_cycle_min;
          }
          need_to_update_backlight = true;
          last_key = light_down;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[4][0] != keyboard_matrix_last_time[4][0]) {
        if (keyboard_matrix[4][0] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new + fan_step;
          if (fan_duty_cycle_new > fan_duty_cycle_max) {
            fan_duty_cycle_new = fan_duty_cycle_max;
          }
          need_to_update_fan = true;
          last_key = fan_up;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[3][0] != keyboard_matrix_last_time[3][0]) {
        if (keyboard_matrix[3][0] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new - fan_step;
          if (fan_duty_cycle_new < fan_duty_cycle_min) {
            fan_duty_cycle_new = fan_duty_cycle_min;
          }
          need_to_update_fan = true;
          last_key = fan_down;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      delay(command_delay);
      return 0;
    }
    if (mouse_dx != 128 || mouse_dy != 128) {
      key_pressed = key_pressed + 10;
      mo.move(mouse_dx - 128, mouse_dy - 128, 0);
      keyboard_signal = keyboard_signal + 1;
    }
    for (int i = 0; i < 11; i++) {
      for (int j = 0; j < 7; j++) {
        if (keyboard_matrix[j][i] != keyboard_matrix_last_time[j][i]) {
          bool press_or_release = false;
          if (keyboard_matrix[j][i]) {
            press_or_release = true;
            key_pressed = key_pressed + 10;
          } else {
            press_or_release = false;
          }
          if (mouse_or_keyboard[j][i]) {
            if (press_or_release == true) {
              if (key_map[j][i] == MOUSE_LEFT) {
                mo.press(MOUSE_LEFT);
                last_key = MOUSE_LEFT;
              } else if (key_map[j][i] == MOUSE_RIGHT) {
                mo.press(MOUSE_RIGHT);
                last_key = MOUSE_RIGHT;
              } else if (key_map[j][i] == WHEEL_UP) {
                mo.move(0, 0, 50);
                last_key = WHEEL_UP;
              } else if (key_map[j][i] == WHEEL_DOWN) {
                mo.move(0, 0, -50);
                last_key = WHEEL_DOWN;
              }
            } else {
              if (key_map[j][i] == MOUSE_LEFT) {
                mo.release(MOUSE_LEFT);
              } else if (key_map[j][i] == MOUSE_RIGHT) {
                mo.release(MOUSE_RIGHT);
              } else if (key_map[j][i] == WHEEL_UP) {
               
              } else if (key_map[j][i] == WHEEL_DOWN) {
                
              }
            }
          } else {
            if (key_map[j][i] == KEY_LEFT_SHIFT || key_map[j][i] == KEY_RIGHT_SHIFT) {
              if (press_or_release) {
                shift = true;
              } else {
                shift = false;
              }
            }
            if (key_map[j][i] == KEY_CAPS_LOCK) {
              if (!press_or_release) {
                capslock = !capslock;
              }
            }
            if (capslock) {
              if (press_or_release) {
                kb.press(key_map_with_capslock[j][i]);
                last_key = key_map_with_capslock[j][i];
                keyboard_signal = keyboard_signal + 100;
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map_with_capslock[j][i]);
                last_key = key_map_with_capslock[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else if (shift) {
              if (press_or_release) {
                kb.press(key_map_with_shift[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map_with_shift[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else {
              if (press_or_release) {
                kb.press(key_map[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(key_map[j][i]);
                last_key = key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            }
          }
          keyboard_matrix_last_time[j][i] = keyboard_matrix[j][i];
        }
      }
    }

#if debug
    Serial1.print("iic_report-d-: ");
    for (int l = 0; l < 14; l++) {
      Serial1.print(iic_report[l]);
      Serial1.print("/");
    }
    Serial1.println("");
    Serial1.print("iic_report-b-: ");
    for (int l = 0; l < 14; l++) {
      Serial1.print(iic_report[l], BIN);
      Serial1.print("/");
    }
    Serial1.println("");
    Serial1.println("keyboard_matrix: ");
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 11; j++) {
        Serial1.print(keyboard_matrix[i][j]);
        Serial1.print("/");
      }
      Serial1.println("");
    }
    Serial1.println("");
#endif


  } else if (iic_report[keyboard_iic_length - 1] == keyboard_iic_crc_new) {
#if debug
    Serial1.println("使用新键盘");
#endif
    bool keyboard_matrix[7][12] = {
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    for (int i = 0; i < 12; i++) {
      keyboard_matrix[0][i] = iic_report[i + 2] & 0b01000000;
      keyboard_matrix[1][i] = iic_report[i + 2] & 0b00100000;
      keyboard_matrix[2][i] = iic_report[i + 2] & 0b00010000;
      keyboard_matrix[3][i] = iic_report[i + 2] & 0b00001000;
      keyboard_matrix[4][i] = iic_report[i + 2] & 0b00000100;
      keyboard_matrix[5][i] = iic_report[i + 2] & 0b00000010;
      keyboard_matrix[6][i] = iic_report[i + 2] & 0b00000001;
    }


#if debug
    Serial1.print("iic_report : ");
    for (int l = 0; l < keyboard_iic_length; l++) {
      Serial1.print(iic_report[l]);
      Serial1.print("/");
    }
    Serial1.println("");

    for (int a = 0; a < 7; a++) {
      for (int b = 0; b < (keyboard_iic_length - 2); b++) {
        if (keyboard_matrix[a][b]) {
          Serial1.print("shift : ");
          Serial1.println(shift);
          Serial1.print("capslock : ");
          Serial1.println(capslock);
          Serial1.print("char(shift) : \" ");
          Serial1.print(new_key_map_with_shift[a][b]);
          uint8_t c = new_key_map_with_shift[a][b];  //- '0';
          Serial1.println(" \"");
          Serial1.print("int : \" 0x");
          Serial1.print(c, HEX);
          Serial1.println(" \"");
          Serial1.print("翻译 : ");
          Serial1.println(key_massage_to_show[c]);
          Serial1.print("lastkey : ");
          Serial1.println(key_massage_to_show[last_key]);
          for (int d = 0; d < 9; d++) {
            // Serial1.print(key_massage_to_show[c][d]);
          }
          Serial1.print(" \"");
        } else {
        }
      }
    }
    Serial1.println();
#endif


    if (iic_report[1] == 66) {
#if debug
      Serial1.println("有特殊指令");
#endif
      if (keyboard_matrix[5][11] != new_keyboard_matrix_last_time[5][11]) {
        if (keyboard_matrix[5][11]) {
          backlight_duty_cycle_new = backlight_duty_cycle_new + 1;
          if (backlight_duty_cycle_new > backlight_duty_cycle_max) {
            backlight_duty_cycle = backlight_duty_cycle_max;
          }
          need_to_update_backlight = true;
          last_key = light_up;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[4][11] != keyboard_matrix_last_time[4][11]) {
        if (keyboard_matrix[4][11] == 1) {
          backlight_duty_cycle_new = backlight_duty_cycle_new - 1;
          if (backlight_duty_cycle_new < backlight_duty_cycle_min) {
            backlight_duty_cycle_new = backlight_duty_cycle_min;
          }
          need_to_update_backlight = true;
          last_key = light_down;
#if debug
          Serial1.print("set backlight:");
          Serial1.println(backlight_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[3][11] != keyboard_matrix_last_time[3][11]) {
        if (keyboard_matrix[3][11] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new + fan_step;
          if (fan_duty_cycle_new > fan_duty_cycle_max) {
            fan_duty_cycle_new = fan_duty_cycle_max;
          }
          need_to_update_fan = true;
          last_key = fan_up;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      if (keyboard_matrix[2][11] != keyboard_matrix_last_time[2][11]) {
        if (keyboard_matrix[2][11] == 1) {
          fan_duty_cycle_new = fan_duty_cycle_new - fan_step;
          if (fan_duty_cycle_new < fan_duty_cycle_min) {
            fan_duty_cycle_new = fan_duty_cycle_min;
          }
          need_to_update_fan = true;
          last_key = fan_down;
#if debug
          Serial1.print("set fan:");
          Serial1.println(fan_duty_cycle_new);
#endif
        }
      }
      delay(command_delay);
      return 0;
    } else if (iic_report[1] == 33) {
#if debug
      Serial1.println("没有特殊指令");
#endif
    }

    bool direction[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int direction_count = 0;
    int dx = 0;
    int dy = 0;
    if (iic_report != 0) {
      direction[0] = iic_report[0] & 0b10000000;  //上
      direction[1] = iic_report[0] & 0b01000000;  //右上
      direction[2] = iic_report[0] & 0b00100000;  //右
      direction[3] = iic_report[0] & 0b00010000;  //右下
      direction[4] = iic_report[0] & 0b00001000;  //下
      direction[5] = iic_report[0] & 0b00000100;  //左下
      direction[6] = iic_report[0] & 0b00000010;  //左
      direction[7] = iic_report[0] & 0b00000001;  //左上
      direction_count = direction[0] + direction[1] + direction[2] + direction[3] + direction[4] + direction[5] + direction[6] + direction[7];
#if debug
      Serial1.print("方向 ： ");
      Serial1.print(direction[0]);
      Serial1.print("/");
      Serial1.print(direction[1]);
      Serial1.print("/");
      Serial1.print(direction[2]);
      Serial1.print("/");
      Serial1.print(direction[3]);
      Serial1.print("/");
      Serial1.print(direction[4]);
      Serial1.print("/");
      Serial1.print(direction[5]);
      Serial1.print("/");
      Serial1.print(direction[6]);
      Serial1.print("/");
      Serial1.print(direction[7]);
      Serial1.print("/");

      Serial1.println();
      Serial1.print("方向计数 ： ");
      Serial1.println(direction_count);
#endif

      if (direction[0]) {
        dy = dy - 64;
#if debug
        Serial1.println("上");
#endif
      }
      if (direction[1]) {
        dx = dx + 45;
        dy = dy - 45;
#if debug
        Serial1.println(" 右上");
#endif
      }
      if (direction[2]) {
        dx = dx + 64;
#if debug
        Serial1.println("右");
#endif
      }
      if (direction[3]) {
        dx = dx + 45;
        dy = dy + 45;
#if debug
        Serial1.println("右下");
#endif
      }
      if (direction[4]) {
        dy = dy + 64;
#if debug
        Serial1.println("下");
#endif
      }
      if (direction[5]) {
        dx = dx - 45;
        dy = dy + 45;
#if debug
        Serial1.println("左下");
#endif
      }
      if (direction[6]) {
        dx = dx - 64;
#if debug
        Serial1.println("左");
#endif
      }
      if (direction[7]) {
        dx = dx - 45;
        dy = dy - 45;
#if debug
        Serial1.println("左上");
#endif
      }
      if (direction_count > 1) {
        dx = dx * 2 / 3;
        dy = dy * 2 / 3;
      }

#if debug
      Serial1.print("dx: ");
      Serial1.println(dx);
      Serial1.print("dy: ");
      Serial1.println(dy);
#endif
    }


    int mouse_dx = 128 + dx;
    int mouse_dy = 128 + dy;
#if debug
    Serial1.print("mouse_dx : ");
    Serial1.println(mouse_dx);
    Serial1.print("mouse_dy: ");
    Serial1.println(mouse_dy);
#endif
    if (mouse_dx != 128 || mouse_dy != 128) {

      mo.move(mouse_dx, mouse_dy, 0);
      keyboard_signal = keyboard_signal + 1;
    }


    for (int i = 0; i < 12; i++) {
      for (int j = 0; j < 7; j++) {
        if (keyboard_matrix[j][i] != new_keyboard_matrix_last_time[j][i]) {
          bool press_or_release = false;
          if (keyboard_matrix[j][i]) {
            press_or_release = true;
            key_pressed = key_pressed + 10;
          } else {
            press_or_release = false;
          }
          if (new_mouse_or_keyboard[j][i]) {
            if (press_or_release == true) {
              if (new_key_map[j][i] == MOUSE_LEFT) {
                mo.press(MOUSE_LEFT);
                last_key = MOUSE_LEFT;
              } else if (new_key_map[j][i] == MOUSE_RIGHT) {
                mo.press(MOUSE_RIGHT);
                last_key = MOUSE_RIGHT;
              } else if (new_key_map[j][i] == WHEEL_UP) {
                mo.move(0, 0, 50);
                last_key = WHEEL_UP;
              } else if (new_key_map[j][i] == WHEEL_DOWN) {
                mo.move(0, 0, -50);
                last_key = WHEEL_DOWN;
              }
            } else {
              if (new_key_map[j][i] == MOUSE_LEFT) {
                mo.release(MOUSE_LEFT);
              } else if (new_key_map[j][i] == MOUSE_RIGHT) {
                mo.release(MOUSE_RIGHT);
              } else if (new_key_map[j][i] == WHEEL_UP) {
               
              } else if (new_key_map[j][i] == WHEEL_DOWN) {
               
              }
            }

          } else {
            if (new_key_map[j][i] == KEY_LEFT_SHIFT || new_key_map[j][i] == KEY_RIGHT_SHIFT) {
              if (press_or_release) {
                shift = true;
              } else {
                shift = false;
              }
            }
            if (new_key_map[j][i] == KEY_CAPS_LOCK) {
              if (!press_or_release) {
                capslock = !capslock;
              }
            }
            if (capslock) {
              if (press_or_release) {
                kb.press(new_key_map_with_capslock[j][i]);
                last_key = new_key_map_with_capslock[j][i];
                keyboard_signal = keyboard_signal + 100;
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map_with_capslock[j][i]);
                last_key = new_key_map_with_capslock[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map_with_capslock[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else if (shift) {
              if (press_or_release) {
                kb.press(new_key_map_with_shift[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map_with_shift[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map_with_shift[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            } else {
              if (press_or_release) {
                kb.press(new_key_map[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("press : ");
                Serial1.print(new_key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              } else {
                kb.release(new_key_map[j][i]);
                last_key = new_key_map_with_shift[j][i];
#if debug
                Serial1.print("release : ");
                Serial1.print(new_key_map[j][i]);
                Serial1.print("/");
                Serial1.print(j);
                Serial1.print("/");
                Serial1.println(i);
#endif
              }
            }
          }
          new_keyboard_matrix_last_time[j][i] = keyboard_matrix[j][i];
        }
      }
    }
  }
  return 0;
}
#endif

void hdmi_update() {
  screen_enabled = digitalread(hdmi_active);
  digitalwrite(screen_enable, screen_enabled);
}

void adc() {
  backlight_voltage = analogread(backlight) * backlight_adc_times * 33000 / 4095;  //单位是0.1mv
  unsigned long long a = 0;
  unsigned long long j = 0;
  for (int i = 0; i < battery_adc_count; i++) {
    j = analogread(battery);
    a = a + j;
  }
  battery_voltage = a / battery_adc_count * battery_adc_times * 33000 / 4095;

  battery_voltage_[battery_count] = battery_voltage;
  battery_count++;
  if (battery_count >= battery_count_max) {
    battery_count = 0;
  }
  unsigned long long int battery_avr = 0;
  for (int i = 0; i < battery_count_max; i++) {
    battery_avr = battery_avr + battery_voltage_[i];
  }
  battery_avr = battery_avr / battery_count_max;
  battery_percent = map(battery_avr, battery_voltage_min, battery_voltage_max, 0, 100);
  if (battery >= 100) {
    battery_percent = 100;
  } else if (battery_percent <= 0) {
    battery_percent = 0;
  }
  main_voltage = analogread(main_power) * main_power_adc_times * 33000 / 4095;
  pi_3v3_voltage = analogread(pi_3v3) * main_power_adc_times * 33000 / 4095;
  charge_or_not = !digitalRead(charge_status);
  charge_done_or_not = !digitalRead(charge_done_status);
}
void set_ws2812() {
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
    11 随机数
    */

  led_count++;
  if (led_count >= 200) {
    led_count = 0;
  }
  if (charge_or_not == true) {
    if (charge_done_or_not == true) {
      rgb[0][0] = 0;
      rgb[1][0] = 8;
      rgb[2][0] = 0;
    } else {
      rgb[0][0] = 0;
      rgb[1][0] = 0;
      rgb[2][0] = 6;
    }
  } else if (battery_percent >= 90) {
    rgb[0][0] = 0;
    rgb[1][0] = 5;
    rgb[2][0] = 0;
  } else if (battery_percent >= 70) {
    rgb[0][0] = 1;
    rgb[1][0] = 4;
    rgb[2][0] = 0;
  } else if (battery_percent >= 50) {
    rgb[0][0] = 2;
    rgb[1][0] = 3;
    rgb[2][0] = 0;
  } else if (battery_percent >= 30) {
    rgb[0][0] = 3;
    rgb[1][0] = 2;
    rgb[2][0] = 0;
  } else if (battery_percent >= 20) {
    rgb[0][0] = 4;
    rgb[1][0] = 1;
    rgb[2][0] = 0;
  } else if (battery_percent >= 10) {
    rgb[0][0] = 5;
    rgb[1][0] = 0;
    rgb[2][0] = 0;
  } else {
    rgb[0][0] = 7;
    rgb[1][0] = 0;
    rgb[2][0] = 0;
  }
  int a = 0;
  if (any_error) {
    if (any_error >= 5) {
      rgb[0][1] = 5;
      rgb[1][1] = 0;
      rgb[2][1] = 0;
    } else if (any_error >= 3) {
      rgb[0][1] = 3;
      rgb[1][1] = 0;
      rgb[2][1] = 0;
    } else {
      rgb[0][1] = 1;
      rgb[1][1] = 0;
      rgb[2][1] = 0;
    }
  } else {
    rgb[0][1] = 0;
    rgb[1][1] = 0;
    rgb[2][1] = 2;
  }
  if (key_pressed > 0) {
    rgb[0][2] = 0;
    rgb[1][2] = 0;
    rgb[2][2] = 3;
    key_pressed--;
  } else {
    rgb[0][2] = 0;
    rgb[1][2] = 3;
    rgb[2][2] = 0;
  }
  if (touched > 0) {
    rgb[0][3] = 0;
    rgb[1][3] = 0;
    rgb[2][3] = 3;
    touched--;
  } else {
    rgb[0][3] = 0;
    rgb[1][3] = 3;
    rgb[2][3] = 0;
  }
  if (capslock) {
    rgb[0][4] = 3;
    rgb[1][4] = 0;
    rgb[2][4] = 3;
  } else {
    rgb[0][4] = 0;
    rgb[1][4] = 3;
    rgb[2][4] = 0;
  }
  a = led_count / breath_speed;
  a = a % 10 + 1;
  if (a > 5) {
    a = 11 - a;
  }
  rgb[0][5] = a;
  rgb[1][5] = 0;
  rgb[2][5] = 0;
  rgb[0][6] = 0;
  rgb[1][6] = a;
  rgb[2][6] = 0;
  rgb[0][7] = 0;
  rgb[1][7] = 0;
  rgb[2][7] = a;
  a = fan_duty_cycle / 20;
  if (a >= 4) {
    rgb[0][8] = 0;
    rgb[1][8] = 0;
    rgb[2][8] = 3;
  } else if (a >= 2) {
    rgb[0][8] = 0;
    rgb[1][8] = 2;
    rgb[2][8] = 0;
  } else if (a > 0) {
    rgb[0][8] = 0;
    rgb[1][8] = 1;
    rgb[2][8] = 0;
  } else {
    rgb[0][8] = 3;
    rgb[1][8] = 0;
    rgb[2][8] = 0;
  }
  a = backlight_duty_cycle / 20;
  if (a >= 4) {
    rgb[0][9] = 0;
    rgb[1][9] = 0;
    rgb[2][9] = 3;
  } else if (a >= 2) {
    rgb[0][9] = 0;
    rgb[1][9] = 2;
    rgb[2][9] = 0;
  } else if (a > 0) {
    rgb[0][9] = 0;
    rgb[1][9] = 1;
    rgb[2][9] = 0;
  } else {
    rgb[0][9] = 3;
    rgb[1][9] = 0;
    rgb[2][9] = 0;
  }
  a = analogread(backlight) % 5;
  rgb[0][10] = a;
  a = analogread(battery) % 5;
  rgb[1][10] = a;
  a = analogread(main_power) % 5;
  rgb[2][10] = a;

  pixels.clear();
  pixels.spc(0, rgb[0][ws2812_0], rgb[1][ws2812_0], rgb[2][ws2812_0]);
  pixels.spc(1, rgb[0][ws2812_1], rgb[1][ws2812_1], rgb[2][ws2812_1]);
  pixels.spc(2, rgb[0][ws2812_2], rgb[1][ws2812_2], rgb[2][ws2812_2]);
  pixels.spc(3, rgb[0][ws2812_3], rgb[1][ws2812_3], rgb[2][ws2812_3]);
  pixels.show();
}

void set_iic_screen() {
  display.cd();
  if (touch_x | touch_y) {
    display.drawCircle(touch_x / 25, touch_y / 20, 1, SSD1306_WHITE);
    touch_x = 0;
    touch_y = 0;
  } else {
    int logo_section = tip_massage / 6;
    logo_section = logo_section % 6;
    if (!logo_section) {
      display.drawBitmap(0, 0, device_logo0, logo_width, logo_hight, 1);
    } else if (logo_section == 1) {
      display.drawBitmap(0, 0, device_logo1, logo_width, logo_hight, 1);
    } else if (logo_section == 2) {
      display.drawBitmap(0, 0, device_logo2, logo_width, logo_hight, 1);
    } else if (logo_section == 3) {
      display.drawBitmap(0, 0, device_logo3, logo_width, logo_hight, 1);
    } else if (logo_section == 4) {
      display.drawBitmap(0, 0, device_logo4, logo_width, logo_hight, 1);
    } else if (logo_section == 5) {
      display.drawBitmap(0, 0, device_logo5, logo_width, logo_hight, 1);
    } else {
      display.drawBitmap(0, 0, my_logo, logo_width, logo_hight, 1);
    }
  }
  if (charge_or_not) {
    display.drawBitmap(0, 24, battery_logo_charge, battery_logo_width, battery_logo_hight, 1);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 37);
    display.cp437(true);
    if (charge_done_or_not) {
      display.print("Charge OK");
    } else {
      display.print("Charging");
    }
  } else {
    display.drawBitmap(0, 24, battery_logo_empty, battery_logo_width, battery_logo_hight, 1);
    int battery_pixel = map(battery_percent, 0, 100, 2, 28);
    for (int i = 2; i < battery_pixel; i++) {
      display.drawPixel(i, 26, SSD1306_WHITE);
      display.drawPixel(i, 27, SSD1306_WHITE);
      display.drawPixel(i, 28, SSD1306_WHITE);
      display.drawPixel(i, 29, SSD1306_WHITE);
    }
    if (battery_percent >= 100) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      display.print("Full");
    } else if (battery_percent >= 75) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      display.print(battery_percent);
      display.print("%");
    } else if (battery_percent >= 50) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      display.print(battery_percent);
      display.print("%");
    } else if (battery_percent >= 25) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      display.print(battery_percent);
      display.print("%");
    } else if (battery_percent >= 10) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 37);
      display.cp437(true);
      display.print(battery_percent);
      display.print("%");
    } else {
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
  if (screen_enabled) {
    display.print("YES");
  } else {
    display.print("NO");
  }
  display.setCursor(0, 54);
  display.setTextSize(1);
  display.print("Light:");
  //  int i = backlight_duty_cycle;
  int i = map(backlight_duty_cycle, 80, 100, 0, 100);
  display.print(i);
  display.setCursor(0, 62);
  display.setTextSize(1);
  display.print("Fan:");
  i = fan_duty_cycle;
  display.print(i);
  display.drawLine(0, 65, 31, 65, SSD1306_WHITE);
  display.setCursor(0, 71);
  display.setTextSize(1);
  int if_touched = touch_x + touch_y;
  display.print("X:");
  display.println(mouse_x);
  display.print("Y:");
  display.println(mouse_y);

  if (keyboard_status) {
    if (key_pressed) {
      display.print("K:");
      key_pressed = 0;
    } else {
      display.print("K:");
    }
    display.println(key_massage_to_show[last_key]);
  }
  int y = 86;
  display.drawLine(0, y, 31, y, SSD1306_WHITE);
  display.setCursor(0, y + 6);
  display.setTextSize(1);
  display.println("TIPS:");
  if (keyboard_status == 1) {
    display.println("STICK");
    display.print("+ ");
  } else if (keyboard_status == 2) {
    display.println("MOU L+R");
    display.print("+ ");
  }

  if (tip_massage < tip_massage_max / 4) {
    display.println("F1");
    display.println("=");
    display.println("lig up");
  } else if (tip_massage < tip_massage_max / 2) {
    display.println("F2");
    display.println("=");
    display.println("lig down");
  } else if (tip_massage < tip_massage_max / 4 * 3) {
    display.println("F3");
    display.println("=");
    display.println("fan up");
  } else if (tip_massage < tip_massage_max) {
    display.println("F4");
    display.println("=");
    display.println("fan down");
  }
  tip_massage++;

  if (tip_massage >= tip_massage_max) {
    tip_massage = 0;
  }
#if debug
  if (any_error) {
    display.print("ERR:");
    for (int i = 0; i < 100; i++) {
      if (error_code[i] > 0) {
        display.print(i);
        display.print("/");
      }
    }
    display.println();
  }
#endif
  if (parameter1_i) {
    display.print(parameter1_s);
    display.print(":");
    display.println(parameter1_i);
  }
  if (parameter2_i) {
    display.print(parameter2_s);
    display.print(":");
    display.println(parameter2_i);
  }
  //下面8行应该被注释掉
  //display.print(" ");
  //display.println(backlight_voltage);
  //display.print(" ");
  //display.println(battery_voltage);
  //display.print(" ");
  //display.println(main_voltage);
  //display.print(" ");
  //display.println(pi_3v3_voltage);
  display.display();
}

void find_any_error() {
#if debug
  Serial1.println("finding error");
#endif
  if (digitalRead(hdmi_active)) {
    if (backlight_voltage > screen_enable_voltage_high) {
      error_code[screen_voltage_high] = screen_voltage_high;
    } else if (backlight_voltage < screen_enable_voltage_low) {
      error_code[screen_voltage_low] = screen_voltage_low;
    } else {
      error_code[screen_voltage_high] = 0;
      error_code[screen_voltage_low] = 0;
    }
#if debug
    if (error_code[screen_voltage_high]) {
      Serial1.println("error find:");
      Serial1.print("error code is :");
      Serial1.println(error_code[screen_voltage_high]);
      Serial1.println("screen backlight voltage too high");
    } else if (error_code[screen_voltage_low]) {
      Serial1.println("error find:");
      Serial1.print("error code is :");
      Serial1.println(error_code[screen_voltage_low]);
      Serial1.println("screen backlight voltage too low");
    } else {
    }
#endif
  } else {
    if (backlight_voltage > screen_not_enable_voltage_high) {
      error_code[screen_voltage_high] = screen_voltage_high;
    } else if (backlight_voltage < screen_not_enable_voltage_low) {
      error_code[screen_voltage_low] = screen_voltage_low;
    } else {
      error_code[screen_voltage_high] = 0;
      error_code[screen_voltage_low] = 0;
    }
#if debug
    if (error_code[screen_voltage_high]) {
      Serial1.println("error find:");
      Serial1.print("error code is :");
      Serial1.println(error_code[screen_voltage_high]);
      Serial1.println("screen backlight voltage too high");
    } else if (error_code[screen_voltage_low]) {
      Serial1.println("error find:");
      Serial1.print("error code is :");
      Serial1.println(error_code[screen_voltage_low]);
      Serial1.println("screen backlight voltage too low");
    } else {
    }
#endif
  }
  if (battery_voltage > battery_voltage_max) {
    error_code[battery_volatge_high] = battery_volatge_high;
  } else if (battery_voltage < battery_voltage_min) {
    error_code[battery_volatge_low] = battery_volatge_low;
  } else {
    error_code[battery_volatge_high] = 0;
    error_code[battery_volatge_low] = 0;
  }
#if debug
  if (error_code[battery_volatge_high]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[battery_volatge_high]);
    Serial1.println("battery volatge too high");
  } else if (error_code[battery_volatge_low]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[battery_volatge_low]);
    Serial1.println("battery volatge too low");
  } else {
  }
#endif
  if (main_voltage > main_power_voltage_max) {
    error_code[main_power_voltage_high] = main_power_voltage_high;
  } else if (main_voltage < main_power_voltage_min) {
    error_code[main_power_voltage_low] = main_power_voltage_low;
  } else {
    error_code[main_power_voltage_high] = 0;
    error_code[main_power_voltage_low] = 0;
  }
#if debug
  if (error_code[main_power_voltage_high]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[main_power_voltage_high]);
    Serial1.println("main power voltage too high");
  } else if (error_code[main_power_voltage_low]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[main_power_voltage_low]);
    Serial1.println("main power voltage too low");
  } else {
  }
#endif
  if (pi_3v3_voltage > pi_3v3_voltage_max) {
    error_code[pi_3v3_voltage_high] = pi_3v3_voltage_high;
  } else if (pi_3v3_voltage < pi_3v3_voltage_min) {
    error_code[pi_3v3_voltage_low] = pi_3v3_voltage_low;
  } else {
    error_code[pi_3v3_voltage_high] = 0;
    error_code[pi_3v3_voltage_low] = 0;
  }
#if debug
  if (error_code[pi_3v3_voltage_high]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[pi_3v3_voltage_high]);
    Serial1.println("pi 3v3 power voltage too high");
  } else if (error_code[pi_3v3_voltage_low]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[pi_3v3_voltage_low]);
    Serial1.println("pi 3v3 power voltage too low");
  } else {
  }
#endif
  if (keyboard_status) {
    error_code[no_keyboard] = 0;
  } else {
    error_code[no_keyboard] = no_keyboard;
  }
#if debug
  if (error_code[no_keyboard]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[no_keyboard]);
    Serial1.println("no keyboard or game pad found");
  }
#endif
  if (!touch_panel_status) {
    error_code[no_touch_panel] = 0;
  } else {
    error_code[no_touch_panel] = no_touch_panel;
  }
#if debug
  if (error_code[no_touch_panel]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[no_touch_panel]);
    Serial1.println("no touch panel found");
  }
#endif
  wire1.beginTransmission(iic_display_address);
  iic_screen_status = !wire1.endTransmission();
  if (iic_screen_status) {
    error_code[no_iic_screen] = 0;
  } else {
    error_code[no_iic_screen] = no_iic_screen;
  }
#if debug
  if (error_code[no_iic_screen]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[no_iic_screen]);
    Serial1.println("no iic screen panel found");
  }
#endif
  if (charge_or_not == false) {
    if (charge_done_or_not == true) {
      error_code[not_chargeing_but_charge_done] = not_chargeing_but_charge_done;
    } else {
      error_code[not_chargeing_but_charge_done] = 0;
    }
  } else {
    error_code[not_chargeing_but_charge_done] = 0;
  }
#if debug
  if (error_code[not_chargeing_but_charge_done]) {
    Serial1.println("error find:");
    Serial1.print("error code is :");
    Serial1.println(error_code[not_chargeing_but_charge_done]);
    Serial1.println("not chargeing but charge done");
  }
#endif
#if debug
  Serial1.println("error code : ");
  for (int i = 1; i < 101; i++) {
    Serial1.print(error_code[i - 1]);
    Serial1.print("/");
    if (i % 10) {
    } else {
      Serial1.println();
    }
  }
  Serial1.println();
#endif

  for (int i = 0; i < 100; i++) {
    any_error = any_error + error_code[i];
  }
#if debug
  Serial1.println("find error finish");
#endif
}

bool lock_up() {
  if (!unlock) {
#if debug
    Serial1.println("机器上锁");
#endif
    pixels.clear();
    pixels.show();
    pwm_instance[0]->setPWM(fan_pwm, fan_pwm_freq, 0);
    need_to_update_fan = true;
    pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 0);
    need_to_update_backlight = true;
    digitalwrite(screen_enable, LOW);
    display.clearDisplay();
    display.drawBitmap(0, 0, lock_logo, sleep_logo_width, sleep_logo_hight, 1);
    display.setCursor(0, 37);
    display.setTextSize(1);
    display.println("Device");
    display.println("Sleeping");
    display.println("");
    display.println("long press");
    display.println("tab+8");
    display.println("to unlock");
    display.display();
    delay(10000);
    while (!unlock) {
      wire1.beginTransmission(keyboard_iic_address);
      wire1.endTransmission(false);
      wire1.requestFrom(keyboard_iic_address, keyboard_iic_length);
      uint8_t iic_keyboard_report[keyboard_iic_length];
      for (int i = 0; i < keyboard_iic_length; i++) {
        iic_keyboard_report[i] = wire1.read();
      }
      if (keyboard_status == 2) {
        if (iic_keyboard_report[5] == 1 & iic_keyboard_report[8] == 32) {
          unlock = true;
        } else {
          unlock = false;
        }
      } else if (keyboard_status == 1) {
        if (iic_keyboard_report[6] == 1 & iic_keyboard_report[9] == 8) {
          unlock = true;
        } else {
          unlock = false;
        }
      }
      delay(1000);
    }
  }
  digitalwrite(screen_enable, HIGH);
  return 1;
}



void update_fan() {
#if debug
  Serial1.println("update fan begin");
#endif
  if (need_to_update_fan) {
    if (fan_duty_cycle_new >= fan_duty_cycle_max) {
      fan_duty_cycle_new = fan_duty_cycle_max;
    }
    if (fan_duty_cycle_new <= fan_duty_cycle_min) {
      fan_duty_cycle_new = fan_duty_cycle_min;
    }
    eeprom.begin(eeprom_space);
    int fan_duty_cycle_i = fan_duty_cycle_new * 10;
    eeprom.write(fan_duty_cycle_address, fan_duty_cycle_i / 100);
#if debug
    Serial1.print("fan_duty_cycle_i = ");
    Serial1.print(fan_duty_cycle_i / 100);
#endif
    fan_duty_cycle_i = fan_duty_cycle_i % 100;
    eeprom.write(fan_duty_cycle_address + 1, fan_duty_cycle_i / 10);
#if debug
    Serial1.print(fan_duty_cycle_i / 10);
#endif
    eeprom.write(fan_duty_cycle_address + 2, fan_duty_cycle_i % 10);
#if debug
    Serial1.print(fan_duty_cycle_i % 10);
    delay(1000);
#endif
    eeprom.end();
    fan_duty_cycle = fan_duty_cycle_new;
    pwm_instance[0]->setPWM(fan_pwm, fan_pwm_freq, fan_duty_cycle);
    delay(50);
#if debug
    Serial1.println("fan_duty_cycle update");
#endif

    need_to_update_fan = false;
    set_iic_screen();
#if debug
    Serial1.print("update fan speed to ");
    Serial1.print(fan_duty_cycle);
    Serial1.println("%");
#endif
  }
#if debug
  Serial1.println("update fan finish");
#endif
}

void update_backlight() {
#if debug
  Serial1.println("update backlight begin");
#endif
  if (need_to_update_backlight) {
    if (backlight_duty_cycle_new >= backlight_duty_cycle_max) {
      backlight_duty_cycle_new = backlight_duty_cycle_max;
    }
    if (backlight_duty_cycle_new <= backlight_duty_cycle_min) {
      backlight_duty_cycle_new = backlight_duty_cycle_min;
    }
    eeprom.begin(eeprom_space);
    int backlight_duty_cycle_i = backlight_duty_cycle_new * 10;
    eeprom.write(backlight_duty_cycle_address, backlight_duty_cycle_i / 100);
    backlight_duty_cycle_i = backlight_duty_cycle_i % 100;
    eeprom.write(backlight_duty_cycle_address + 1, backlight_duty_cycle_i / 10);
    eeprom.write(backlight_duty_cycle_address + 1, backlight_duty_cycle_i % 10);
    eeprom.end();
    backlight_duty_cycle = backlight_duty_cycle_new;
    pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_duty_cycle);
#if debug
    Serial1.println("backlight update");
#endif

    need_to_update_backlight = false;
    set_iic_screen();
#if debug
    Serial1.print("update backlight to ");
    Serial1.print(backlight_duty_cycle);
    Serial1.println("%");
#endif
  }
#if debug
  Serial1.println("update backlight finish");
#endif
}