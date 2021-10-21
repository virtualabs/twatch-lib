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


esp_err_t twatch_gps_init(void);
esp_err_t twatch_gps_on(void);
esp_err_t twatch_gps_off(void);

#endif /* __INC_GPS_H */