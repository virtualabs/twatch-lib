#ifndef __INC_DRIVER_PCF8563_H
#define __INC_DRIVER_PCF8563_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "drivers/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdbool.h>


#define PCF8563_SLAVE_ADDRESS   (0x51) //7-bit I2C Address

/**
 * Register map (addresses)
 **/

#define PCF8563_STAT1_REG       (0x00)
#define PCF8563_STAT2_REG       (0x01)
#define PCF8563_SEC_REG         (0x02)
#define PCF8563_MIN_REG         (0x03)
#define PCF8563_HR_REG          (0x04)
#define PCF8563_DAY_REG         (0x05)
#define PCF8563_WEEKDAY_REG     (0x06)
#define PCF8563_MONTH_REG       (0x07)
#define PCF8563_YEAR_REG        (0x08)
#define PCF8563_ALRM_MIN_REG    (0x09)
#define PCF8563_SQW_REG         (0x0D)
#define PCF8563_TIMER1_REG      (0x0E)
#define PCF8563_TIMER2_REG      (0x0F)

#define PCF8563_VOL_LOW_MASK    (0x80)
#define PCF8563_minuteS_MASK    (0x7F)
#define PCF8563_HOUR_MASK       (0x3F)
#define PCF8563_WEEKDAY_MASK    (0x07)
#define PCF8563_CENTURY_MASK    (0x80)
#define PCF8563_DAY_MASK        (0x3F)
#define PCF8563_MONTH_MASK      (0x1F)
#define PCF8563_TIMER_CTL_MASK  (0x03)


#define PCF8563_ALARM_AF        (0x08)
#define PCF8563_TIMER_TF        (0x04)
#define PCF8563_ALARM_AIE       (0x02)
#define PCF8563_TIMER_TIE       (0x01)
#define PCF8563_TIMER_TE        (0x80)
#define PCF8563_TIMER_TD10      (0x03)

#define PCF8563_NO_ALARM        (0xFF)
#define PCF8563_ALARM_ENABLE    (0x80)
#define PCF8563_CLK_ENABLE      (0x80)

/**
 * Enums (constants)
 **/ 

enum {
    PCF8563_CLK_32_768KHZ,
    PCF8563_CLK_1024KHZ,
    PCF8563_CLK_32HZ,
    PCF8563_CLK_1HZ,
    PCF8563_CLK_MAX
};

enum {
    PCF_TIMEFORMAT_HM,
    PCF_TIMEFORMAT_HMS,
    PCF_TIMEFORMAT_YYYY_MM_DD,
    PCF_TIMEFORMAT_MM_DD_YYYY,
    PCF_TIMEFORMAT_DD_MM_YYYY,
    PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S,
};


/**
 * Structures
 **/

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} pcf8563_datetime_t;


typedef struct {
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t weekday;
} pcf8563_alarm_t;


/**
 * Exported functions
 **/

esp_err_t pcf8563_init(void);
esp_err_t pcf8563_probe(void);

/* Set/get date and time. */
esp_err_t pcf8563_set_date_time(pcf8563_datetime_t *p_datetime_t);
esp_err_t pcf8563_get_date_time(pcf8563_datetime_t *p_datetime_t);

/* Set/get/enable/disable alarm. */
esp_err_t pcf8563_set_alarm(pcf8563_alarm_t *p_alarm);
esp_err_t pcf8563_get_alarm(pcf8563_alarm_t *p_alarm);
void pcf8563_enable_alarm(bool enable);



#endif /* __INC_DRIVER_PCF8563_H */