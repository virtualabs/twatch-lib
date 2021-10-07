#include <string.h>
#include "esp_log.h"
#include "drivers/i2c.h"
#include "drivers/bma423/bma.h"

#define TAG   "[driver::bma423]"


uint16_t _irqStatus;
bool _init;
Bma _dev;

/* I2C primitives for bma4_dev structure. */
uint16_t _bma_read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t _bma_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);


/**
 * @brief Interface function to read bytes from BMA423.
 * @param addr: device address
 * @param reg: register to read data from
 * @param data: pointer to destination buffer
 * @param len: number of bytes to read
 * @retval 0 if successful, 0x2000 otherwise
 **/

uint16_t _bma_read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
  if (twatch_i2c_readBytes(I2C_PRI, addr, reg, data, len, 1000/portTICK_RATE_MS) == ESP_OK)
    return 0;
  else
    return (1<<13);
}


/**
 * @brief Interface function to write bytes to BMA423.
 * @param addr: device address
 * @param reg: register to read data from
 * @param data: pointer to destination buffer
 * @param len: number of bytes to read
 * @retval 0 if successful, 0x2000 otherwise
 **/

uint16_t _bma_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
  if (twatch_i2c_writeBytes(I2C_PRI, addr, reg, data, len, 1000/portTICK_RATE_MS) == ESP_OK)
    return 0;
  else
    return (1<<13);
}


/**
 * @brief Interface delay function
 * @param ms: milliseconds to wait
 **/

void _bma_delay(uint32_t ms)
{
  vTaskDelay(ms/portTICK_RATE_MS);
}


/**
 * @brief Initializes T-Watch's BMA423.
 **/

esp_err_t bma_init(void)
{
  /* Initialize device structure. */
  _dev.dev_addr        = BMA4_I2C_ADDR_SECONDARY;
  _dev.interface       = BMA4_I2C_INTERFACE;
  _dev.bus_read        = _bma_read;
  _dev.bus_write       = _bma_write;
  _dev.delay           = _bma_delay;
  _dev.read_write_len  = 8;
  _dev.resolution      = 12;
  _dev.feature_len     = BMA423_FEATURE_SIZE;

  bma_reset();

  _bma_delay(20);

  /* Initialize device. */
  if (bma423_init(&_dev) != BMA4_OK) {
      ESP_LOGI(TAG, "bma423_init FAIL");
      return ESP_FAIL;
  }

  struct bma4_int_pin_config config ;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl = BMA4_ACTIVE_HIGH;
  config.od = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en = BMA4_INPUT_DISABLE;
  if (BMA4_OK == bma4_set_int_pin_config(&config, BMA4_INTR1_MAP, &_dev))
    return ESP_OK;

  /* Write device feature config file. */
  if (bma423_write_config_file(&_dev) != BMA4_OK)
  {
    ESP_LOGI(TAG, "bma423_init FAIL: cannot write config file");
  }

  /* Failure. */
  return ESP_FAIL;
}


/**
 * @brief resets BMA423.
 **/

void bma_reset()
{
    uint8_t reg = 0xB6;
    _bma_write(BMA4_I2C_ADDR_SECONDARY, 0x7E, &reg, 1);
}


/**
 * @brief Get accelerometer data
 * @param acc: pointer to a destination Accel structure
 * @retval true on success, false otherwise
 **/

bool bma_getAccel(Accel *acc)
{
    memset(acc, 0, sizeof(Accel));
    if (bma4_read_accel_xyz(acc, &_dev) != BMA4_OK) {
        return false;
    }
    return true;
}


/**
 * @brief Get orientation
 * @retval value specifying the current orientation.
 **/

uint8_t bma_direction()
{
    Accel acc;
    if (bma4_read_accel_xyz(&acc, &_dev) != BMA4_OK) {
        return 0;
    }
    uint16_t absX = abs(acc.x);
    uint16_t absY = abs(acc.y);
    uint16_t absZ = abs(acc.z);

    if ((absZ > absX) && (absZ > absY)) {
        if (acc.z > 0) {
            return  DIRECTION_DISP_DOWN;
        } else {
            return DIRECTION_DISP_UP;
        }
    } else if ((absY > absX) && (absY > absZ)) {
        if (acc.y > 0) {
            return DIRECTION_BOTTOM_EDGE;
        } else {
            return  DIRECTION_TOP_EDGE;
        }
    } else {
        if (acc.x < 0) {
            return  DIRECTION_RIGHT_EDGE;
        } else {
            return DIRECTION_LEFT_EDGE;
        }
    }
}


/**
 * @brief Get the actual temperature
 * @retval temperature
 **/

float bma_temperature()
{
    int32_t data = 0;
    bma4_get_temperature(&data, BMA4_DEG, &_dev);
    float res = (float)data / (float)BMA4_SCALE_TEMP;
    /* 0x80 - temp read from the register and 23 is the ambient temp added.
     * If the temp read from register is 0x80, it means no valid
     * information is available */
    if (((data - 23) / BMA4_SCALE_TEMP) == 0x80) {
        res = 0;
    }
    return res;
}


/**
 * @brief Disables accelerometer.
 * @retval true on success, false otherwise.
 **/
 
