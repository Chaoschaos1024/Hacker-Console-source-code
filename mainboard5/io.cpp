#include "io.h"

uint32_t pwm_pins[] = { fan_pwm, screen_backlight_pwm };
RP2040_PWM *pwm_instance[2];

bool IO_CONFIG::begin() {
  serial_setup();
#if debug
  Serial1.println("io setup begin");
#endif
  iic_setup();
  pin_setup();
  adc_setup();
#if debug
  Serial1.println("io setup finish");
#endif
  return true;
}
bool IO_CONFIG::serial_setup() {
#if debug
  Serial1.setRX(debug_rx);
  Serial1.setTX(debug_tx);
  Serial1.setFIFOSize(128);
#endif
  Serial1.begin(115200);
#if debug
  Serial1.println("debug serial setup finish");
#endif
  return true;
}

bool IO_CONFIG::iic_setup() {
  Wire1.setSDA(iic1_sda);
  Wire1.setSCL(iic1_scl);
  Wire1.begin();
  Wire1.setClock(3400000);
#if debug
  Serial1.println("iic setup finish");
#endif
  return true;
}

bool IO_CONFIG::pin_setup() {
  pinMode(keyboard_int, INPUT_PULLUP);
  pinMode(touch_int, INPUT);
  pinMode(touch_rst, OUTPUT);
  pinMode(hdmi_active, INPUT_PULLUP);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(charge_status, INPUT_PULLUP);
  pinMode(charge_done_status, INPUT_PULLUP);
#if debug
  Serial1.println("pin setup finish");
#endif
  return true;
}

bool IO_CONFIG::adc_setup() {
  analogReadResolution(12);
  return true;
}

bool IO_CONFIG::battery_voltage_update() {
  if (battery_count > battery_count_max) {
    battery_count = 0;
    battery_voltage = battery_long /  battery_count_max;
    battery_voltage = (uint32_t)battery_voltage * 33000 * 11 / 4095;
    int battery_percent_ = map(battery_voltage, battery_0_percent, battery_100_percent, 0, 100);
    if (battery_percent_ > 100) {
      battery_percent = 100;
    } else if (battery_percent_ < 0) {
      battery_percent = 0;
    } else {
      battery_percent = battery_percent_;
    }
    battery_long = 0;
  } else {
    battery_long = battery_long + analogRead(battery);
    battery_count++;
  }

#if debug
  Serial1.print("battery voltage:");
  Serial1.print(battery_voltage);
  Serial1.println();
  Serial1.print("battery percent:");
  Serial1.print(battery_percent);
  Serial1.println();
  Serial1.println("battery read finish");
#endif
  return true;
}

uint8_t IO_CONFIG::battery_percent_read() {
  battery_voltage_update();
#if debug
  Serial1.println("return battery percent");
#endif
  return battery_percent;
}
uint16_t IO_CONFIG::battery_voltage_read() {
  // battery_voltage_update();
#if debug
  Serial1.println("return battery voltage");
#endif
  return battery_voltage;
}

uint16_t IO_CONFIG::pi_main_power_read() {
  pi_main_power_voltage = map(analogRead(main_power), 0, 4095, 0, 33000 * 2);
#if debug
  Serial1.print("pi main power voltage:");
  Serial1.println(pi_main_power_voltage);
#endif
  return pi_main_power_voltage;
}

uint16_t IO_CONFIG::backlight_voltage_read() {
  backlight_voltage = map(analogRead(backlight), 0, 4095, 0, 33000 * 11);
#if debug
  Serial1.print("backlight voltage:");
  Serial1.println(backlight_voltage);
#endif
  return backlight_voltage;
}

uint16_t IO_CONFIG::pi_3v3_voltage_read() {
  pi_3v3_voltage = map(analogRead(pi_3v3), 0, 4095, 0, 33000 * 2);
#if debug
  Serial1.print("pi 3v3 voltage:");
  Serial1.println(pi_3v3_voltage);
#endif
  return pi_3v3_voltage;
}

