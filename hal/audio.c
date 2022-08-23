#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "hal/audio.h"

volatile int sample_rate = SOUND_DEFAULT_SAMPLE_RATE;

/**
 * sound_init()
 *
 * Initialize audio DAC interface (I2S).
 **/

esp_err_t twatch_audio_init(int sample_rate)
{
  #if defined(CONFIG_TWATCH_V1) || defined(CONFIG_TWATCH_V3)
    esp_err_t result;
    i2s_config_t      ss_config;
    i2s_pin_config_t  ss_pin_config;

    /**
     * Initialize the sound system.
     **/

    /* Initialize ESP32 I2S config. */
    ss_config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
    ss_config.sample_rate = sample_rate;
    ss_config.bits_per_sample = 16;
    ss_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    ss_config.communication_format = I2S_COMM_FORMAT_STAND_MSB;

    /* Use ESP_INTR_FLAG_LEVEL2, as ESP_INTR_FLAG_LEVEL1 is already used. */
    ss_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL2;
    ss_config.dma_buf_count = 6;
    ss_config.dma_buf_len = 60;
    ss_config.use_apll = false;

    /* Initialize ESP32 pin config. */
    ss_pin_config.bck_io_num = 26;
    ss_pin_config.ws_io_num = 25;
    ss_pin_config.data_out_num = 33;
    ss_pin_config.data_in_num = -1;

    /* Install I2S driver. */
    result = i2s_driver_install(
      SOUND_DEFAULT_I2S_PORT,
      &ss_config,
      0,
      NULL
    );
    if (result != ESP_OK)
    {
      /* Error while installing I2S driver. */
      ESP_LOGE("sound_system", "cannot install i2s driver: %d\r\n", result);
      return ESP_FAIL;
    }

    result = i2s_set_pin(SOUND_DEFAULT_I2S_PORT, &ss_pin_config);
    if (result != ESP_OK)
    {
      /* Error while setting pins. */
      ESP_LOGE("sound_system", "setting i2s pins failed: %d\r\n", result);
      return ESP_FAIL;
    }
  #endif

  /* Success. */
  return ESP_OK;
}


/**
 * sound_send_samples()
 *
 * Send 2-channel 16-bit samples to audio DAC.
 **/

esp_err_t twatch_audio_send_samples(void *samples, size_t samples_size, size_t *p_bytes_written, TickType_t ticks_to_wait)
{
  #if defined(CONFIG_TWATCH_V1) || defined(CONFIG_TWATCH_V3)
    return i2s_write(
      SOUND_DEFAULT_I2S_PORT,
      samples,
      samples_size,
      p_bytes_written,
      ticks_to_wait
    );
  #else
    return ESP_FAIL;
  #endif
}

/**
 * audio_deinit()
 *
 * Deinitialize audio DAC interface.
 **/

esp_err_t twatch_audio_deinit(void)
{
  #if defined(CONFIG_TWATCH_V1) || defined(CONFIG_TWATCH_V3)
    esp_err_t result;

    result = i2s_driver_uninstall(SOUND_DEFAULT_I2S_PORT);
    if (result != ESP_OK)
    {
      /* Error, cannot uninstall driver. */
      ESP_LOGE("sound_system", "cannot uninstall driver: %d\r\n", result);
      return ESP_FAIL;
    }
  #endif

  /* Success. */
  return ESP_OK;
}