bool bma_disableAccel()
{
    return bma_enableAccel(false);
}

bool bma_enableAccel(bool en)
{
    return (BMA4_OK == bma4_set_accel_enable(en ? BMA4_ENABLE : BMA4_DISABLE, &_dev));
}

bool bma_accelConfig(Acfg *cfg)
{
    return (BMA4_OK == bma4_set_accel_config(cfg, &_dev));
}

bool bma_disableIrq(uint16_t int_map)
{
    return (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, int_map, BMA4_DISABLE, &_dev));
}

bool bma_enableIrq(uint16_t int_map)
{
    return (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, int_map, BMA4_ENABLE, &_dev));
}

bool bma_enableFeature(uint8_t feature, uint8_t enable)
{
    if ((feature & BMA423_STEP_CNTR) == BMA423_STEP_CNTR) {
        bma423_step_detector_enable(enable ? BMA4_ENABLE : BMA4_DISABLE, &_dev);
    }

    return (BMA4_OK == bma423_feature_enable(feature, enable, &_dev));
}

bool bma_resetStepCounter()
{
    return  BMA4_OK == bma423_reset_step_counter(&_dev) ;
}

// Reserved for the SimpleWatch example
// Reserved for the SimpleWatch example
// Reserved for the SimpleWatch example
void bma_attachInterrupt()
{
    uint16_t rslt = BMA4_OK;
    if (bma4_set_accel_enable(BMA4_ENABLE, &_dev)) {
        return;
    }
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    if (bma4_set_accel_config(&cfg, &_dev)) {
        ESP_LOGE(TAG, "[bma4] set accel config fail");
        return;
    }
    // // rslt |= bma423_reset_step_counter(&_dev);
    rslt |= bma423_step_detector_enable(BMA4_ENABLE, &_dev);
    rslt |= bma423_feature_enable(BMA423_STEP_CNTR, BMA4_ENABLE, &_dev);
    rslt |= bma423_feature_enable(BMA423_WAKEUP, BMA4_ENABLE, &_dev);
    rslt |= bma423_feature_enable(BMA423_TILT, BMA4_ENABLE, &_dev);
    rslt |= bma423_step_counter_set_watermark(100, &_dev);

    // rslt |= bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT | BMA423_WAKEUP_INT, BMA4_ENABLE, &_dev);
    rslt |= bma423_map_interrupt(BMA4_INTR1_MAP,  BMA423_STEP_CNTR_INT, BMA4_ENABLE, &_dev);
    rslt |= bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_TILT_INT, BMA4_ENABLE, &_dev);

    bma423_anymotion_enable_axis(BMA423_ALL_AXIS_EN, &_dev);
}

bool bma_set_remap_axes(struct bma423_axes_remap *remap_data)
{
    return (BMA4_OK == bma423_set_remap_axes(remap_data, &_dev));
}

bool bma_readInterrupt()
{
    return bma423_read_int_status(&_irqStatus, &_dev) == BMA4_OK;
}

uint8_t bma_getIrqStatus()
{
    return _irqStatus;
}

uint32_t bma_getCounter()
{
    uint32_t stepCount;
    if (bma423_step_counter_output(&stepCount, &_dev) == BMA4_OK) {
        return stepCount;
    }
    return 0;
}

bool bma_isStepCounter()
{
    return (bool)(BMA423_STEP_CNTR_INT & _irqStatus);
}

bool bma_isDoubleClick()
{
    return (bool)(BMA423_WAKEUP_INT & _irqStatus);
}


bool bma_isTilt()
{
    return (bool)(BMA423_TILT_INT & _irqStatus);
}


bool bma_isActivity()
{
    return (bool)(BMA423_ACTIVITY_INT & _irqStatus);
}

bool bma_isAnyNoMotion()
{
    return (bool)(BMA423_ANY_NO_MOTION_INT & _irqStatus);
}

const char *bma_getActivity()
{
    uint8_t activity;
    bma423_activity_output(&activity, &_dev);
    if (activity & BMA423_USER_STATIONARY) {
        return "BMA423_USER_STATIONARY";
    } else if (activity & BMA423_USER_WALKING) {
        return "BMA423_USER_WALKING";
    } else if (activity & BMA423_USER_RUNNING) {
        return "BMA423_USER_RUNNING";
    } else if (activity & BMA423_STATE_INVALID) {
        return "BMA423_STATE_INVALID";
    }
    return "None";
}

bool bma_enableStepCountInterrupt(bool en)
{
    return  (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP,  BMA423_STEP_CNTR_INT, en, &_dev));
}

bool bma_enableTiltInterrupt(bool en)
{
    return  (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_TILT_INT, en, &_dev));
}

bool bma_enableWakeupInterrupt(bool en)
{
    return  (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_WAKEUP_INT, en, &_dev));
}

bool bma_enableAnyNoMotionInterrupt(bool en)
{
    return  (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_ANY_NO_MOTION_INT, en, &_dev));
}

bool bma_enableActivityInterrupt(bool en)
{
    return  (BMA4_OK == bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_ACTIVITY_INT, en, &_dev));
}