bool IO_CONFIG::fan_setup(uint8_t fan_duty_cycle) {
  if (fan_duty_cycle > fan_speed_max)
  {
    fan_duty_cycle = fan_speed_max;
  }
  else if (fan_duty_cycle < fan_speed_min)
  {
    fan_duty_cycle = fan_speed_min;
  }
  pwm_instance[0] = new RP2040_PWM(fan_pwm, fan_pwm_freq, fan_duty_cycle);
  fan_speed_value = fan_duty_cycle;
#if debug
  Serial1.print("fan setup finsh.fan speed:");
  Serial1.println(fan_duty_cycle);
#endif
  return true;
}
bool IO_CONFIG::fan_update(uint8_t fan_duty_cycle) {
  if (fan_duty_cycle > fan_speed_max)
  {
    fan_duty_cycle = fan_speed_max;
  }
  else if (fan_duty_cycle < fan_speed_min)
  {
    fan_duty_cycle = fan_speed_min;
  }
  pwm_instance[0]->setPWM(fan_pwm, fan_pwm_freq, fan_duty_cycle);
  fan_speed_value = fan_duty_cycle;
  delay(delay_);
#if debug
  Serial1.print("fan update finish.fan speed:");
  Serial1.println(fan_duty_cycle);
#endif
  return true;
}
bool IO_CONFIG::backlight_setup(float backlight_duty_cycle) {
  if (backlight_duty_cycle > backlight_max)
  {
    backlight_duty_cycle = backlight_max;
  }
  else if (backlight_duty_cycle < backlight_min)
  {
    backlight_duty_cycle = backlight_min;
  }
  pwm_instance[1] = new RP2040_PWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_duty_cycle);
  backlight_value = backlight_duty_cycle;
#if debug
  Serial1.print("backlight setup finish.backlight value:");
  Serial1.println(backlight_duty_cycle);
#endif
  return true;
}
bool IO_CONFIG::backlight_update(float backlight_duty_cycle) {
  if (backlight_duty_cycle > backlight_max)
  {
    backlight_duty_cycle = backlight_max;
  }
  else if (backlight_duty_cycle < backlight_min)
  {
    backlight_duty_cycle = backlight_min;
  }
  pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_duty_cycle);
  backlight_value = backlight_duty_cycle;
  delay(delay_);
#if debug
  Serial1.print("backlight update finish.backlight value:");
  Serial1.println(backlight_duty_cycle);
#endif
  return true;
}
uint8_t IO_CONFIG::fan_report() {
#if debug
  Serial1.print("report fan speed:");
  Serial1.println(fan_speed_value);
#endif
  return fan_speed_value;
}
uint8_t IO_CONFIG::backlight_report() {
#if debug
  Serial1.print("report backlight value:");
  Serial1.println(backlight_value);
#endif
  return backlight_value;
}
bool IO_CONFIG::read_hdmi_status() {
  if (digitalRead(hdmi_active)) {
    if (screen_enabled != true) {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_value);
      screen_enabled = true;
    }
  } else {
    if (screen_enabled != false) {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100);
      screen_enabled = false;
    }
  }
  return screen_enabled;
}
bool IO_CONFIG::update_charge_status() {
  charge_or_not = !digitalRead(charge_status);
  charge_done_or_not = !digitalRead(charge_done_status);
  return true;
}

void IO_CONFIG::led1_flip() {
  digitalWrite(led1, !digitalRead(led1));
}
void IO_CONFIG::led2_flip() {
  digitalWrite(led2, !digitalRead(led2));
}

void IO_CONFIG::led1_high() {
  digitalWrite(led1, HIGH);
}
void IO_CONFIG::led1_low() {
  digitalWrite(led1, LOW);
}
void IO_CONFIG::led2_high() {
  digitalWrite(led2, HIGH);
}
void IO_CONFIG::led2_low() {
  digitalWrite(led2, LOW);
}
