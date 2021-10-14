/*!
 * @file drv2605.c
 *
 * @mainpage Adafruit DRV2605L Haptic Driver
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit DRV2605L Haptic Driver ---->
 * http://www.adafruit.com/products/2305
 *
 * Check out the links above for our tutorials and wiring diagrams.
 *
 * This motor/haptic driver uses I2C to communicate.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * 
 * @section license License
 *
 * MIT license, all text above must be included in any redistribution.
 *
 */
/**************************************************************************/

#include "drivers/drv2605.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "drivers/i2c.h"

#define TAG "[driver::drv2605]"

/**
 * read_register8()
 * 
 * @brief Read 8-bit register through I2C
 * @param reg: register address
 * @param p_value: pointer to a uint8_t value
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t read_register8(uint8_t reg, uint8_t *p_value)
{
  return twatch_i2c_readBytes(
    I2C_PRI,
    DRV2605_ADDR,
    reg,
    p_value,
    1,
    1000/portTICK_RATE_MS
  );
}


/**
 * read_register8()
 * 
 * @brief Read 8-bit register through I2C
 * @param reg: register address
 * @param value: value to write into register
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t write_register8(uint8_t reg, uint8_t value)
{
  return twatch_i2c_writeBytes(
    I2C_PRI,
    DRV2605_ADDR,
    reg,
    &value,
    1,
    1000/portTICK_RATE_MS
  );
}


/**
 * drv2605_init()
 * 
 * @brief Initialize DRV2605L
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_init(void)
{
  uint8_t temp;

  /* Ensure communication is OK. */
  read_register8(DRV2605_REG_STATUS, &temp);

  if ((temp>>5) == 7)
  {
    ESP_LOGI(TAG, "DRV2605L is present");

    /* Check comm. */
    write_register8(DRV2605_REG_MODE, 0x42); // out of standby
    temp = 0;
    read_register8(DRV2605_REG_MODE, &temp); // out of standby
    if (temp != 0x42)
      ESP_LOGE(TAG, "comm error.");
    else
      ESP_LOGI(TAG, "comm ok.");

    /* Initialize registers. */
    write_register8(DRV2605_REG_MODE, 0x00); // out of standby
    write_register8(DRV2605_REG_RTPIN, 0x00); // no real-time-playback
    write_register8(DRV2605_REG_WAVESEQ1, 1); // strong click
    write_register8(DRV2605_REG_WAVESEQ2, 0); // end sequence
    write_register8(DRV2605_REG_OVERDRIVE, 0); // no overdrive
    write_register8(DRV2605_REG_SUSTAINPOS, 0);
    write_register8(DRV2605_REG_SUSTAINNEG, 0);
    write_register8(DRV2605_REG_BREAK, 0);
    write_register8(DRV2605_REG_AUDIOMAX, 0x64);

  // turn off N_ERM_LRA
  read_register8(DRV2605_REG_FEEDBACK, &temp);
  write_register8(DRV2605_REG_FEEDBACK, temp & 0x7F);

  // turn on ERM_OPEN_LOOP
  read_register8(DRV2605_REG_CONTROL3, &temp);
  write_register8(DRV2605_REG_CONTROL3, temp | 0x20);
    return ESP_OK;
  }
  else
  {
    ESP_LOGE(TAG, "cannot initialize DRV2605L !");
    return ESP_FAIL;
  }
}


/**
 * drv2605_set_waveform()
 * 
 * @brief Set a waveform into DRV2605L
 * @param slot: waveform slot (0-7)
 * @param w: effect (1 - 117)
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_set_waveform(uint8_t slot, uint8_t w)
{
  return write_register8(DRV2605_REG_WAVESEQ1 + slot, w);
}


/**
 * drv2605_select_library()
 * 
 * @brief Select a library from DRV2605L library set
 * @param lib: library number
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_select_library(uint8_t lib)
{
  return write_register8(DRV2605_REG_LIBRARY, lib);
}


/**
 * drv2605_go()
 * 
 * @brief Trigger internal GO
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_go(void)
{
  return write_register8(DRV2605_REG_GO, 1);
}


/**
 * drv2605_stop()
 * 
 * @brief Reset internal GO (stop motor)
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_stop(void)
{
  return write_register8(DRV2605_REG_GO, 0);
}


/**
 * drv2605_set_mode()
 * 
 * @brief Set DRV2605L mode
 * @param mode: mode to set
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_set_mode(uint8_t mode)
{
  return write_register8(DRV2605_REG_MODE, mode);
}


/**
 * drv2605_set_realtime_value()
 * 
 * @brief Set DRV2605L RTP value
 * @param rtp: rtp value
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_set_realtime_value(uint8_t rtp)
{
  return write_register8(DRV2605_REG_RTPIN, rtp);
}

/**
 * drv2605_self_test()
 * 
 * @brief DRV2605L self-test/diagnostic mode
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_self_test(void)
{
  uint8_t result;
  
  ESP_LOGI(TAG, "enabling diagnostic mode");
  write_register8(DRV2605_REG_MODE, 6); /* Diagnostic mode */
  ESP_LOGI(TAG, "starting diagnostic");
  write_register8(DRV2605_REG_GO, 1);
  result = 1;
  while (result & 1)
    read_register8(DRV2605_REG_GO, &result);
  ESP_LOGI(TAG, "diagnostic done");
  read_register8(DRV2605_REG_STATUS, &result);
  ESP_LOGI(TAG, "status=%02x", result);
  if (result&0x08)
    ESP_LOGE(TAG, "actuator issue");
  else
    ESP_LOGI(TAG, "actuator ok");

  return ESP_OK;
}


/**
 * drv2605_use_erm()
 * 
 * @brief Tell DRV2605L we are using an ERM actuator
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_use_erm(void)
{
  uint8_t temp;

  if (read_register8(DRV2605_REG_FEEDBACK, &temp) != ESP_OK)
    return ESP_FAIL;

  return write_register8(DRV2605_REG_FEEDBACK, temp & 0x7F);
}


/**
 * drv2605_use_lra()
 * 
 * @brief Tell DRV2605L we are using an LRA actuator
 * @return ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t drv2605_use_lra(void)
{
 uint8_t temp;

  if (read_register8(DRV2605_REG_FEEDBACK, &temp) != ESP_OK)
    return ESP_FAIL;

  return write_register8(DRV2605_REG_FEEDBACK, temp | 0x80);
}
