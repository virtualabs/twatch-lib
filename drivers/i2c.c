#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "drivers/i2c.h"

static bool i2c_bus_init = false;
volatile SemaphoreHandle_t i2c_sems[2];

/**
 * twatch_i2c_init()
 *
 * Initialize both T-Watch I2C interfaces.
 **/

void twatch_i2c_init(void)
{
  esp_err_t res;
  i2c_config_t i2c_pri_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_PRI_SDA_PIN,
    .scl_io_num = I2C_PRI_SCL_PIN,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };
  i2c_config_t i2c_sec_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SEC_SDA_PIN,
    .scl_io_num = I2C_SEC_SCL_PIN,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };

  if (i2c_bus_init)
    return;
  else
    i2c_bus_init = true;

  /* Configure first I2C port in order to use BMA423 and AXP202. */
  i2c_sems[I2C_PRI] = xSemaphoreCreateRecursiveMutex();
  res = i2c_param_config(I2C_NUM_0, &i2c_pri_config);
  assert(res == ESP_OK);
  res = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
  assert(res == ESP_OK);

  /* Configure second I2C port for touch board. */
  i2c_sems[I2C_SEC] = xSemaphoreCreateRecursiveMutex();
  res = i2c_param_config(I2C_NUM_1, &i2c_sec_config);
  assert(res == ESP_OK);
  res = i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);
  assert(res == ESP_OK);
}


/**
 * twatch_i2c_master_cmd_begin()
 *
 * Sends an I2C command to a specific I2C bus, and takes care of resource
 * ownership.
 **/

esp_err_t twatch_i2c_master_cmd_begin(i2c_bus_t bus, i2c_cmd_handle_t cmd, TickType_t ticks_to_wait)
{
  esp_err_t result;

  /* Send command to the right I2C bus. */
  if ((bus == I2C_PRI) || (bus == I2C_SEC))
  {
    xSemaphoreTakeRecursive(i2c_sems[bus], portMAX_DELAY);
    result = i2c_master_cmd_begin((bus==I2C_PRI)?I2C_NUM_0:I2C_NUM_1, cmd, ticks_to_wait);
    xSemaphoreGiveRecursive(i2c_sems[bus]);

    /* Return result. */
    return result;
  }
  else
    return ESP_FAIL;
}
