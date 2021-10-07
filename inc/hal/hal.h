#ifndef __INC_TWATCH_HAL_H
#define __INC_TWATCH_HAL_H

#include "drivers/i2c.h"

#include "hal/pmu.h"
#include "hal/screen.h"
#include "hal/rtc.h"
#include "hal/touch.h"
#include "hal/vibrate.h"
#include "hal/audio.h"

bool twatch_hal_init(void);

#endif /* __INC_TWATCH_HAL_H */