/*
   @Author: 无序熵增
   @Date: 2025-07-25 11:54:52
   @LastEditors: 无序熵增
   @LastEditTime: 2025-12-09 14:09:44
   @Description:

   Copyright (c) 2025 by 无序熵增,
*/
#include "iic_keyboard.h"
#include <Keyboard.h>
#include "MyMouseAbsolute.h"
#include <Wire.h>
#include "config.h"


bool IIC_KEYBOARD::begin()
{
  Keyboard.begin();
  MyMouseAbsolute.begin();

#if debug
  Serial1.println("keyboard enable");
#endif

  // 第一次读取（默认地址）
  read_keyboard_iic();
  read_new_keyboard_iic();


  // 检查是否有键盘被检测到
  bool keyboard_detected = old_keyboard_flag || new_keyboard_flag || new_address_keyboard_flag;

#if debug
  if (!keyboard_detected)
  {
    Serial1.println("no keyboard detect");
  }
#endif

  return keyboard_detected;
}

bool IIC_KEYBOARD::read_keyboard_iic()
{
  bool result = read_iic_data(keyboard_iic_address);
  if (result)
  {
    if (iic_buffer[keyboard_iic_length - 1] == new_keyboard_crc)
    {
      new_keyboard_flag = true;
#if debug
      Serial1.println("new keyboard detect at new address 0x30");
#endif
    }
    if (iic_buffer[keyboard_iic_length - 1] == old_keyboard_crc)
    {
      old_keyboard_flag = true;
#if debug
      Serial1.println("old keyboard detect at new address 0x30");
#endif
    }
  }
  else
  {
    new_keyboard_flag = false;
    old_keyboard_flag = false;
#if debug
    Serial1.println("no keyboard detect at new address 0x30");
#endif
  }
  keyboard_connect = new_address_keyboard_flag * 4 + new_keyboard_flag * 2 + old_keyboard_flag;
  return result;
}

bool IIC_KEYBOARD::read_new_keyboard_iic()
{
  bool result = read_iic_data(new_keyboard_iic_address);

  if (result && iic_buffer[keyboard_iic_length - 1] == new_keyboard_crc)
  {
    new_address_keyboard_flag = true;
#if debug
    Serial1.println("new keyboard detect at new address 0x31");
#endif
  }
  else
  {
    new_address_keyboard_flag = false;
#if debug
    Serial1.println("no new keyboard detect at new address 0x31");
#endif
  }
  keyboard_connect = new_address_keyboard_flag * 4 + new_keyboard_flag * 2 + old_keyboard_flag;
  return result;
}

bool IIC_KEYBOARD::read_iic_data(uint8_t address)
{
  Wire1.beginTransmission(address);
  Wire1.endTransmission(false);
  Wire1.requestFrom(address, keyboard_iic_length);

#if debug
  Serial1.print("keyboard iic read at 0x");
  Serial1.print(address, HEX);
  Serial1.print(": ");
#endif

  // 读取数据
  for (int i = 0; i < keyboard_iic_length; i++)
  {
    iic_buffer[i] = Wire1.read();
#if debug
    Serial1.print(iic_buffer[i]);
    Serial1.print("/");
#endif
  }

  uint8_t received_crc = iic_buffer[keyboard_iic_length - 1];
  if (received_crc == old_keyboard_crc )
  {
#if debug
    Serial1.println(". crc ok! is old keyboard!");
#endif
    return true;
  }
  else if (received_crc == new_keyboard_crc)
  {
#if debug
    Serial1.println(". crc ok! is new keyboard!");
#endif
    return true;
  }
  else
  {
#if debug
    Serial1.println(". crc error!");
#endif
    return false;
  }
}
bool IIC_KEYBOARD::reconnect_keyboard()
{
  read_keyboard_iic();
  read_new_keyboard_iic();
  keyboard_connect = new_address_keyboard_flag * 4 + new_keyboard_flag * 2 + old_keyboard_flag;
  bool keyboard_detected = old_keyboard_flag || new_keyboard_flag || new_address_keyboard_flag;

#if debug
  if (!keyboard_detected)
  {
    Serial1.println("no keyboard detect");
  }
#endif

  return keyboard_detected;
}
bool IIC_KEYBOARD::routine(int mouse_x, int mouse_y)
{
  if (new_keyboard_flag || old_keyboard_flag || new_address_keyboard_flag)
  {
    if (new_keyboard_flag)
    {
      if (read_keyboard_iic())
      {
        new_keyboard_routine(mouse_x, mouse_y);
#if debug
        Serial1.println("new keyboard routine at old address running");
#endif
      }
    }
    if (old_keyboard_flag)
    {
      if (read_keyboard_iic())
      {
        old_keyboard_routine(mouse_x, mouse_y);
#if debug
        Serial1.println("old keyboard routine at old address running");
#endif
      }
    }
    if (new_address_keyboard_flag)
    {
      if (read_new_keyboard_iic())
      {
        new_keyboard_routine(mouse_x, mouse_y);
#if debug
        Serial1.println("new keyboard routine at new address running");
#endif
      }
    }
    return true;
  }
  else
  {
#if debug
    Serial1.println("reconect keyboard");
#endif
    reconnect_keyboard();
  }
  return false;
}

