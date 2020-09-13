#ifndef __INC_DRIVERS_I2C_H
#define __INC_DRIVERS_I2C_H

#include "driver/gpio.h"
#include "driver/i2c.h"

#define I2C_PRI_SDA_PIN (GPIO_NUM_21)
#define I2C_PRI_SCL_PIN (GPIO_NUM_22)
#define I2C_SEC_SDA_PIN (23)
#define I2C_SEC_SCL_PIN (32)

typedef enum {
  I2C_PRI,
  I2C_SEC
} i2c_bus_t;

void twatch_i2c_init(void);
esp_err_t twatch_i2c_master_cmd_begin(i2c_bus_t bus, i2c_cmd_handle_t cmd, TickType_t ticks_to_wait);

esp_err_t twatch_i2c_readBytes(
  i2c_bus_t bus,
  uint8_t addr,
  uint8_t reg,
  uint8_t *data,
  uint16_t len,
  TickType_t ticks_to_wait
);

esp_err_t twatch_i2c_writeBytes(
  i2c_bus_t bus,
  uint8_t addr,
  uint8_t reg,
  uint8_t *data,
  uint16_t len,
  TickType_t ticks_to_wait
);


#endif /* __INC_DRIVERS_I2C_H */
