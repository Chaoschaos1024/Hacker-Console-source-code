#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "keyboard_keymap.h"
#include "short.h"
#include "io.h"
#include "config.h"

#if debug
SoftwareSerial ds(debug_rx, debug_tx);
#endif

// 全局变量
int loop_count = 0;
int loop_count_max = 20000;
const int keyboard_matrix_x[7] = { gpio15, gpio16, gpio17, gpio18, gpio19, gpio20, gpio21 };
const int keyboard_matrix_y[11] = { gpio22, gpio23, gpio24, gpio25, gpio26, gpio9, gpio10, gpio11, gpio12, gpio13, gpio14 };
uint8_t iic_report[keyboard_iic_length] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, keyboard_iic_crc };
uint8_t iic_report_old[keyboard_iic_length] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, keyboard_iic_crc };
uint8_t calibre_x_p = 0;
uint8_t calibre_x_m = 0;
uint8_t calibre_y_p = 0;
uint8_t calibre_y_m = 0;

void setup() {
#if debug
  ds.begin(115200);
  ds.println("setup begin");
#endif

  analogReadResolution(12);

  EEPROM.begin(256);
  if (EEPROM.read(eeprom_crc) == eeprom_crc) {
    calibre_x_m = EEPROM.read(0) * 100 + EEPROM.read(1);
    calibre_x_p = EEPROM.read(2) * 100 + EEPROM.read(3);
    calibre_y_m = EEPROM.read(4) * 100 + EEPROM.read(5);
    calibre_y_p = EEPROM.read(6) * 100 + EEPROM.read(7);
#if debug
    int x = analogRead(direction_x) + calibre_x_p - calibre_x_m;
    int y = analogRead(direction_y) + calibre_y_p - calibre_y_m;
    x = x / 16;
    y = y / 16;
    if (abs(y - 128) < 10) {
      y = 128;
    }
    if (abs(x - 128) < 10) {
      x = 128;
    }
    if (x < 0) {
      x = 0;
    }
    if (x > 255) {
      x = 255;
    }
    if (y < 0) {
      y = 0;
    }
    if (y > 255) {
      y = 255;
    }
    if (x != 128 || y != 128) {
      ds.println("校准失败，需要重新校准摇杆");
      EEPROM.write(eeprom_crc, 0);
      while (1) {
        delay(3000);
        ds.print("x:");
        ds.println(analogRead(direction_x));
        ds.print("y:");
        ds.println(analogRead(direction_y));
        ds.println("校准后");
        ds.print("x:");
        ds.println(analogRead(direction_x) + calibre_x_p - calibre_x_m);
        ds.print("y:");
        ds.println(analogRead(direction_y) + calibre_y_p - calibre_y_m);
      }
    }
#endif
  } else {
#if debug
    ds.println("执行第一次校准中");
#endif
    for (int i = 0; i < 256; i++) {
      EEPROM.write(i, 0);
    }
    int x = 0;
    int y = 0;
    for (int j = 0; j < 10; j++) {
      x = x + analogRead(direction_x);
      y = y + analogRead(direction_y);
    }
    x = x / 10;
    y = y / 10;
    if (x != 2047) {
      if (x > 2047) {
        calibre_x_m = x - 2047;
      } else {
        calibre_x_p = 2047 - x;
      }
    }
    if (y != 2047) {
      if (y > 2047) {
        calibre_y_m = y - 2047;
      } else {
        calibre_y_p = 2047 - y;
      }
    }
    EEPROM.write(0, calibre_x_m / 100);
    EEPROM.write(1, calibre_x_m % 100);

    EEPROM.write(2, calibre_x_p / 100);
    EEPROM.write(3, calibre_x_p % 100);

    EEPROM.write(4, calibre_y_m / 100);
    EEPROM.write(5, calibre_y_m % 100);

    EEPROM.write(6, calibre_y_p / 100);
    EEPROM.write(7, calibre_y_p % 100);

    EEPROM.write(eeprom_crc, eeprom_crc);

#if debug
    ds.println("第一次校准完成");
    delay(10000);
#endif
  }

  pinMode(i2c0_int, output);
  digitalwrite(i2c0_int, high);
  wire1.setSDA(i2c1_sda);
  wire1.setSCL(i2c1_scl);
  wire1.begin(keyboard_iic_address);
  wire1.onRequest(key_scan);

  pinMode(gpio21, input_pulldown);
  pinMode(gpio20, input_pulldown);
  pinMode(gpio19, input_pulldown);
  pinMode(gpio18, input_pulldown);
  pinMode(gpio17, input_pulldown);
  pinMode(gpio16, input_pulldown);
  pinMode(gpio15, input_pulldown);

  pinMode(gpio9, output);
  pinMode(gpio10, output);
  pinMode(gpio11, output);
  pinMode(gpio12, output);
  pinMode(gpio13, output);
  pinMode(gpio14, output);
  pinMode(gpio22, output);
  pinMode(gpio23, output);
  pinMode(gpio24, output);
  pinMode(gpio25, output);
  pinMode(gpio26, output);

  pinMode(press_stick, input_pullup);

  digitalwrite(i2c0_int, low);
#if debug
  ds.println("setup finished");
#endif
}

