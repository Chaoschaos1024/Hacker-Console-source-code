/*
 * @Author: 无序熵增
 * @Date: 2023-12-20 23:22:55
 * @LastEditors: 无序熵增
 * @LastEditTime: 2024-03-18 16:43:48
 * @Description:
 *
 * Copyright (c) 2023 by 无序熵增, All Rights Reserved.
 */
//指令
#define key_press_command 0x01
#define key_release_command 0x02
#define mouse_move_command 0x03
#define mouse_press_command 0x04 
#define mouse_release_command 0x05
#define mouse_wheel_command 0x06
#define machine_command 0x07
// 鼠标
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
#define lock 0x11
#define unlock 0x12
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

bool keyboard_matrix_last_time[7][11] =
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
bool press_or_release[7][11] =
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
bool shift = 0;
bool capslock = 0;
const char key_map[7][11] =
    {
        {' ', 'n', 'h', 'y', '`', '1', '2', 'u', 'j', 'm', KEY_RIGHT_ALT},
        {' ', 'b', 'g', 't', 0, '3', '4', 'i', 'k', ',', KEY_RIGHT_CTRL},
        {KEY_F5, 'v', 'f', 'r', 0, '5', '6', 'o', 'l', '.', KEY_RIGHT_GUI},
        {KEY_F4, 'c', 'd', 'e', WHEEL_UP, '7', '8', 'p', ';', '/', KEY_LEFT_ARROW},
        {KEY_F3, 'x', 's', 'w', MOUSE_LEFT, '9', '0', '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'z', 'a', 'q', MOUSE_RIGHT, '-', '=', ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};
const char key_map_with_shift[7][11] =
    {
        {' ', 'N', 'H', 'Y', '~', '!', '@', 'U', 'J', 'M', KEY_RIGHT_ALT},
        {' ', 'B', 'G', 'T', 0, '#', '$', 'I', 'K', '<', KEY_RIGHT_CTRL},
        {KEY_F5, 'V', 'F', 'R', 0, '%', '^', '&', 'L', '>', KEY_RIGHT_GUI},
        {KEY_F4, 'C', 'D', 'E', WHEEL_UP, '&', '*', 'P', ':', '?', KEY_LEFT_ARROW},
        {KEY_F3, 'X', 'S', 'W', MOUSE_LEFT, '(', ')', '{', '\"', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'Z', 'A', 'Q', MOUSE_RIGHT, '_', '+', '}', '|', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};
const char key_map_with_capslock[7][11] =
    {
        {' ', 'N', 'H', 'Y', '`', '1', '2', 'u', 'J', 'M', KEY_RIGHT_ALT},
        {' ', 'B', 'G', 'T', 0, '3', '4', 'I', 'K', ',', KEY_RIGHT_CTRL},
        {KEY_F5, 'V', 'F', 'R', 0, '5', '6', 'o', 'L', '.', KEY_RIGHT_GUI},
        {KEY_F4, 'C', 'D', 'E', WHEEL_UP, '7', '8', 'p', ';', '/', KEY_LEFT_ARROW},
        {KEY_F3, 'X', 'S', 'W', MOUSE_LEFT, '9', '0', '[', '\'', KEY_UP_ARROW, KEY_DOWN_ARROW},
        {KEY_F2, 'Z', 'A', 'Q', MOUSE_RIGHT, '-', '=', ']', '\\', KEY_RIGHT_SHIFT, KEY_RIGHT_ARROW},
        {KEY_F1, KEY_LEFT_SHIFT, KEY_CAPS_LOCK, KEY_TAB, WHEEL_DOWN, KEY_BACKSPACE, KEY_ESC, KEY_DELETE, KEY_KP_ENTER, KEY_HOME, KEY_END}};
const bool mouse_or_keyboard[7][11]=
        {
            {0,0,0,0,0,0,0,0,0,0,0,},
            {0,0,0,0,0,0,0,0,0,0,0,},            
            {0,0,0,0,0,0,0,0,0,0,0,},
            {0,0,0,0,1,0,0,0,0,0,0,},
            {0,0,0,0,1,0,0,0,0,0,0,},
            {0,0,0,0,1,0,0,0,0,0,0,},
            {0,0,0,0,1,0,0,0,0,0,0,},
        };