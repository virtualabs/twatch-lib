#ifndef __INC_TWATCH_PMU_H
#define __INC_TWATCH_PMU_H

#include "drivers/axp20x.h"

/* Global power management functions. */
esp_err_t twatch_pmu_init(void);
esp_err_t twatch_pmu_power(bool enable);

/* Touchscreen. */
void twatch_pmu_reset_touchscreen(void);

/* Peripheral power management functions. */
esp_err_t twatch_pmu_audio_power(bool enable);
esp_err_t twatch_pmu_screen_power(bool enable);

/* DRV2605 management. */
esp_err_t twatch_pmu_vibration(bool enable);

/* User button. */
void twatch_pmu_read_irq(void);
bool twatch_pmu_is_userbtn_pressed(void);

/* Deep sleep */
void twatch_pmu_deepsleep(void);

/* Battery. */
int twatch_pmu_get_battery_level(void);
bool twatch_pmu_is_usb_plugged(bool b_query_irq);

#endif /* __INC_TWATCH_PMU_H */