void loop() {
#if debug
  ds.println("loop start");
  ds.print("wire : ");
  ds.println(keyboard_iic_address);
#endif
  int x = analogRead(direction_x) + calibre_x_p - calibre_x_m;
  int y = analogRead(direction_y) + calibre_y_p - calibre_y_m;
  x = x / 16;
  y = y / 16;
  if (abs(y - 128) < blind_area) {
    y = 128;
  }
  if (abs(x - 128) < blind_area) {
    x = 128;
  }
  if (x < 0) {
    x = 0;
  }
  if (x > 255) {
    x = 255;
  }
  if (y < 0) {
    y = 0;
  }
  if (y > 255) {
    y = 255;
  }
  iic_report[0] = x;
  iic_report[1] = y;
  int a = !digitalread(press_stick);
  iic_report[2] = a * 33 + 33;
  bool keyboard_matrix[7][11] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  };
  for (int j = 0; j < 7; j++) {
    for (int k = 0; k < 11; k++) {
      for (int l = 0; l < 11; l++) {
        pinMode(keyboard_matrix_y[l], INPUT);
      }
      pinMode(keyboard_matrix_y[k], OUTPUT);
      digitalwrite(keyboard_matrix_y[k], high);
      asm("nop");
      if (digitalread(keyboard_matrix_x[j])) {
        keyboard_matrix[j][k] = 1;
#if debug
        ds.print("keyboard_matrix : ");
        ds.print(j);
        ds.print("\t");
        ds.print(k);
        ds.print("\t");
        ds.print("key is ");
        ds.println(key_map[j][k], HEX);
#endif
      } else {
        keyboard_matrix[j][k] = 0;
      }
      digitalwrite(keyboard_matrix_y[k], low);
    }
  }
  for (int i = 0; i < 11; i++) {
    iic_report[i + 3] = 64 * keyboard_matrix[0][i] + 32 * keyboard_matrix[1][i] + 16 * keyboard_matrix[2][i] + 8 * keyboard_matrix[3][i] + 4 * keyboard_matrix[4][i] + 2 * keyboard_matrix[5][i] + keyboard_matrix[6][i];
  }
  iic_report[keyboard_iic_length - 1] = keyboard_iic_crc;


#if debug
  ds.print("iic_report : ");
  for (int l = 0; l < keyboard_iic_length; l++) {
    ds.print(iic_report[l]);
    ds.print("/");
  }
  ds.println("");
  ds.print("  ");
  ds.print("\t");
  for (int j = 0; j < map_y; j++) {
    ds.print(j);
    ds.print("\t");
  }
  ds.println();
  for (int i = 0; i < map_x; i++) {
    ds.print(i);
    ds.print(":");
    ds.print("\t");
    for (int j = 0; j < map_y; j++) {
      //ds.print(keyboard_matrix[i][j]);
      if (keyboard_matrix[i][j]) {
        ds.print("@");
      } else {
        ds.print(" ");
      }
      ds.print("\t");
    }
    ds.println();
    ds.println("----------------------------------------------------------------------------------------------------------------");
  }
#endif

#if debug
  ds.println("loop finish");
  ds.println("-------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
  delay(20);
#endif
}


void key_scan() {
  for (int i = 0; i < keyboard_iic_length; i++) {
    wire1.write(iic_report[i]);
  }
}