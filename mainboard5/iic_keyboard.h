/*
 * @Author: 无序熵增
 * @Date: 2025-07-25 11:54:52
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-12-09 12:30:12
 * @Description:
 *
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved.
 */
#ifndef IIC_KEYBOARD_H
#define IIC_KEYBOARD_H
#include <Keyboard.h>
#include "MyMouseAbsolute.h"
#include <SoftwareSerial.h>
/// #include <RP2040USB.h>
// #include "tusb.h"
// #include "class/hid/hid_device.h"

#define keyboard_iic_length 15

#define MOUSE_LEFT 0x01
#define MOUSE_RIGHT 0x02
#define MOUSE_MIDDLE 0x04
#define WHEEL_UP 0x05
#define WHEEL_DOWN 0x06
// 指令
#define light_up 0x7
#define light_down 0x08
#define fan_up 0x09
#define fan_down 0x10
#define lock_up_ 0x11
#define unlock_ 0x12
// 键盘
#define KEY_LEFT_CTRL 0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT 0x82
#define KEY_LEFT_GUI 0x83
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT 0x86
#define KEY_RIGHT_GUI 0x87

#define KEY_RETURN 0xB0
#define KEY_ESC 0xB1
#define KEY_BACKSPACE 0xB2
#define KEY_TAB 0xB3

#define KEY_CAPS_LOCK 0xC1
#define KEY_F1 0xC2
#define KEY_F2 0xC3
#define KEY_F3 0xC4
#define KEY_F4 0xC5
#define KEY_F5 0xC6
#define KEY_F6 0xC7
#define KEY_F7 0xC8
#define KEY_F8 0xC9
#define KEY_F9 0xCA
#define KEY_F10 0xCB
#define KEY_F11 0xCC
#define KEY_F12 0xCD
#define KEY_PRINT_SCREEN 0xCE
#define KEY_SCROLL_LOCK 0xCF
#define KEY_PAUSE 0xD0
#define KEY_INSERT 0xD1
#define KEY_HOME 0xD2
#define KEY_PAGE_UP 0xD3
#define KEY_DELETE 0xD4
#define KEY_END 0xD5
#define KEY_PAGE_DOWN 0xD6
#define KEY_RIGHT_ARROW 0xD7
#define KEY_LEFT_ARROW 0xD8
#define KEY_DOWN_ARROW 0xD9
#define KEY_UP_ARROW 0xDA
#define KEY_NUM_LOCK 0xDB
#define KEY_KP_SLASH 0xDC
#define KEY_KP_ASTERISK 0xDD
#define KEY_KP_MINUS 0xDE
#define KEY_KP_PLUS 0xDF
#define KEY_KP_ENTER 0xE0
#define KEY_KP_1 0xE1
#define KEY_KP_2 0xE2
#define KEY_KP_3 0xE3
#define KEY_KP_4 0xE4
#define KEY_KP_5 0xE5
#define KEY_KP_6 0xE6
#define KEY_KP_7 0xE7
#define KEY_KP_8 0xE8
#define KEY_KP_9 0xE9
#define KEY_KP_0 0xEA
#define KEY_KP_DOT 0xEB
#define KEY_MENU 0xED
#define KEY_F13 0xF0
#define KEY_F14 0xF1
#define KEY_F15 0xF2
#define KEY_F16 0xF3
#define KEY_F17 0xF4
#define KEY_F18 0xF5
#define KEY_F19 0xF6
#define KEY_F20 0xF7
#define KEY_F21 0xF8
#define KEY_F22 0xF9
#define KEY_F23 0xFA
#define KEY_F24 0xFB

class IIC_KEYBOARD
{
public:
    bool begin();
    bool routine(int mouse_x, int mouse_y);
    bool is_iic_keyboard_connect();

    bool new_keyboard_flag = false;
    bool new_address_keyboard_flag = false;
    bool old_keyboard_flag = false;
    uint8_t keyboard_connect = 0; // 0无键盘 1老键盘 2新键盘 4新地址键盘,累计加上去
    bool capslock = false;
    bool shift = false;
    bool need_increase_backlight = false;
    bool need_reduce_backlight = false;
    bool need_increase_fan_speed = false;
    bool need_reduce_fan_speed = false;
    int mouse_x_ = 128;
    int mouse_y_ = 128;
    const float stick_speed = 0.4;
    unsigned int pressed = 0;
    unsigned int last_key = 0;
    bool mouse_moved = false;

private:
    bool read_keyboard_iic();
    bool read_new_keyboard_iic();
    bool read_iic_data(uint8_t address);
    bool new_keyboard_routine(int mouse_x, int mouse_y);
    bool old_keyboard_routine(int mouse_x, int mouse_y);
    bool reconnect_keyboard();
    const uint8_t keyboard_iic_address = 0x30;
    const uint8_t new_keyboard_iic_address = 0x31;

