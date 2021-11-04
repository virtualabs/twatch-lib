#ifndef __INC_GPS_H
#define __INC_GPS_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"
#include "drivers/uart.h"
#include "hal/pmu.h"

#define TWATCH_GPS_WAKEUP GPIO_NUM_33

#define TWATCH_GPS_ERROR      -1
#define TWATCH_GPS_RX_FULL    -2

typedef struct t_gps_raw_degrees {
  uint16_t deg;
  uint32_t billionths;
  bool negative;
} gps_raw_degrees_t;

/* Exposed functions. */
esp_err_t twatch_gps_init(void);
esp_err_t twatch_gps_on(void);
esp_err_t twatch_gps_off(void);

void gps_get_lat_lng(gps_raw_degrees_t *p_lat, gps_raw_degrees_t *p_lng);
int gps_get_day(void);
int gps_get_month(void);
int gps_get_year(void);
int gps_get_secs(void);
int gps_get_mins(void);
int gps_get_hours(void);
float gps_get_speed(void);
int gps_get_satellites(void);
float gps_get_hdop(void);
float gps_get_alt(void);



#endif /* __INC_GPS_H */