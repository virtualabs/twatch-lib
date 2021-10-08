#include "hal/hal.h"

/**
 * twatch_hal_init()
 * 
 * @brief Initialize hardware abstraction layer
 **/

bool twatch_hal_init(void)
{
  /* Initialize I2C */
  twatch_i2c_init();

  /* Initialize Power Management Unit. */
  if (twatch_pmu_init() != ESP_OK)
  {
    return false;
  }

  /* Initialize screen. */
  if (twatch_screen_init() != ESP_OK)
  {
    return false;
  }

  /* Initialize touch screen. */
  if (twatch_touch_init() != ESP_OK)
  {
    return false;
  }

  #ifdef CONFIG_TWATCH_V1
    /* Initialize vibrate */
    if (twatch_vibrate_init() != ESP_OK)
    {
      return false;
    }

    /* Initialize audio. */
    if (twatch_audio_init(SOUND_DEFAULT_SAMPLE_RATE) != ESP_OK)
    {
      return false;
    }
  #endif

  /* Success ! */
  return true;
}