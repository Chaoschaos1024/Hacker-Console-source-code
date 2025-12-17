/*
   @Author: 无序熵增
   @Date: 2025-09-16 16:04:57
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-11-04 19:22:24
   @Description:

   Copyright (c) 2025 by 无序熵增, All Rights Reserved.
*/
/*
    MouseAbsolute.h

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

#pragma once

#ifdef USE_TINYUSB
#error MouseAbsolute is not compatible with Adafruit TinyUSB
#endif

#include <HID_Mouse.h>

class MyMouseAbsolute_ : public HID_Mouse {
  public:
    MyMouseAbsolute_(void);
    virtual void move(int x, int y, signed char wheel = 0) override;
    void press(int x, int y, uint8_t b);
    void release(int x, int y, uint8_t b);

  private:
    uint8_t _id; 
};
extern MyMouseAbsolute_ MyMouseAbsolute;
