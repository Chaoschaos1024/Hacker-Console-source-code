#include <Wire.h>
#include "io.h"
#include "keymap.h"

#define debug 0
#define version 3
#define keyboard_iic_address 0x30
#define keyboard_iic_length 15
#define keyboard_iic_crc 0x12

#define wire Wire
#define wire1 Wire1
#define input INPUT
#define output OUTPUT
#define input_pullup INPUT_PULLUP
#define input_pulldown INPUT_PULLDOWN
#define high HIGH
#define low LOW
#define digitalread digitalRead
#define digitalwrite digitalWrite
#define analogread analogRead
#define eeprom EEPROM
#define pinmode pinMode
#define ma MouseAbsolute
#define kb Keyboard
#define se Serial
#define df dataFile
#define fw FILE_WRITE
#define spc setPixelColor
#define cd clearDisplay

#define map_x 9
#define map_y 13
const int keyboard_matrix_x[map_x] = { gpio15, gpio16, gpio17, gpio18, gpio19, gpio20, gpio21, gpio29, gpio8 };
const int keyboard_matrix_y[map_y] = { gpio22, gpio23, gpio24, gpio25, gpio26, gpio9, gpio10, gpio11, gpio12, gpio13, gpio14, gpio27, gpio28 };
uint8_t iic_report[keyboard_iic_length] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, keyboard_iic_crc };
uint8_t iic_report_old[keyboard_iic_length] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, keyboard_iic_crc };

int loop_count = 0;

void setup() {
  delay(10);
#if debug
  delay(1000);
  Serial.begin(115200);
  Serial.println("setup");
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  digitalWrite(0, HIGH);
  digitalWrite(1, LOW);
#endif
  pinMode(6, OUTPUT);
  digitalWrite(7, LOW);
  pinMode(i2c0_int, output);
  digitalwrite(i2c0_int, high);
  // wire1.setSDA(i2c1_sda);
  //wire1.setSCL(i2c1_scl);
  //wire1.begin(keyboard_iic_address);
  //wire1.onRequest(key_scan);

  for (int i = 0; i < map_x; i++) {
    pinMode(keyboard_matrix_x[i], input_pulldown);
  }

  for (int i = 0; i < map_y; i++) {
    pinMode(keyboard_matrix_y[i], OUTPUT_4MA);
  }

  digitalwrite(i2c0_int, low);


#if debug
  Serial.println("setup finish");
  digitalWrite(0, LOW);
#endif
}

