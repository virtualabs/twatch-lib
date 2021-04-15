#include "drivers/pcf8563.h"

/**
 * Helpers
 **/

uint8_t bcd_to_dec(uint8_t val)
{
    return ( ((val / 16) * 10) + (val % 16) );
}


uint8_t dec_to_bcd(uint8_t val)
{
    return ( ((val / 10) * 16) + (val % 10) );
}


int get_day_of_week(uint32_t day, uint32_t month, uint32_t year)
{
  uint32_t val;

  if (month < 3)
  {
      month = 12u + month;
      year--;
  }

  val = (day + (((month + 1u) * 26u) / 10u) + year + (year / 4u) + (6u * (year / 100u)) + (year / 400u)) % 7u;
  if (0u == val)
  {
      val = 7;
  }

  return (val - 1);
}

int constrain(int x, int a, int b)
{
  if (x<a)
    return a;
  if (x>b)
    return b;
  return x;
}

/**
 * _pcf8563_interrupt_handler()
 **/

void IRAM_ATTR _pcf8563_interrupt_handler(void *parameter)
{
}

/**
 * pcf8563_init()
 * 
 * @brief Initialize the PCF8563.
 * @return ESP_OK on success, ESP_FAIL on error.
 ***/

esp_err_t pcf8563_init(void)
{
  gpio_config_t irq_conf;

  /* Initialize PCF8563 IRQ pin as input pin. */
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;
  irq_conf.pin_bit_mask = (1ULL << 37);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_down_en = 0;
  irq_conf.pull_up_en = 1;
  gpio_config(&irq_conf);

  /* Install our user button interrupt handler. */
  if (gpio_install_isr_service(0) != ESP_OK)
    printf("[isr2] Error while installing service\r\n");
  gpio_isr_handler_add(GPIO_NUM_37, _pcf8563_interrupt_handler, NULL);

  /* Enable Twatch i2c bus. */
  twatch_i2c_init();

  /* Success. */
  return ESP_OK;
}

/**
 * PCF8563 I2C read byte interface.
 **/

void pcf8563_read_bytes(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
  twatch_i2c_readBytes(
    I2C_PRI,
    PCF8563_SLAVE_ADDRESS,
    reg,
    data,
    nbytes,
    portMAX_DELAY
  );
}


/**
 * PCF8563 I2C write byte interface.
 **/

void pcf8563_write_bytes(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
  twatch_i2c_writeBytes(
    I2C_PRI,
    PCF8563_SLAVE_ADDRESS,
    reg,
    data,
    nbytes,
    portMAX_DELAY
  );
}

esp_err_t pcf8563_probe(void)
{
  uint8_t status_reg1=0;
  pcf8563_read_bytes(PCF8563_STAT1_REG, 1, &status_reg1);
  printf("[pcf8563:probe] STATUS1=%02x\r\n", status_reg1);

  /* Success. */
  return (status_reg1==0x08)?ESP_OK:ESP_FAIL;
}


esp_err_t pcf8563_set_date_time(pcf8563_datetime_t *p_datetime)
{
  uint8_t data[16];

  data[0] = dec_to_bcd(p_datetime->second) & (~PCF8563_VOL_LOW_MASK);
  data[1] = dec_to_bcd(p_datetime->minute);
  data[2] = dec_to_bcd(p_datetime->hour);
  data[3] = dec_to_bcd(p_datetime->day);
  data[4] = get_day_of_week(p_datetime->day, p_datetime->month, p_datetime->year);
  data[5] = dec_to_bcd(p_datetime->month);
  data[6] = dec_to_bcd(p_datetime->year % 100);

  if ( p_datetime->year < 2000) {
      data[4] |= PCF8563_CENTURY_MASK;
  } else {
      data[4] &= (~PCF8563_CENTURY_MASK);
  }

  pcf8563_write_bytes(PCF8563_SEC_REG, 7, data);

  /* Success. */
  return ESP_OK;
}