bool IIC_KEYBOARD::old_keyboard_routine(int mouse_x, int mouse_y)
{
#if debug
  Serial1.println("old keyboard routine running");
#endif
  // read raw stick bytes from I2C buffer
  mouse_x_ = iic_buffer[0];
  mouse_y_ = iic_buffer[1];

  // decode matrix into explicit bools
  bool keyboard_matrix[7][11] = {{0}};
  for (int col = 0; col < 11; col++)
  {
    uint8_t b = iic_buffer[col + 3];
    keyboard_matrix[0][col] = (b & 0b01000000) != 0;
    keyboard_matrix[1][col] = (b & 0b00100000) != 0;
    keyboard_matrix[2][col] = (b & 0b00010000) != 0;
    keyboard_matrix[3][col] = (b & 0b00001000) != 0;
    keyboard_matrix[4][col] = (b & 0b00000100) != 0;
    keyboard_matrix[5][col] = (b & 0b00000010) != 0;
    keyboard_matrix[6][col] = (b & 0b00000001) != 0;
  }

#if debug
  for (int r = 0; r < 7; r++)
  {
    for (int c = 0; c < 11; c++)
    {
      if (keyboard_matrix[r][c])
      {
        Serial1.print("shift : ");
        Serial1.println(shift);
        Serial1.print("capslock : ");
        Serial1.println(capslock);
        Serial1.print("char(shift) : \" ");
        Serial1.print(old_key_map_with_shift[r][c]);
        uint8_t cc = old_key_map_with_shift[r][c];
        Serial1.println(" \"");
        Serial1.print("int : \" 0x");
        Serial1.print(cc, HEX);
        Serial1.println(" \"");
        Serial1.print("trans : ");
        // safer to print the mapped description instead of raw char
        Serial1.print((const char *)key_massage_to_show[last_key]);
        Serial1.println(" \"");
      }
    }
  }
#endif

  // ----- special instruction branch (e.g. backlight / fan) -----
  if (iic_buffer[2] == 66)
  {
#if debug
    Serial1.println("special instruction");
#endif
    // if changed, set flag AND update old matrix so we don't keep retriggering
    if (keyboard_matrix[6][0] != old_keyboard_matrix_last_time[6][0])
    {
      need_increase_backlight = true;
      old_keyboard_matrix_last_time[6][0] = keyboard_matrix[6][0];
#if dbeug
      Serial1.println("need increase backlight");
#endif
    }
    if (keyboard_matrix[5][0] != old_keyboard_matrix_last_time[5][0])
    {
      need_reduce_backlight = true;
      old_keyboard_matrix_last_time[5][0] = keyboard_matrix[5][0];
#if dbeug
      Serial1.println("need reduce backlight");
#endif
    }
    if (keyboard_matrix[4][0] != old_keyboard_matrix_last_time[4][0])
    {
      need_increase_fan_speed = true;
      old_keyboard_matrix_last_time[4][0] = keyboard_matrix[4][0];
#if dbeug
      Serial1.println("need increase fan speed");
#endif
    }
    if (keyboard_matrix[3][0] != old_keyboard_matrix_last_time[3][0])
    {
      need_reduce_fan_speed = true;
      old_keyboard_matrix_last_time[3][0] = keyboard_matrix[3][0];
#if dbeug
      Serial1.println("need reduce fan speed");
#endif
    }
    return true;
  }

  // ----- analog stick deadzone normalization -----
  if (mouse_x_ != 128 || mouse_y_ != 128)
  {
    if (mouse_x_ < 128 + mouse_dead_zone && mouse_x_ > 128 - mouse_dead_zone)
      mouse_x_ = 128;
    if (mouse_y_ < 128 + mouse_dead_zone && mouse_y_ > 128 - mouse_dead_zone)
      mouse_y_ = 128;
  }

  // ----- mouse movement calculation and clamp -----
  if (mouse_x_ != 128 || mouse_y_ != 128)
  {
    // preserve original precedence explicitly
    mouse_x = mouse_x + (mouse_x_ * stick_speed) - (128 * stick_speed);
    mouse_y = mouse_y + (mouse_y_ * stick_speed) - (128 * stick_speed);

    // clamp to safe HID range (adjust if your HID expects different range)
    const long MOUSE_MIN = 0L;
    const long MOUSE_MAX = 32767L;
    if (mouse_x < MOUSE_MIN)
      mouse_x = MOUSE_MIN;
    if (mouse_x > MOUSE_MAX)
      mouse_x = MOUSE_MAX;
    if (mouse_y < MOUSE_MIN)
      mouse_y = MOUSE_MIN;
    if (mouse_y > MOUSE_MAX)
      mouse_y = MOUSE_MAX;

    if (mouse_left_flag)
      MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_LEFT);
    else
      MyMouseAbsolute.move(mouse_x, mouse_y, 0);

    mouse_moved = true;
#if debug
    Serial1.print("keyboard mouse move to x : ");
    Serial1.print(mouse_x);
    Serial1.print(" y : ");
    Serial1.print(mouse_y);
    Serial1.println();
#endif
  }

  // helper lambda: safe check for keycode indices that index key_massage_to_show etc.
  auto keycode_valid = [](int k) -> bool
  {
    return (k >= 0 && k < 256); // conservative bound; adjust if your key tables differ
  };

  // ----- matrix diff & key handling (single unified flow) -----
  for (int r = 0; r < 7; r++)
  {
    for (int c = 0; c < 11; c++)
    {
      if (keyboard_matrix[r][c] == old_keyboard_matrix_last_time[r][c])
        continue; // no change

      // update old state immediately
      old_keyboard_matrix_last_time[r][c] = keyboard_matrix[r][c];
      bool press_or_release = keyboard_matrix[r][c];

      // pressed counter (your existing "press pressure" behaviour)
      if (press_or_release)
      {
        if (pressed < pressed_max)
          pressed += 10;
        else
          pressed = pressed_max;
      }

      // mouse-mapped keys branch
      if (old_mouse_or_keyboard[r][c])
      {
        int mapped = old_key_map[r][c];

        if (mapped == MOUSE_LEFT)
        {
          if (press_or_release)
          {
            MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_LEFT);
            mouse_left_flag = true;
            last_key = MOUSE_LEFT;
#if debug
            Serial1.println("mouse left pressed");
#endif
          }
          else
          {
            MyMouseAbsolute.release(mouse_x, mouse_y, MOUSE_LEFT);
            mouse_left_flag = false;
#if debug
            Serial1.println("mouse left released");
#endif
          }
        }
        else if (mapped == MOUSE_RIGHT)
        {
          if (press_or_release)
          {
            MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_RIGHT);
            last_key = MOUSE_RIGHT;
#if debug
            Serial1.println("mouse right pressed");
#endif
          }
          else
          {
            MyMouseAbsolute.release(mouse_x, mouse_y, MOUSE_RIGHT);
#if debug
            Serial1.println("mouse right released");
#endif
          }
        }
        else if (mapped == WHEEL_UP)
        {
          if (press_or_release)
          {
            MyMouseAbsolute.move(mouse_x, mouse_y, 50);
            mouse_moved = true;
            last_key = WHEEL_UP;
#if debug
            Serial1.println("mouse wheel up");
#endif
          }
        }
        else if (mapped == WHEEL_DOWN)
        {
          if (press_or_release)
          {
            MyMouseAbsolute.move(mouse_x, mouse_y, -50);
            mouse_moved = true;
            last_key = WHEEL_DOWN;
#if debug
            Serial1.println("mouse wheel down");
#endif
          }
        }
        // other mouse-mapped actions can be added here
      }
      else // keyboard-mapped key handling
      {
        int k = old_key_map[r][c];

        // SHIFT keys - simple press/release behaviour
        if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT)
        {
          if (press_or_release)
          {
            shift = true;
#if debug
            Serial1.println("shift enable");
#endif
          }
          else
          {
            shift = false;
#if debug
            Serial1.println("shift disable");
#endif
          }
          continue;
        }

        // CAPS LOCK - toggle on press (common behavior)
        if (k == KEY_CAPS_LOCK)
        {
          if (press_or_release)
          {
            capslock = !capslock;
#if debug
            if (capslock)
              Serial1.println("capslock enable");
            else
              Serial1.println("capslock disable");
#endif
          }
          continue;
        }

        // compute which keycode to send based on capslock/shift
        int key_to_send = -1;
        if (capslock)
          key_to_send = old_key_map_with_capslock[r][c];
        else if (shift)
          key_to_send = old_key_map_with_shift[r][c];
        else
          key_to_send = old_key_map[r][c];

        // safety: validate keycode before using it
        if (!keycode_valid(key_to_send))
        {
#if debug
          Serial1.print("invalid keycode at r=");
          Serial1.print(r);
          Serial1.print(" c=");
          Serial1.println(c);
#endif
          continue;
        }

        // press / release the single consolidated key
        if (press_or_release)
        {
          Keyboard.press(key_to_send);
          last_key = key_to_send;
#if debug
          Serial1.print("press ( ");
          Serial1.print(char(last_key));
          Serial1.print(" / ");
          Serial1.print((const char *)key_massage_to_show[last_key]);
          // Serial1.print((const char*)key_massage_to_show[last_key]);
          // Serial1.print(key_massage_to_show[last_key]);
          Serial1.print(" ) -- capslock: ");
          Serial1.print(capslock ? "yes" : "no");
          Serial1.print(" shift: ");
          Serial1.print(shift ? "yes" : "no");
          Serial1.println();
#endif
        }
        else
        {
          Keyboard.release(key_to_send);
#if debug
          Serial1.print("release ( ");
          Serial1.print(char(last_key));
          Serial1.print(" / ");
          Serial1.print((const char *)key_massage_to_show[last_key]);
          // Serial1.print(key_massage_to_show[last_key]);
          Serial1.print(" ) -- capslock: ");
          Serial1.print(capslock ? "yes" : "no");
          Serial1.print(" shift: ");
          Serial1.print(shift ? "yes" : "no");
          Serial1.println();
#endif
        }
      } // end keyboard branch
    } // end col loop
  } // end row loop

  return true;
}

