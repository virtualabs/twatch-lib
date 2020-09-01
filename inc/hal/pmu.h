#ifndef __INC_TWATCH_PMU_H
#define __INC_TWATCH_PMU_H

#include "drivers/axp20x.h"

/* Global power management functions. */
esp_err_t twatch_pmu_init(void);
esp_err_t twatch_pmu_power(bool enable);

/* Peripheral power management functions. */
esp_err_t twatch_pmu_audio_power(bool enable);
esp_err_t twatch_pmu_screen_power(bool enable);

/* User button. */
bool twatch_pmu_is_userbtn_pressed(void);

#endif /* __INC_TWATCH_PMU_H */
