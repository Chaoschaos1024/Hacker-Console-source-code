/*
   @Author: 无序熵增
   @Date: 2025-09-16 16:04:57
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-11-04 17:59:42
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
/*
    Mouse.cpp

    Copyright (c) 2015, Arduino LLC
    Original code (pre-library): Copyright (c) 2011, Peter Barrett

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "MyMouseAbsolute.h"
// #include <USB.h>
#include <RP2040USB.h>

#include "tusb.h"
#include "class/hid/hid_device.h"

void __USBInstallAbsoluteMouse() { /* noop */ }

MyMouseAbsolute_::MyMouseAbsolute_(void) : HID_Mouse(true)
{
  /* noop */
}

void MyMouseAbsolute_::move(int x, int y, signed char wheel)
{
  CoreMutex m(&__usb_mutex);
  tud_task();
  if (tud_hid_ready())
  {
    tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, limit_xy(x), limit_xy(y), wheel, 0);
  }
  tud_task();
}

void MyMouseAbsolute_::press(int x,int y,uint8_t b)
{
  _buttons = b;
  CoreMutex m(&__usb_mutex);
  tud_task();
  if (tud_hid_ready())
  {
    tud_hid_abs_mouse_report(__USBGetMouseReportID(), _buttons, limit_xy(x), limit_xy(y), 0, 0);
  }
  tud_task();
}

void MyMouseAbsolute_::release(int x, int y, uint8_t b)
{
  _buttons = b;
  CoreMutex m(&__usb_mutex);
  tud_task();
  if (tud_hid_ready())
  {
    tud_hid_abs_mouse_report(__USBGetMouseReportID(), 0, limit_xy(x), limit_xy(y), 0, 0);
  }
  tud_task();
}

MyMouseAbsolute_ MyMouseAbsolute;
/*

static const uint8_t desc_hid_report_absmouse[] = { TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(1)) };

MyMouseAbsolute_::MyMouseAbsolute_(void) : HID_Mouse(true) {
}

void MyMouseAbsolute_::begin() {
  if (_running) {
    return;
  }
  USB.disconnect();
  _id = USB.registerHIDDevice(desc_hid_report_absmouse, sizeof(desc_hid_report_absmouse), 21, 0x0002);
  USB.connect();
  HID_Mouse::begin();
}

void MyMouseAbsolute_::end() {
  if (_running) {
    USB.disconnect();
    USB.unregisterHIDDevice(_id);
    USB.connect();
  }
  HID_Mouse::end();
}

void MyMouseAbsolute_::move(int x, int y, signed char wheel) {
  if (!_running)
  {
    return;
  }
  CoreMutex m(&USB.mutex);
  tud_task();
  if (tud_hid_ready())
  {
    tud_hid_abs_mouse_report(USB.findHIDReportID(_id), _buttons, limit_xy(x), limit_xy(y), wheel, 0);
  }
  tud_task();
}
void MyMouseAbsolute_::press(uint8_t b) {
  if (!_running)
  {
    return;
  }
  if (b & MOUSE_LEFT)   _buttons |= 0x01;
  if (b & MOUSE_RIGHT)  _buttons |= 0x02;
  if (b & MOUSE_MIDDLE) _buttons |= 0x04;
  CoreMutex m(&USB.mutex);
  tud_task();
  if (tud_hid_ready()) {
    _buttons = b;
    tud_hid_abs_mouse_report(USB.findHIDReportID(_id), _buttons, limit_xy(_last_x), limit_xy(_last_y), 0, 0);
  }
  tud_task();
}

void MyMouseAbsolute_::release(uint8_t b) {
  if (!_running)
  {
    return;
  }
  if (b & MOUSE_LEFT)   _buttons &= ~0x01;
  if (b & MOUSE_RIGHT)  _buttons &= ~0x02;
  if (b & MOUSE_MIDDLE) _buttons &= ~0x04;
  CoreMutex m(&USB.mutex);
  tud_task();
  if (tud_hid_ready()) {
    _buttons = 0;
    tud_hid_abs_mouse_report(USB.findHIDReportID(_id), _buttons, limit_xy(_last_x), limit_xy(_last_y), 0, 0);
  }
  tud_task();
}

MyMouseAbsolute_ MyMouseAbsolute;
*/
