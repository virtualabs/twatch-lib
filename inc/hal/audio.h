#ifndef __INC_AUDIO_H
#define __INC_AUDIO_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"

#define SOUND_DEFAULT_I2S_PORT  (0)
#define SOUND_DEFAULT_SAMPLE_RATE (44100)

/* Initialize 16-bit sound system. */
esp_err_t audio_init(int sample_rate);

/* Send samples to sound system. */
esp_err_t audio_send_samples(void *samples, size_t samples_size, size_t *p_bytes_written, TickType_t ticks_to_wait);

/* Deinitialize 16-bit sound system. */
esp_err_t audio_deinit(void);


#endif /* __INC_AUDIO_H */
