#include "hal/gps.h"

#define GPS_RX_BUFSIZE  1024
#define TAG "[hal::gps]"

volatile uint8_t gps_rx_buffer[GPS_RX_BUFSIZE];
volatile int gps_rx_buffer_len = 0;

typedef enum {
  GPS_IDLE,
  GPS_ON,
  GPS_READY
} gps_state_t;


/* GPS state (FSM). */
volatile gps_state_t g_gps_state;

static int hex_to_dec(uint8_t ascii)
{
  if ((ascii >= '0') && (ascii <= '9'))
    return (ascii - '0');
  else if ((ascii >= 'A') && (ascii <= 'F'))
    return (ascii - 'A') + 10;
  else if ((ascii >= 'a') && (ascii <= 'f'))
    return (ascii - 'a') + 10;
  else
    return -1;
}

static int hexbyte_to_dec(uint8_t *p_hexbyte)
{
  int low = hex_to_dec(*(p_hexbyte + 1));
  int high = hex_to_dec(*(p_hexbyte));
  if ((low>=0) && (high >= 0))
    return (low + high*16);
  else
    return -1;
}

static int gps_read_data(uart_event_t *p_uart_evt)
{
  /* Ensure we are reading data from UART. */
  if (p_uart_evt->type == UART_DATA)
  {
    /* Check if GPS rx buffer is full. */
    if ((gps_rx_buffer_len + p_uart_evt->size) < GPS_RX_BUFSIZE)
    {
      /* Read data into our RX buffer. */
      twatch_uart_receive(&gps_rx_buffer[gps_rx_buffer_len], p_uart_evt->size);

      /* Take into account the newly read data. */
      gps_rx_buffer_len += p_uart_evt->size;

      /* Success. */
      return p_uart_evt->size;
    }
    else
    {
      /* RX buffer is full. */
      return TWATCH_GPS_RX_FULL;
    }
  }
  else
  {
    /* TODO. */
    return TWATCH_GPS_ERROR;
  }
}

static int gps_process_rx()
{
  uart_event_t uart_evt;
  int nb_rx_bytes;
  int i, result = -1;
  uint8_t parity = 0;
  int checksum;

  int sentence_start = -1;
  int sentence_end = -1;


  /* Wait for UART event. */
  if(twatch_uart_wait_event(&uart_evt))
  {
    ESP_LOGI(TAG, "uart[%d] event: %d", EX_UART_NUM, uart_evt.type);
    switch(uart_evt.type)
    {
      case UART_DATA:
        {
          /* Fetch data. */
          nb_rx_bytes = gps_read_data(&uart_evt);

          /* Check we managed to read data. */
          if (nb_rx_bytes > 0)
          {
            sentence_start = -1;
            for (i=0; i<gps_rx_buffer_len; i++)
            {
              switch (gps_rx_buffer[i])
              {

                case '$':
                  {
                    if (sentence_start < 0)
                    {
                      sentence_start = i;
                      parity = 0;
                    }
                  }
                  break;

                case '*':
                  {
                    if (sentence_start >= 0)
                    {
                      if (gps_rx_buffer_len >= (i + 2))
                      {
                        sentence_end = i+3;
                        checksum = hexbyte_to_dec(&gps_rx_buffer[i+1]);
                        if (checksum >= 0)
                        {
                          if (parity == (uint8_t)checksum)
                          {
                            /* NMEA sentence is valid, parse. */
                            gps_rx_buffer[i] = '\0';
                            ESP_LOGI(TAG, "received valid sentence: %s", &gps_rx_buffer[sentence_start]);
                          }
                          else
                          {
                            gps_rx_buffer[i] = '\0';
                            ESP_LOGI(TAG, "received invalid sentence: %s", &gps_rx_buffer[sentence_start]);
                          }
                        }
                        else
                        {
                          ESP_LOGI(TAG, "checksum error for sentence: %s", gps_rx_buffer);
                        }
                      }

                      sentence_start = -1;
                      parity = 0;
                    }
                  }
                  break;

                case '\n':
                case '\r':
                  sentence_start = -1;
                  parity = 0;
                  break;

                default:
                  parity ^= gps_rx_buffer[i];
                  break;
              }
            }

            if (sentence_end >= 0)
            {
              /* Chomp. */
              memcpy(gps_rx_buffer, &gps_rx_buffer[sentence_end], GPS_RX_BUFSIZE - sentence_end);
              gps_rx_buffer_len -= sentence_end; 
            }
            else if (sentence_start >= 0)
            {
              /* Chomp. */
              memcpy(gps_rx_buffer, &gps_rx_buffer[sentence_start], GPS_RX_BUFSIZE - sentence_start);
              gps_rx_buffer_len -= sentence_start; 
            }
          }
        }
        break;

      default:
        break;
    }
  }

  return result;
}


static void twatch_gps_control(void *pvParameters)
{
  uart_event_t uart_evt;
  uint8_t buffer[256];
  for(;;)
  {

    switch(g_gps_state)
    {
      case GPS_IDLE:
        break;

      case GPS_ON:
        {
          ESP_LOGI(TAG, "Waking up GPS ...");
          /* Wake-up GPS. */
          vTaskDelay(60/portTICK_RATE_MS);
          gpio_set_level(TWATCH_GPS_WAKEUP, 0);
          vTaskDelay(200/portTICK_RATE_MS);
          gpio_set_level(TWATCH_GPS_WAKEUP, 1);

          /* GPS should be woken up. */
          ESP_LOGI(TAG, "GPS has been woken up !");
          g_gps_state = GPS_READY;
        }
        break;

      case GPS_READY:
        {
          gps_process_rx();
        }
        break;
    }

    vTaskDelay(10);
  }
}

esp_err_t twatch_gps_init(void)
{
  /* Initialize UART. */
  twatch_uart_init(9600);

  /* Initialize wake-up GPIO. */
  gpio_config_t gps_wake_up;

  /* Configure wake-up GPIO. */
  gps_wake_up.mode = GPIO_MODE_OUTPUT;
  gps_wake_up.pin_bit_mask = (1ULL << TWATCH_GPS_WAKEUP);
  gpio_config(&gps_wake_up);
  gpio_set_level(TWATCH_GPS_WAKEUP, 1);

  /* GPS is idling. */
  g_gps_state = GPS_IDLE;

  /* Start our GPS controller. */
  return xTaskCreate(twatch_gps_control, "twatch_gps_control", 10000, NULL, 12, NULL);
}

esp_err_t twatch_gps_on(void)
{
  esp_err_t result = ESP_FAIL;

  if (g_gps_state == GPS_IDLE)
  {
    ESP_LOGI(TAG, "switching on GPS ...");

    /* Power on GPS through LDO4. */
    result = twatch_pmu_gps_power(true);
    vTaskDelay(200/portTICK_RATE_MS);

    /* Change current state. */
    g_gps_state = GPS_ON;
  }
  else
    ESP_LOGE(TAG, "GPS is already on !");

  return result;
}

esp_err_t twatch_gps_off(void)
{
  esp_err_t result;

  /* Power off GPS through LDO4. */
  ESP_LOGI(TAG, "switching off GPS ...");
  result = twatch_pmu_gps_power(false);

  /* Mark GPS as powered off. */
  if (result == ESP_OK)
    g_gps_state = GPS_IDLE;

  /* Return result. */
  return result;
}
