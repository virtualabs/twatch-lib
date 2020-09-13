
#ifndef __INC_DRIVER_BMA423_H
#define __INC_DRIVER_BMA423_H

#include "drivers/bma423/bma423.h"
#include "drivers/i2c.h"

enum {
    DIRECTION_TOP_EDGE        = 0,
    DIRECTION_BOTTOM_EDGE     = 1,
    DIRECTION_LEFT_EDGE       = 2,
    DIRECTION_RIGHT_EDGE      = 3,
    DIRECTION_DISP_UP         = 4,
    DIRECTION_DISP_DOWN       = 5
} ;

typedef struct bma4_dev Bma;
typedef struct bma4_accel Accel;
typedef struct bma4_accel_config Acfg;

esp_err_t bma_init(void);

void bma_reset();
uint8_t bma_direction();
float bma_temperature();

bool bma_disableAccel();
bool bma_enableAccel(bool en);

bool bma_disableIrq(uint16_t int_map);
bool bma_enableIrq(uint16_t int_map);

void bma_attachInterrupt();

uint32_t bma_getCounter();
bool bma_isStepCounter();
bool bma_isDoubleClick();
bool bma_readInterrupt();
bool bma_isTilt();
bool bma_isActivity();
bool bma_isAnyNoMotion();
bool bma_getAccel(Accel *acc);
uint8_t bma_getIrqStatus();
const char *bma_getActivity();

bool bma_resetStepCounter();
bool bma_enableFeature(uint8_t feature, uint8_t enable );
bool bma_accelConfig(Acfg *cfg);

bool bma_set_remap_axes(struct bma423_axes_remap *remap_data);
bool bma_enableStepCountInterrupt(bool en);
bool bma_enableTiltInterrupt(bool en);
bool bma_enableWakeupInterrupt(bool en);
bool bma_enableAnyNoMotionInterrupt(bool en);
bool bma_enableActivityInterrupt(bool en);

#endif /* __INC_DRIVER_BMA423_H */
