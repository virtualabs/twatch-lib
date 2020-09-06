#include "hal/vibrate.h"

#define TAG "[hal::vibrate]"

void _twatch_vibration_task(void *parameter)
{
  vibrate_parameter_t *vibration_config = (vibrate_parameter_t *)parameter;
  int i;

  ESP_LOGI(TAG, "Starting vibration task...");

  /* Drive motor based on vibration_config. */
  switch (vibration_config->mode)
  {
    case VIBRATE_DURATION:
      {
        ESP_LOGI(TAG, "single vibration enabled");

        /* Enable motor. */
        gpio_set_level(GPIO_NUM_4, 1);

        /* Wait for duration. */
        vTaskDelay(vibration_config->duration/portTICK_RATE_MS);

        /* Stop motor. */
        gpio_set_level(GPIO_NUM_4, 0);
      }
      break;

    case VIBRATE_PATTERN:
      {
        ESP_LOGI(TAG, "pattern vibration enabled");
        for (i=0; i<vibration_config->pattern_length; i++)
        {
          if (vibration_config->pattern[i].level == 1)
          {
            /* Enable motor. */
            gpio_set_level(GPIO_NUM_4, 1);

            /* Wait for duration. */
            vTaskDelay(vibration_config->pattern[i].duration/portTICK_RATE_MS);
          }
          else
          {
            /* Disable motor. */
            gpio_set_level(GPIO_NUM_4, 0);

            /* Wait for duration. */
            vTaskDelay(vibration_config->pattern[i].duration/portTICK_RATE_MS);
          }
        }
      }
      break;
  }

  /* Free vibration_config. */
  ESP_LOGI(TAG, "freeing memory ...");
  free(vibration_config);

  vTaskDelete( NULL );
}

esp_err_t twatch_vibrate_init(void)
{
  gpio_config_t motor;

  /* Configure GPIO. */
  motor.mode = GPIO_MODE_OUTPUT;
  motor.pin_bit_mask = (1ULL << GPIO_NUM_4);
  gpio_config(&motor);

  /* Initialized. */
  return ESP_OK;
}

esp_err_t twatch_vibrate_vibrate(int duration)
{
  vibrate_parameter_t *parameter = (vibrate_parameter_t *)malloc(sizeof(vibrate_parameter_t));
  if (parameter != NULL)
  {
    parameter->mode = VIBRATE_DURATION;
    parameter->duration = duration;

    /* Start a vibration task. */
    xTaskCreate(_twatch_vibration_task, "_vib_task", 10000, parameter, 1, NULL);

    /* Success. */
    return ESP_OK;
  }
  else
    /* Cannot allocate memory. */
    return ESP_FAIL;
}

esp_err_t twatch_vibrate_pattern(vibrate_pattern_t *pattern, int length)
{
  vibrate_parameter_t *parameter = (vibrate_parameter_t *)malloc(sizeof(vibrate_parameter_t));
  if (parameter != NULL)
  {
    parameter->mode = VIBRATE_PATTERN;
    parameter->pattern = pattern;
    parameter->pattern_length = length;

    /* Start a vibration task. */
    xTaskCreate(_twatch_vibration_task, "_vib_task", 10000, parameter, 1, NULL);

    /* Success. */
    return ESP_OK;
  }
  else
    /* Cannot allocate memory. */
    return ESP_FAIL;
}
