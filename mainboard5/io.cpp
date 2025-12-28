#include "io.h"

uint32_t pwm_pins[] = {fan_pwm, screen_backlight_pwm};
RP2040_PWM *pwm_instance[2];
static const BatteryLUT battery_lut[] =
{
  {42000, 100},
  {41500, 95},
  {41000, 90},
  {40500, 85},
  {40000, 80},
  {39500, 75},
  {39000, 70},
  {38500, 65},
  {38000, 60},
  {37500, 55},
  {37000, 45},
  {36500, 35},
  {36000, 25},
  {35500, 15},
  {35000, 8},
  {34000, 3},
  {33000, 0}
};

bool IO_CONFIG::begin()
{
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
bool IO_CONFIG::serial_setup()
{
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

bool IO_CONFIG::iic_setup()
{
  Wire1.setSDA(iic1_sda);
  Wire1.setSCL(iic1_scl);
  Wire1.begin();
  Wire1.setClock(3400000);
#if debug
  Serial1.println("iic setup finish");
#endif
  return true;
}

bool IO_CONFIG::pin_setup()
{
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

bool IO_CONFIG::adc_setup()
{
  analogReadResolution(12);
  return true;
}

/*bool IO_CONFIG::battery_voltage_update()
  {
  if (battery_count >= battery_count_max)
  {
    battery_count = 0;
    battery_voltage = battery_long / battery_count_max;
    battery_voltage = (uint32_t)battery_voltage * 33000 * 11 / 4095;
    int battery_percent_ = map(battery_voltage, battery_0_percent, battery_100_percent, 0, 100);
    if (battery_percent_ > 100)
    {
      battery_percent = 100;
    }
    else if (battery_percent_ < 0)
    {
      battery_percent = 0;
    }
    else
    {
      battery_percent = battery_percent_;
    }
    battery_long = 0;
  }
  else
  {
    battery_long = battery_long + analogRead(battery);
    battery_count++;
  }

  #if debug
  Serial1.print("battery voltage:");
  Serial1.print(battery_voltage);
  Serial1.println();
  Serial1.print("battery percent:");
  Serial1.println(battery_percent);
  Serial1.println("battery read finish");
  #endif
  return true;
  }*/
bool IO_CONFIG::battery_voltage_update()
{
  uint32_t adc = analogRead(battery);

  // ADC → 电压（mV）
  uint32_t voltage_mv =
    adc * 33000UL * 11 / 4095;

  /* ---------- 电压 IIR 滤波 ---------- */
  if (battery_voltage == 0)
  {
    // 冷启动直接赋值
    battery_voltage = voltage_mv;
  }
  else
  {
    battery_voltage +=
      (int32_t)(voltage_mv - battery_voltage) /
      battery_count_max;
  }
  battery_percent = battery_voltage_to_percent(battery_voltage);

  /*
    int percent =
      (int32_t)(battery_voltage - battery_0_percent) * 100 /
      (battery_100_percent - battery_0_percent);

    if (percent > 100)
      percent = 100;
    else if (percent < 0)
      percent = 0;

    if (battery_percent == 0)
    {
      battery_percent = percent;
    }
    else
    {
      battery_percent +=
        (percent - battery_percent) / battery_count_max;
    }
  */
#if debug
  Serial1.print("battery voltage:");
  Serial1.println(battery_voltage);
  Serial1.print("battery percent:");
  Serial1.println(battery_percent);
  Serial1.println("battery read finish");
#endif

  return true;
}

uint8_t IO_CONFIG::battery_percent_read()
{
  battery_voltage_update();
#if debug
  Serial1.println("return battery percent");
#endif
  return battery_percent;
}
uint16_t IO_CONFIG::battery_voltage_read()
{
  // battery_voltage_update();
#if debug
  Serial1.println("return battery voltage");
#endif
  return battery_voltage;
}
uint8_t IO_CONFIG::battery_voltage_to_percent(uint16_t v_01mv)
{
  if (v_01mv >= battery_lut[0].v_01mv)
    return 100;

  for (uint8_t i = 1; i < sizeof(battery_lut)/sizeof(battery_lut[0]); i++)
  {
    if (v_01mv >= battery_lut[i].v_01mv)
    {
      uint32_t v1 = battery_lut[i - 1].v_01mv;
      uint32_t v2 = battery_lut[i].v_01mv;
      uint8_t  p1 = battery_lut[i - 1].percent;
      uint8_t  p2 = battery_lut[i].percent;

      return p2 +
             (uint32_t)(v_01mv - v2) * (p1 - p2) /
             (v1 - v2);
    }
  }
  return 0;
}

/*uint16_t IO_CONFIG::pi_main_power_read()
  {

  if (pi_main_power_voltage_count < pi_main_power_voltage_count_max)
  {
    pi_main_power_voltage_long = pi_main_power_voltage_long + analogRead(main_power);
    pi_main_power_voltage_count++;
  }
  else
  {
    pi_main_power_voltage_long = (pi_main_power_voltage_long / pi_main_power_voltage_count_max);
    pi_main_power_voltage_long_long = pi_main_power_voltage_long_long + pi_main_power_voltage_long * 33000 * 2 / 4095;
    pi_main_power_voltage_long_long_count++;
    pi_main_power_voltage_long = 0;
    pi_main_power_voltage_count = 0;
  }
  if (pi_main_power_voltage_long_long_count >=pi_main_power_voltage_long_count_max)
  {
    pi_main_power_voltage = pi_main_power_voltage_long_long / pi_main_power_voltage_long_count_max;
    pi_main_power_voltage_long_long = 0;
    pi_main_power_voltage_long_long_count = 0;
  }
  // pi_main_power_voltage = map(analogRead(main_power), 0, 4095, 0, 33000 * 2);
  #if debug
  Serial1.print("pi main power voltage:");
  Serial1.println(pi_main_power_voltage);
  #endif
  return pi_main_power_voltage;
  }*/
uint16_t IO_CONFIG::pi_main_power_read()
{
  // 1. 读取 ADC
  uint16_t adc = analogRead(main_power);

  // 2. ADC → 电压（单位 mV，保留你原来的比例）
  uint32_t voltage_mv =
    (uint32_t)adc * 33000 * 2 / 4095;

  // 3. 一阶 IIR 滤波（使用你已有的 long 变量）
  if (pi_main_power_voltage_long == 0)
  {
    // 首次初始化，避免冷启动拉低
    pi_main_power_voltage_long = voltage_mv;
  }
  else
  {
    // IIR: y = y + (x - y) / N
    pi_main_power_voltage_long +=
      (int32_t)(voltage_mv - pi_main_power_voltage_long) /
      pi_main_power_voltage_count_max;
  }

  // 4. 使用你原来的计数器，控制“何时更新对外电压”
  pi_main_power_voltage_count++;

  if (pi_main_power_voltage_count >= pi_main_power_voltage_count_max)
  {
    pi_main_power_voltage = (uint16_t)pi_main_power_voltage_long;
    pi_main_power_voltage_count = 0;
  }

#if debug
  Serial1.print("pi main power voltage:");
  Serial1.println(pi_main_power_voltage);
#endif

  return pi_main_power_voltage;
}


/*uint16_t IO_CONFIG::backlight_voltage_read()
  {
  backlight_voltage = map(analogRead(backlight), 0, 4095, 0, 33000 * 11);
  #if debug
  Serial1.print("backlight voltage:");
  Serial1.println(backlight_voltage);
  #endif
  return backlight_voltage;
  }

  uint16_t IO_CONFIG::pi_3v3_voltage_read()
  {
  pi_3v3_voltage = map(analogRead(pi_3v3), 0, 4095, 0, 33000 * 2);
  #if debug
  Serial1.print("pi 3v3 voltage:");
  Serial1.println(pi_3v3_voltage);
  #endif
  return pi_3v3_voltage;
  }*/
uint16_t IO_CONFIG::backlight_voltage_read()
{
  uint32_t voltage_mv =
    (uint32_t)analogRead(backlight) * 33000 * 11 / 4095;

  // IIR 滤波
  if (backlight_voltage == 0)
  {
    // 冷启动直接赋值，避免慢爬
    backlight_voltage = voltage_mv;
  }
  else
  {
    backlight_voltage +=
      (int32_t)(voltage_mv - backlight_voltage) /
      backlight_voltage_count_max;
  }

#if debug
  Serial1.print("backlight voltage:");
  Serial1.println(backlight_voltage);
#endif

  return backlight_voltage;
}
uint16_t IO_CONFIG::pi_3v3_voltage_read()
{
  uint32_t voltage_mv =
    (uint32_t)analogRead(pi_3v3) * 33000 * 2 / 4095;

  // IIR 滤波
  if (pi_3v3_voltage == 0)
  {
    // 冷启动初始化
    pi_3v3_voltage = voltage_mv;
  }
  else
  {
    pi_3v3_voltage +=
      (int32_t)(voltage_mv - pi_3v3_voltage) /
      pi_3v3_voltage_count_max;
  }

#if debug
  Serial1.print("pi 3v3 voltage:");
  Serial1.println(pi_3v3_voltage);
#endif

  return pi_3v3_voltage;
}


bool IO_CONFIG::fan_setup(uint8_t fan_duty_cycle)
{
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
bool IO_CONFIG::fan_update(uint8_t fan_duty_cycle)
{
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
bool IO_CONFIG::backlight_setup(uint8_t backlight_duty_cycle)
{
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
bool IO_CONFIG::backlight_update(uint8_t backlight_duty_cycle)
{
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
uint8_t IO_CONFIG::fan_report()
{
#if debug
  Serial1.print("report fan speed:");
  Serial1.println(fan_speed_value);
#endif
  return fan_speed_value;
}
uint8_t IO_CONFIG::backlight_report()
{
#if debug
  Serial1.print("report backlight value:");
  Serial1.println(backlight_value);
#endif
  return backlight_value;
}
bool IO_CONFIG::read_hdmi_status()
{
  if (digitalRead(hdmi_active))
  {
    if (screen_enabled != true)
    {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100 - backlight_value);
      screen_enabled = true;
    }
  }
  else
  {
    if (screen_enabled != false)
    {
      pwm_instance[1]->setPWM(screen_backlight_pwm, backlight_pwm_freq, 100);
      screen_enabled = false;
    }
  }
  return screen_enabled;
}
bool IO_CONFIG::update_charge_status()
{
  charge_or_not = !digitalRead(charge_status);
  charge_done_or_not = !digitalRead(charge_done_status);
  return true;
}

void IO_CONFIG::led1_flip()
{
  digitalWrite(led1, !digitalRead(led1));
}
void IO_CONFIG::led2_flip()
{
  digitalWrite(led2, !digitalRead(led2));
}

void IO_CONFIG::led1_high()
{
  digitalWrite(led1, HIGH);
}
void IO_CONFIG::led1_low()
{
  digitalWrite(led1, LOW);
}
void IO_CONFIG::led2_high()
{
  digitalWrite(led2, HIGH);
}
void IO_CONFIG::led2_low()
{
  digitalWrite(led2, LOW);
}
uint16_t IO_CONFIG::set_battery_0_percent_value(uint16_t value)
{
  battery_0_percent = value;
  return value;
}
uint16_t IO_CONFIG::set_battery_100_percent_value(uint16_t value)
{
  battery_100_percent = value;
  return value;
}
