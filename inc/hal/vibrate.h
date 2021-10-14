#ifndef __INC_TWATCH_VIBRATE_H
#define __INC_TWATCH_VIBRATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include <inttypes.h>
#include <stdbool.h>
#include "drivers/drv2605.h"
#include "hal/pmu.h"

#define VIBRATE_ON  1
#define VIBRATE_OFF 0

typedef enum {
  VIBRATE_DURATION,
  VIBRATE_PATTERN
} vibrate_mode_t;

typedef struct {
  int duration;
  int level;
} vibrate_pattern_t;

typedef struct {
  vibrate_mode_t mode;
  int duration;
  vibrate_pattern_t *pattern;
  int pattern_length;
} vibrate_parameter_t;

esp_err_t twatch_vibrate_init(void);
esp_err_t twatch_vibrate_vibrate(int duration);
esp_err_t twatch_vibrate_pattern(vibrate_pattern_t *pattern, int length);

#endif /* __INC_TWATCH_VIBRATE_H */