bool IIC_KEYBOARD::new_keyboard_routine(int mouse_x, int mouse_y)
{
#if debug
  Serial1.println("new keyboard routine running");
#endif
  bool keyboard_matrix[7][12] = {0};

  // --- 解析按键矩阵 ---
  for (int i = 0; i < 12; i++)
  {
    uint8_t v = iic_buffer[i + 2];
    keyboard_matrix[0][i] = v & 0b01000000;
    keyboard_matrix[1][i] = v & 0b00100000;
    keyboard_matrix[2][i] = v & 0b00010000;
    keyboard_matrix[3][i] = v & 0b00001000;
    keyboard_matrix[4][i] = v & 0b00000100;
    keyboard_matrix[5][i] = v & 0b00000010;
    keyboard_matrix[6][i] = v & 0b00000001;
  }

  // --- 特殊指令修复：必须更新 old_matrix，否则一直触发 ---
  if (iic_buffer[1] == 66)
  {
    if (keyboard_matrix[5][11] != new_keyboard_matrix_last_time[5][11])
    {
      new_keyboard_matrix_last_time[5][11] = keyboard_matrix[5][11];
      need_increase_backlight = true;
    }

    if (keyboard_matrix[4][11] != new_keyboard_matrix_last_time[4][11])
    {
      new_keyboard_matrix_last_time[4][11] = keyboard_matrix[4][11];
      need_reduce_backlight = true;
    }

    if (keyboard_matrix[3][11] != new_keyboard_matrix_last_time[3][11])
    {
      new_keyboard_matrix_last_time[3][11] = keyboard_matrix[3][11];
      need_increase_fan_speed = true;
    }

    if (keyboard_matrix[2][11] != new_keyboard_matrix_last_time[2][11])
    {
      new_keyboard_matrix_last_time[2][11] = keyboard_matrix[2][11];
      need_reduce_fan_speed = true;
    }

    return true;
  }

  // --- 解析摇杆方向 ---
  bool mouse_direction[16] = {0};
  uint8_t d = iic_buffer[0];

  if (d)
  {
    // 直接对应你的编码表
    mouse_direction[0] = (d == 0b10000000);
    mouse_direction[1] = (d == 0b11000000);
    mouse_direction[2] = (d == 0b01000000);
    mouse_direction[3] = (d == 0b01100000);
    mouse_direction[4] = (d == 0b00100000);
    mouse_direction[5] = (d == 0b00110000);
    mouse_direction[6] = (d == 0b00010000);
    mouse_direction[7] = (d == 0b00011000);
    mouse_direction[8] = (d == 0b00001000);
    mouse_direction[9] = (d == 0b00001100);
    mouse_direction[10] = (d == 0b00000100);
    mouse_direction[11] = (d == 0b00000110);
    mouse_direction[12] = (d == 0b00000010);
    mouse_direction[13] = (d == 0b00000011);
    mouse_direction[14] = (d == 0b00000001);
    mouse_direction[15] = (d == 0b10000001);
  }

  // --- 摇杆移动 ---
  for (int i = 0; i < 16; i++)
  {
    if (mouse_direction[i])
    {
      mouse_x_ = new_keyboard_angle_cos_x[i];
      mouse_y_ = new_keyboard_angle_sin_y[i];

      mouse_x = mouse_x + mouse_x_ * stick_speed;
      mouse_y = mouse_y + mouse_y_ * stick_speed;

      // 限制范围避免坐标溢出
      if (mouse_x < 0)
        mouse_x = 0;
      if (mouse_y < 0)
        mouse_y = 0;
      if (mouse_x > 32767)
        mouse_x = 32767;
      if (mouse_y > 32767)
        mouse_y = 32767;

      if (mouse_left_flag)
        MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_LEFT);
      else
        MyMouseAbsolute.move(mouse_x, mouse_y, 0);

      mouse_moved = true;
      break;
    }
  }

  // --- 整合后的 key mapping 逻辑 ---
  auto get_keycode = [&](int r, int c) -> uint8_t
  {
    // Shift 应该优先（更符合键盘行为）
    if (shift)
      return new_key_map_with_shift[r][c];

    // CapsLock 应该只影响字母
    uint8_t kc = new_key_map[r][c];
    if (capslock && kc >= 'a' && kc <= 'z')
      return kc - 'a' + 'A';

    return kc;
  };

  // --- 主按键逻辑 ---
  for (int i = 0; i < 7; i++)
  {
    for (int j = 0; j < 12; j++)
    {
      if (keyboard_matrix[i][j] == new_keyboard_matrix_last_time[i][j])
        continue;

      bool press_or_release = keyboard_matrix[i][j];
      new_keyboard_matrix_last_time[i][j] = press_or_release;

      // 鼠标键
      if (new_mouse_or_keyboard[i][j])
      {
        uint8_t mk = new_key_map[i][j];

        if (press_or_release)
        {
          if (mk == MOUSE_LEFT)
          {
            MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_LEFT);
            mouse_left_flag = true;
            last_key = MOUSE_LEFT;
          }
          else if (mk == MOUSE_RIGHT)
          {
            MyMouseAbsolute.press(mouse_x, mouse_y, MOUSE_RIGHT);
            last_key = MOUSE_RIGHT;
          }
          else if (mk == WHEEL_UP)
          {
            MyMouseAbsolute.move(mouse_x, mouse_y, 50);
            mouse_moved = true;
            last_key = WHEEL_UP;
          }
          else if (mk == WHEEL_DOWN)
          {
            MyMouseAbsolute.move(mouse_x, mouse_y, -50);
            mouse_moved = true;
            last_key = WHEEL_DOWN;
          }
        }
        else
        {
          if (mk == MOUSE_LEFT)
          {
            MyMouseAbsolute.release(mouse_x, mouse_y, MOUSE_LEFT);
            mouse_left_flag = false;
          }
          else if (mk == MOUSE_RIGHT)
          {
            MyMouseAbsolute.release(mouse_x, mouse_y, MOUSE_RIGHT);
          }
        }

        continue;
      }

      // shift key
      if (new_key_map[i][j] == KEY_LEFT_SHIFT || new_key_map[i][j] == KEY_RIGHT_SHIFT)
      {
        shift = press_or_release;
        continue;
      }

      // capslock
      if (new_key_map[i][j] == KEY_CAPS_LOCK)
      {
        if (!press_or_release)
          capslock = !capslock;
        continue;
      }

      // --- 真正的键盘键 ---
      uint8_t keycode = get_keycode(i, j);
      last_key = keycode;

      if (press_or_release)
        Keyboard.press(keycode);
      else
        Keyboard.release(keycode);
    }
  }

  return true;
}