esp_err_t pcf8563_get_date_time(pcf8563_datetime_t *p_datetime)
{
  uint8_t data[16];
  uint16_t year;
  uint8_t century = 0;
  bool b_voltage_low;

  pcf8563_read_bytes(PCF8563_SEC_REG, 7, data);

  b_voltage_low = (data[0] & PCF8563_VOL_LOW_MASK);
  data[0] = bcd_to_dec(data[0] & (~PCF8563_VOL_LOW_MASK));
  data[1] = bcd_to_dec(data[1] & PCF8563_minuteS_MASK);
  data[2] = bcd_to_dec(data[2] & PCF8563_HOUR_MASK);
  data[3] = bcd_to_dec(data[3] & PCF8563_DAY_MASK);
  data[4] = bcd_to_dec(data[4] & PCF8563_WEEKDAY_MASK);
  century = data[5] & PCF8563_CENTURY_MASK;
  data[5] = bcd_to_dec(data[5] & PCF8563_MONTH_MASK);
  p_datetime->year = bcd_to_dec(data[6]);
  p_datetime->year = century ? 1900 + p_datetime->year : 2000 + p_datetime->year;
  p_datetime->month = data[5];
  p_datetime->day = data[3];
  p_datetime->hour = data[2];
  p_datetime->minute = data[1];
  p_datetime->second = data[0];

  /* Success. */
  return ESP_OK;
}


/**
 * Alarm
 **/

esp_err_t pcf8563_set_alarm(pcf8563_alarm_t *p_alarm)
{
  uint8_t data[4];

  if (p_alarm->minute != PCF8563_NO_ALARM) {
      data[0] = dec_to_bcd(constrain(p_alarm->minute, 0, 59));
      data[0] &= ~PCF8563_ALARM_ENABLE;
  } else {
      data[0] = PCF8563_ALARM_ENABLE;
  }

  if (p_alarm->hour != PCF8563_NO_ALARM) {
      data[1] = dec_to_bcd(constrain(p_alarm->hour, 0, 23));
      data[1] &= ~PCF8563_ALARM_ENABLE;
  } else {
      data[1] = PCF8563_ALARM_ENABLE;
  }
  if (p_alarm->day != PCF8563_NO_ALARM) {
      data[2] = dec_to_bcd(constrain(p_alarm->day, 1, 31));
      data[2] &= ~PCF8563_ALARM_ENABLE;
  } else {
      data[2] = PCF8563_ALARM_ENABLE;
  }
  if (p_alarm->weekday != PCF8563_NO_ALARM) {
      data[3] = dec_to_bcd(constrain(p_alarm->weekday, 0, 6));
      data[3] &= ~PCF8563_ALARM_ENABLE;
  } else {
      data[3] = PCF8563_ALARM_ENABLE;
  }

  pcf8563_write_bytes(PCF8563_ALRM_MIN_REG, 4, data);

  /* Success. */
  return ESP_OK;
}

esp_err_t pcf8563_get_alarm(pcf8563_alarm_t *p_alarm)
{
  uint8_t data[4];

  /* Read and convert data. */
  pcf8563_read_bytes(PCF8563_ALRM_MIN_REG, 4, data);

  data[0] = bcd_to_dec(data[0] & PCF8563_minuteS_MASK);
  data[1] = bcd_to_dec(data[1] & PCF8563_HOUR_MASK);
  data[2] = bcd_to_dec(data[2] & PCF8563_DAY_MASK);
  data[3] = bcd_to_dec(data[3] & PCF8563_WEEKDAY_MASK);

  /* Fill alarm info. */
  p_alarm->minute = data[0];
  p_alarm->hour = data[1];
  p_alarm->day = data[2];
  p_alarm->weekday = data[3];

  /* Success. */
  return ESP_OK;
}


void pcf8563_enable_alarm(bool enable)
{
  uint8_t data;

  pcf8563_read_bytes(PCF8563_STAT2_REG, 1, &data);
  data &= ~PCF8563_ALARM_AF;
  data |= (PCF8563_TIMER_TF | PCF8563_ALARM_AIE);
  pcf8563_write_bytes(PCF8563_STAT2_REG, 1, &data);
}