void loop() {
#if debug
  digitalWrite(1, !digitalRead(1));
#endif
  bool keyboard_matrix[map_x][map_y];
  for (int i = 0; i < map_x; i++) {
    for (int j = 0; j < map_y; j++) {
      keyboard_matrix[i][j] = 0;
    }
  }
  for (int j = 0; j < map_x; j++) {
    for (int k = 0; k < map_y; k++) {
      for (int l = 0; l < map_y; l++) {
        pinMode(keyboard_matrix_y[l], INPUT);
      }
      pinMode(keyboard_matrix_y[k], OUTPUT);
      digitalwrite(keyboard_matrix_y[k], high);
      asm("nop");
      if (digitalread(keyboard_matrix_x[j])) {

        keyboard_matrix[j][k] = 1;
#if debug
        Serial.print("keyboard_matrix : ");
        Serial.print(j);
        Serial.print("\t");
        Serial.print(k);
        Serial.print("\t");
        Serial.println();
        Serial.print("pin_map : ");
        Serial.print(keyboard_matrix_x[j]);
        Serial.print("\t");
        Serial.print(keyboard_matrix_y[k]);
        Serial.print("\t");
        Serial.println();
#endif
      } else {
        keyboard_matrix[j][k] = 0;
      }
      digitalwrite(keyboard_matrix_y[k], low);
    }
  }
  iic_report[0] = 128 * keyboard_matrix[8][12] + 64 * keyboard_matrix[6][12] + 32 * keyboard_matrix[5][12] + 16 * keyboard_matrix[4][12] + 8 * keyboard_matrix[3][12] + 4 * keyboard_matrix[2][12] + 2 * keyboard_matrix[1][12] + keyboard_matrix[0][12];
//                         上，                             右上，             右，                       右下，                 下，                            左下，                   左，             左上
#if debug
  if (keyboard_matrix[8][12]) {
    Serial.println("上");
  }
  if (keyboard_matrix[3][12]) {
    Serial.println("下");
  }
  if (keyboard_matrix[1][12]) {
    Serial.println("左");
  }
  if (keyboard_matrix[5][12]) {
    Serial.println("右");
  }
  if (keyboard_matrix[0][12]) {
    Serial.println("左上");
  }
  if (keyboard_matrix[2][12]) {
    Serial.println("左下");
  }
  if (keyboard_matrix[4][12]) {
    Serial.println("右下");
  }
  if (keyboard_matrix[6][12]) {
    Serial.println("右上");
  }
#endif

  if (keyboard_matrix[7][12]) {
    if (keyboard_matrix[6][6]) {
      iic_report[1] = 66;  //特殊功能使能键
#if debug
      Serial.println("特殊功能启动");
#endif
    } else {
      iic_report[1] = 33;
    }
  } else {
    iic_report[1] = 33;
  }
  keyboard_matrix[4][6] = keyboard_matrix[7][12];

  for (int i = 0; i < keyboard_iic_length - 2; i++) {
    iic_report[i + 2] = 64 * keyboard_matrix[0][i] + 32 * keyboard_matrix[1][i] + 16 * keyboard_matrix[2][i] + 8 * keyboard_matrix[3][i] + 4 * keyboard_matrix[4][i] + 2 * keyboard_matrix[5][i] + keyboard_matrix[6][i];
  }
  iic_report[keyboard_iic_length - 1] = keyboard_iic_crc;

#if debug
  Serial.print("iic_report : ");
  for (int l = 0; l < keyboard_iic_length; l++) {
    Serial.print(iic_report[l]);
    Serial.print("/");
  }
  Serial.println("");
  bool key_status[7][keyboard_iic_length - 2];
  for (int a = 0; a < 7; a++) {
    for (int b = 0; b < (keyboard_iic_length - 2); b++) {
      key_status[a][b] = 0;
    }
  }
  for (int i = 0; i < keyboard_iic_length - 3; i++) {
    key_status[0][i] = iic_report[i + 2] & 0b01000000;
    key_status[1][i] = iic_report[i + 2] & 0b00100000;
    key_status[2][i] = iic_report[i + 2] & 0b00010000;
    key_status[3][i] = iic_report[i + 2] & 0b00001000;
    key_status[4][i] = iic_report[i + 2] & 0b00000100;
    key_status[5][i] = iic_report[i + 2] & 0b00000010;
    key_status[6][i] = iic_report[i + 2] & 0b00000001;
  }
  for (int a = 0; a < 7; a++) {
    for (int b = 0; b < (keyboard_iic_length - 2); b++) {
      if (key_status[a][b]) {
        Serial.print(a);
        Serial.print("\\");
        Serial.print(b);
      } else {
        Serial.print("   ");
      }
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println("---------------------------------------------------------------------------------------------------------------------------------------");
  for (int a = 0; a < 7; a++) {
    for (int b = 0; b < (keyboard_iic_length - 2); b++) {
      if (key_status[a][b]) {
        Serial.print("char : \" ");
        Serial.print(new_key_map_with_shift[a][b]);
        uint8_t c = new_key_map_with_shift[a][b];  //- '0';
        Serial.println(" \"");
        Serial.print("int : \" 0x");
        Serial.print(c, HEX);
        Serial.println(" \"");
        Serial.print("翻译 : ");
        Serial.print(key_massage_to_show[c]);
        for (int d = 0; d < 9; d++) {
          // Serial.print(key_massage_to_show[c][d]);
        }
        Serial.print(" \"");
      } else {
      }
    }
  }
  Serial.println();


#endif


#if debug
  Serial.println("loop finish");
  Serial.println("-------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
  delay(1000);
#endif
}


void key_scan() {
  for (int i = 0; i < keyboard_iic_length; i++) {
    wire1.write(iic_report[i]);
  }
}


void setup1() {
  wire1.setSDA(i2c1_sda);
  wire1.setSCL(i2c1_scl);
  wire1.begin(keyboard_iic_address);
  wire1.onRequest(key_scan);
}
void loop1() {
}