    const uint8_t old_keyboard_crc = 0x11;
    const uint8_t new_keyboard_crc = 0x12;
    const uint8_t pressed_max = 100;
    const uint8_t mouse_dead_zone = 30;

    bool mouse_left_flag = false;
    bool need_to_release_touch=false;

    uint8_t iic_buffer[keyboard_iic_length];

    bool old_keyboard_matrix_last_time[7][11] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    bool old_press_or_release[7][11] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    const uint8_t old_key_map[7][11] = {
        {' ', 'n', 'h', 'y', '`', '1', '2', 'u', 'j', 'm', KEY_RIGHT_ALT},
        {' ', 'b', 'g', 't', 0, '3', '4', 'i', 'k', ',', KEY_RIGHT_CTRL},
        {KEY_F5, 'v', 'f', 'r', 0, '5', '6', 'o', 'l', '.', KEY_RIGHT_GUI},
        {KEY_F4, 'c', 'd', 'e', WHEEL_UP, '7', '8', 'p', ';', '/', KEY_LEFT_ARROW},
        {KEY_F3, 'x', 's', 'w', MOUSE_LEFT, '9', '0', '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'z', 'a', 'q', MOUSE_RIGHT, '-', '=', ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};
    const uint8_t old_key_map_with_shift[7][11] = {
        {' ', 'N', 'H', 'Y', '~', '!', '@', 'U', 'J', 'M', KEY_RIGHT_ALT},
        {' ', 'B', 'G', 'T', 0, '#', '$', 'I', 'K', '<', KEY_RIGHT_CTRL},
        {KEY_F5, 'V', 'F', 'R', 0, '%', '^', 'O', 'L', '>', KEY_RIGHT_GUI},
        {KEY_F4, 'C', 'D', 'E', WHEEL_UP, '&', '*', 'P', ':', '?', KEY_LEFT_ARROW},
        {KEY_F3, 'X', 'S', 'W', MOUSE_LEFT, '(', ')', '{', '\"', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'Z', 'A', 'Q', MOUSE_RIGHT, '_', '+', '}', '|', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};
    const uint8_t old_key_map_with_capslock[7][11] = {
        {' ', 'n', 'h', 'y', '`', '1', '2', 'u', 'j', 'm', KEY_RIGHT_ALT},
        {' ', 'b', 'g', 't', 0, '3', '4', 'i', 'k', ',', KEY_RIGHT_CTRL},
        {KEY_F5, 'v', 'f', 'r', 0, '5', '6', 'o', 'l', '.', KEY_RIGHT_GUI},
        {KEY_F4, 'c', 'd', 'e', WHEEL_UP, '7', '8', 'p', ';', '/', KEY_LEFT_ARROW},
        {KEY_F3, 'x', 's', 'w', MOUSE_LEFT, '9', '0', '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'z', 'a', 'q', MOUSE_RIGHT, '-', '=', ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};

    const bool old_mouse_or_keyboard[7][11] = {
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
        },
    };

    bool new_keyboard_matrix_last_time[7][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    bool new_press_or_release[7][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    const int radius = 91;
    const int x_center = 128;
    const int y_center = 128;
    const int new_keyboard_angle[16] = {
        int(90),
        int(90 - 22.5),
        int(90 - 45),
        int(90 - 67.5),
        int(0),
        int(0 - 22.5),
        int(0 - 45),
        int(0 - 67.5),
        int(-90),
        int(-90 - 22.5),
        int(-90 - 45),
        int(-90 - 67.5),
        int(-180),
        int(-180 - 22.5),
        int(-180 - 45),
        int(-180 - 67.5)};
    const int new_keyboard_angle_sin_y[16] = {
        y_center - 128, // sin(90) * 91 + 128 = 128
        y_center - 113, // sin(67.5) * 91 + 128 = 113.98
        y_center - 90,  // sin(45) * 91 + 128 = 90.99
        y_center - 64,  // sin(22.5) * 91 + 128 = 63.64
        y_center - 0,   // sin(0) * 91 + 128 = 0
        y_center + 64,  // sin(-22.5) * 91 + 128 = -64.36
        y_center + 90,  // sin(-45) * 91 + 128 = -89.99
        y_center + 113, // sin(-67.5) * 91 + 128 = -113.98
        y_center + 128, // sin(-90) * 91 + 128 = -128
        y_center + 113, // sin(-112.5) * 91 + 128 = -113.98
        y_center + 90,  // sin(-135) * 91 + 128 = -89.99
        y_center - 0,   // sin(-180) * 91 + 128 = 0
        y_center - 64,  // sin(-157.5) * 91 + 128 = 64
        y_center - 90,  // sin(-135) * 91 + 128 = 89.99
        y_center - 113, // sin(-112.5) * 91 + 128 = 113.98
        y_center - 128  // sin(-90) * 91 + 128 = 128
    };

    // 计算后的 X (cos) 坐标
    const int new_keyboard_angle_cos_x[16] = {
        x_center + 0,   // cos(90) * 91 + 128 = 0
        x_center + 45,  // cos(67.5) * 91 + 128 = 45.25
        x_center + 90,  // cos(45) * 91 + 128 = 89.99
        x_center + 114, // cos(22.5) * 91 + 128 = 113.98
        x_center + 128, // cos(0) * 91 + 128 = 128
        x_center + 114, // cos(-22.5) * 91 + 128 = 113.98
        x_center + 90,  // cos(-45) * 91 + 128 = 89.99
        x_center + 45,  // cos(-67.5) * 91 + 128 = 45.25
        x_center + 0,   // cos(-90) * 91 + 128 = 0
        x_center - 45,  // cos(-112.5) * 91 + 128 = -45.25
        x_center - 90,  // cos(-135) * 91 + 128 = -89.99
        x_center - 128, // cos(-180) * 91 + 128 = -128
        x_center - 114, // cos(-157.5) * 91 + 128 = -113.98
        x_center - 90,  // cos(-135) * 91 + 128 = -89.99
        x_center - 45,  // cos(-112.5) * 91 + 128 = -45.25
        x_center + 0    // cos(-90) * 91 + 128 = 0
    };
    const uint8_t new_key_map[7][12] = {
        {' ', 'n', 'h', 'y', '6', KEY_F7, '7', 'u', 'j', 'm', KEY_RIGHT_ALT, KEY_F6},
        {' ', 'b', 'g', 't', '5', '-', '8', 'i', 'k', ',', KEY_RIGHT_CTRL, KEY_F5},
        {KEY_PAGE_DOWN, 'v', 'f', 'r', '4', '=', '9', 'o', 'l', '.', KEY_RIGHT_GUI, KEY_F4},
        {KEY_PAGE_UP, 'c', 'd', 'e', '3', KEY_BACKSPACE, '0', 'p', ';', '/', KEY_LEFT_ARROW, KEY_F3},
        {KEY_LEFT_ALT, 'x', 's', 'w', '2', 0, MOUSE_LEFT, '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_F2},
        {KEY_LEFT_GUI, 'z', 'a', 'q', '1', 0, 0, ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW, KEY_F1},
        {KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, '`', 0, MOUSE_RIGHT, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END, KEY_ESC}};
    const uint8_t new_key_map_with_shift[7][12] = {
        {' ', 'N', 'H', 'Y', '^', KEY_F7, '&', 'U', 'J', 'M', KEY_RIGHT_ALT, KEY_F6},
        {' ', 'B', 'G', 'T', '%', '_', '*', 'I', 'K', '<', KEY_RIGHT_CTRL, KEY_F5},
        {KEY_PAGE_DOWN, 'V', 'F', 'R', '$', '+', '(', 'O', 'L', '>', KEY_RIGHT_GUI, KEY_F4},
        {KEY_PAGE_UP, 'C', 'D', 'E', '#', KEY_BACKSPACE, ')', 'P', ':', '?', KEY_LEFT_ARROW, KEY_F3},
        {KEY_LEFT_ALT, 'X', 'S', 'W', '@', 0, MOUSE_LEFT, '{', '\"', KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_F2},
        {KEY_LEFT_GUI, 'Z', 'A', 'Q', '!', 0, 0, '}', '|', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW, KEY_F1},
        {KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, '~', 0, MOUSE_RIGHT, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END, KEY_ESC}};
    const uint8_t new_key_map_with_capslock[7][12] = {
        {' ', 'n', 'h', 'y', '6', KEY_F7, '7', 'u', 'j', 'm', KEY_RIGHT_ALT, KEY_F6},
        {' ', 'b', 'g', 't', '5', '-', '8', 'i', 'k', ',', KEY_RIGHT_CTRL, KEY_F5},
        {KEY_PAGE_DOWN, 'v', 'f', 'r', '4', '=', '9', 'o', 'l', '.', KEY_RIGHT_GUI, KEY_F4},
        {KEY_PAGE_UP, 'c', 'd', 'e', '3', KEY_BACKSPACE, '0', 'p', ';', '/', KEY_LEFT_ARROW, KEY_F3},
        {KEY_LEFT_ALT, 'x', 's', 'w', '2', 0, MOUSE_LEFT, '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_F2},
        {KEY_LEFT_GUI, 'z', 'a', 'q', '1', 0, 0, ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW, KEY_F1},
        {KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, '`', 0, MOUSE_RIGHT, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END, KEY_ESC}};

    const bool new_mouse_or_keyboard[7][12] = {
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
        },
    };

public:
    const uint8_t key_massage_to_show[256][9] = {
        {" "},        // 0x00
        {"MOU_L"},    // 0x01
        {"MOU_R"},    // 0x02
        {" "},        // 0x03
        {"MOU_M"},    // 0x04
        {"WHE_U"},    // 0x05
        {"WHE_M"},    // 0x06
        {"LIG_U"},    // 0x07
        {"LIG_D"},    // 0x08
        {"FAN_U"},    // 0x09
        {"FAN_D"},    // 0xA
        {"LOCK"},     // 0xB
        {"UNLOCK"},   // 0xC
        {""},         // 0xD
        {""},         // 0xE
        {""},         // 0xF
        {""},         // 0x10
        {""},         // 0x11
        {""},         // 0x12
        {""},         // 0x13
        {""},         // 0x14
        {""},         // 0x15
        {""},         // 0x16
        {""},         // 0x17
        {""},         // 0x18
        {""},         // 0x19
        {""},         // 0x1A
        {""},         // 0x1B
        {""},         // 0x1C
        {""},         // 0x1D
        {""},         // 0x1E
        {""},         // 0x1F
        {"SPACE"},    // 0x20
        {"1/!"},      // 0x21
        {"'/\""},     // 0x22
        {"3/#"},      // 0x23
        {"4/$"},      // 0x24
        {"5/%"},      // 0x25
        {"6/&"},      // 0x26
        {"'/\""},     // 0x27
        {"9/("},      // 0x28
        {"0/)"},      // 0x29
        {"8/*"},      // 0x2A
        {""},         // 0x2B
        {",/<"},      // 0x2C
        {"-/_"},      // 0x2D
        {"./>"},      // 0x2E
        {"//?"},      // 0x2F
        {"0/)"},      // 0x30
        {"1/!"},      // 0x31
        {"2/@"},      // 0x32
        {"3/#"},      // 0x33
        {"4/$"},      // 0x34
        {"5/%"},      // 0x35
        {"6/^"},      // 0x36
        {"7/&"},      // 0x37
        {"8/*"},      // 0x38
        {"9/("},      // 0x39
        {";/:"},      // 0x3A
        {";/:"},      // 0x3B
        {",/<"},      // 0x3C
        {"=/+"},      // 0x3D
        {"./>"},      // 0x3E
        {"//?"},      // 0x3F
        {"2/@"},      // 0x40
        {"A"},        // 0x41
        {"B"},        // 0x42
        {"C"},        // 0x43
        {"D"},        // 0x44
        {"E"},        // 0x45
        {"F"},        // 0x46
        {"G"},        // 0x47
        {"H"},        // 0x48
        {"I"},        // 0x49
        {"J"},        // 0x4A
        {"K"},        // 0x4B
        {"L"},        // 0x4C
        {"M"},        // 0x4D
        {"N"},        // 0x4E
        {"O"},        // 0x4F
        {"P"},        // 0x50
        {"Q"},        // 0x51
        {"R"},        // 0x52
        {"S"},        // 0x53
        {"T"},        // 0x54
        {"U"},        // 0x55
        {"V"},        // 0x56
        {"W"},        // 0x57
        {"X"},        // 0x58
        {"Y"},        // 0x59
        {"Z"},        // 0x5A
        {"[/{"},      // 0x5B
        {"\\/|"},     // 0x5C
        {"]/}"},      // 0x5D
        {"6/^"},      // 0x5E
        {"-/_"},      // 0x5F
        {"`/~"},      // 0x60
        {"a"},        // 0x61
        {"b"},        // 0x62
        {"c"},        // 0x63
        {"d"},        // 0x64
        {"e"},        // 0x65
        {"f"},        // 0x66
        {"g"},        // 0x67
        {"h"},        // 0x68
        {"i"},        // 0x69
        {"j"},        // 0x6A
        {"k"},        // 0x6B
        {"l"},        // 0x6C
        {"m"},        // 0x6D
        {"n"},        // 0x6E
        {"o"},        // 0x6F
        {"p"},        // 0x70
        {"q"},        // 0x71
        {"r"},        // 0x72
        {"s"},        // 0x73
        {"t"},        // 0x74
        {"u"},        // 0x75
        {"v"},        // 0x76
        {"w"},        // 0x77
        {"X"},        // 0x78
        {"y"},        // 0x79
        {"Z"},        // 0x7A
        {"[/{"},      // 0x7B
        {"\\/|"},     // 0x7C
        {"]/}"},      // 0x7D
        {"`/~"},      // 0x7E
        {""},         // 0x7F
        {"CTRL"},     // 0x80
        {"SHIFT"},    // 0x81
        {"ALT"},      // 0x82
        {"GUI"},      // 0x83
        {"CTRL"},     // 0x84
        {"SHIFT"},    // 0x85
        {"ALT"},      // 0x86
        {"GUI"},      // 0x87
        {""},         // 0x88
        {""},         // 0x89
        {""},         // 0x8A
        {""},         // 0x8B
        {""},         // 0x8C
        {""},         // 0x8D
        {""},         // 0x8E
        {""},         // 0x8F
        {""},         // 0x90
        {""},         // 0x91
        {""},         // 0x92
        {""},         // 0x93
        {""},         // 0x94
        {""},         // 0x95
        {""},         // 0x96
        {""},         // 0x97
        {""},         // 0x98
        {""},         // 0x99
        {""},         // 0x9A
        {""},         // 0x9B
        {""},         // 0x9C
        {""},         // 0x9D
        {""},         // 0x9E
        {""},         // 0x9F
        {""},         // 0xA0
        {""},         // 0xA1
        {""},         // 0xA2
        {""},         // 0xA3
        {""},         // 0xA4
        {""},         // 0xA5
        {""},         // 0xA6
        {""},         // 0xA7
        {""},         // 0xA8
        {""},         // 0xA9
        {""},         // 0xAA
        {""},         // 0xAB
        {""},         // 0xAC
        {""},         // 0xAD
        {""},         // 0xAE
        {""},         // 0xAF
        {""},         // 0xB0
        {"ESC"},      // 0xB1
        {"BACKSP"},   // 0xB2
        {"TAB"},      // 0xB3
        {""},         // 0xB4
        {""},         // 0xB5
        {""},         // 0xB6
        {""},         // 0xB7
        {""},         // 0xB8
        {""},         // 0xB9
        {""},         // 0xBA
        {""},         // 0xBB
        {""},         // 0xBC
        {""},         // 0xBD
        {""},         // 0xBE
        {""},         // 0xBF
        {""},         // 0xC0
        {"CAPSLOCK"}, // 0xC1
        {"F1"},       // 0xC2
        {"F2"},       // 0xC3
        {"F3"},       // 0xC4
        {"F4"},       // 0xC5
        {"F5"},       // 0xC6
        {"F6"},       // 0xC7
        {"F7"},       // 0xC8
        {""},         // 0xC9
        {""},         // 0xCA
        {""},         // 0xCB
        {""},         // 0xCC
        {""},         // 0xCD
        {""},         // 0xCE
        {""},         // 0xCF
        {""},         // 0xD0
        {""},         // 0xD1
        {"HOME"},     // 0xD2
        {"PAGE_UP"},  // 0xD3
        {"DELETE"},   // 0xD4
        {"END"},      // 0xD5
        {"PAGE_DN"},  // 0xD6
        {"RIGHT"},    // 0xD7
        {"LEFT"},     // 0xD8
        {"DOWN"},     // 0xD9
        {"UP"},       // 0xDA
        {""},         // 0xDB
        {""},         // 0xDC
        {""},         // 0xDD
        {""},         // 0xDE
        {""},         // 0xDF
        {"ENTER"},    // 0xE0
        {""},         // 0xE1
        {""},         // 0xE2
        {""},         // 0xE3
        {""},         // 0xE4
        {""},         // 0xE5
        {""},         // 0xE6
        {""},         // 0xE7
        {""},         // 0xE8
        {""},         // 0xE9
        {""},         // 0xEA
        {""},         // 0xEB
        {""},         // 0xEC
        {""},         // 0xED
        {""},         // 0xEE
        {""},         // 0xEF
        {""},         // 0xF0
        {""},         // 0xF1
        {""},         // 0xF2
        {""},         // 0xF3
        {""},         // 0xF4
        {""},         // 0xF5
        {""},         // 0xF6
        {""},         // 0xF7
        {""},         // 0xF8
        {""},         // 0xF9
        {""},         // 0xFA
        {""},         // 0xFB
        {""},         // 0xFC
        {""},         // 0xFD
        {""},         // 0xFE
        {""},         // 0xFF
    };
};
#endif
