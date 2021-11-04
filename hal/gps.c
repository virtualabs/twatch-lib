#include <ctype.h>

#include "hal/gps.h"


#define GPS_RX_BUFSIZE      1024
#define NMEA_GNGGA_PREFIX   "GNGGA"
#define NMEA_GNRMC_PREFIX   "GNRMC"
#define NMEA_PREFIX_LENGTH  5
#define TAG "[hal::gps]"

//static uint8_t g_gga_test_terms[] =  "064036.289,4836.5375,N,00740.9373,E,1,04,3.2,200.2,M,,,,0000";
//static uint8_t g_gga_test_terms[] =  "064036.289,,,00740.9373,E,1,04,3.2,200.2,M,,,,0000";
//static uint8_t g_rmc_test_terms[] = "053740.000,A,2503.6319,N,12136.0099,E,2.69,79.65,100106,,,A";

volatile uint8_t gps_rx_buffer[GPS_RX_BUFSIZE];
volatile int gps_rx_buffer_len = 0;

typedef enum {
  GPS_IDLE,
  GPS_ON,
  GPS_READY
} gps_state_t;

enum gga_term_pos {
  GGA_TIME_TERM,
  GGA_LAT_TERM,
  GGA_LAT_DIR_TERM,
  GGA_LNG_TERM,
  GGA_LNG_DIR_TERM,
  GGA_POS_TYPE_TERM,
  GGA_SAT_TERM,
  GGA_HDOP_TERM,
  GGA_ALT_TERM,
  GGA_ALT_UNIT_TERM
};

enum rmc_term_pos {
  RMC_TIME_TERM,
  RMC_VALID_TERM,
  RMC_LAT_TERM,
  RMC_LAT_DIR_TERM,
  RMC_LNG_TERM,
  RMC_LNG_DIR_TERM,
  RMC_SPEED_TERM,
  RMC_ROUTE_TERM,
  RMC_DATE_TERM,
  RMC_DECMAG_TERM,
  RMC_DECMAG_DIR_TERM,
  RMC_POS_MODE_TERM
};

/* GPS state (FSM). */
volatile gps_state_t g_gps_state;
static gps_raw_degrees_t g_lat, g_lng;
int g_nb_sat = 0;
int g_hdop;
int g_alt;
int g_speed;
int g_date;
int g_time;

/* Include the following code only for Twatch v2. */
#ifdef CONFIG_TWATCH_V2

/**
 * hex_to_dec()
 * 
 * @brief Convert an hexadecimal digit into its value.
 * @param ascii: hexadecimal digit to convert
 * @return digit value
 **/

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


/**
 * hexbyte_to_dec()
 * 
 * @brief Convert 2-digit hexadecimal value to its value
 * @param p_hexbyte: pointer to a 2-digit hex value
 * @return corresponding value as int
 **/

static int hexbyte_to_dec(uint8_t *p_hexbyte)
{
  int low = hex_to_dec(*(p_hexbyte + 1));
  int high = hex_to_dec(*(p_hexbyte));
  if ((low>=0) && (high >= 0))
    return (low + high*16);
  else
    return -1;
}


/**
 * gps_read_data()
 * 
 * @brief Read data sent by GPS into our receive buffer
 * @param p_uart_evt: pointer to a `uart_event_t` structure representing an event
 * @return TWATCH_GPS_ERROR on error, TWATCH_GPS_RX_FULL if FIFO is full, or size of bytes read on success.
 **/

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


/**
 * gps_parse_decimal_2()
 * 
 * @brief Parse a 2 decimal value (XXXX.YY)
 * @param term: pointer to a text string representing the decimal value
 * @return decimal value x 100 (as integer)
 **/

int32_t gps_parse_decimal_2(uint8_t *term)
{
  bool negative = *term == '-';
  if (negative) ++term;
  int32_t ret = 100 * (int32_t)atol((char *)term);
  while (isdigit(*term)) ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 10 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += term[2] - '0';
  }
  return negative ? -ret : ret;
}


/**
 * gps_parse_decimal_3()
 * 
 * @brief Parse a 3 decimal value (XXXX.YYY)
 * @param term: pointer to a text string representing the decimal value
 * @return decimal value x 1000 (as integer)
 **/

int32_t gps_parse_decimal_3(uint8_t *term)
{
  bool negative = *term == '-';
  if (negative) ++term;
  int32_t ret = 1000 * (int32_t)atol((char *)term);
  while (isdigit(*term)) ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 100 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += 10 * (term[2] - '0');
    if (isdigit(term[3]))
      ret += term[3] - '0';
  }
  return negative ? -ret : ret;
}

/**
 * gps_parse_degrees()
 * 
 * @brief Parse a lat/lng degrees value from text
 * @param term: pointer to a text string representing the decimal value
 * @param p_degrees: pointer to a `gps_raw_degrees_t` structure that will be filled
 **/

void gps_parse_degrees(uint8_t *term, gps_raw_degrees_t *p_degrees)
{
  uint32_t leftOfDecimal = (uint32_t)atol((char *)term);
  uint16_t minutes = (uint16_t)(leftOfDecimal % 100);
  uint32_t multiplier = 10000000UL;
  uint32_t tenMillionthsOfMinutes = minutes * multiplier;

  p_degrees->deg = (int16_t)(leftOfDecimal / 100);

  while (isdigit(*term))
    ++term;

  if (*term == '.')
    while (isdigit(*++term))
    {
      multiplier /= 10;
      tenMillionthsOfMinutes += (*term - '0') * multiplier;
    }

  p_degrees->billionths = (5 * tenMillionthsOfMinutes + 1) / 3;
  p_degrees->negative = false;
}

/**
 * gps_parse_gngga_sentence()
 * 
 * @brief Parse a NMEA GNGGA sentence.
 * @param nmea_gga_terms: pointer to a text string containing the terms to parse
 **/

static void gps_parse_gngga_sentence(uint8_t *nmea_gga_terms)
{
  int term_pos, term_length;
  int32_t i32_time;
  uint8_t *term_start, *term_end;
  
  /* We start with first term. */
  term_pos = 0;

  /* Parse terms from start. */
  term_start = nmea_gga_terms;
  term_end = term_start;

  /* Loop until null char is met. */
  while (*term_end != 0)
  {
    /* Process character. */
    switch(*term_end)
    {
      case ',':
        {
          /* Term separator found, parse term. */
          *term_end = 0;

          switch (term_pos)
          {
            case GGA_TIME_TERM:
              {
                if ((term_end - term_start) > 0)
                {
                  /* Parse decimal number as time. */
                  g_time = gps_parse_decimal_3(term_start);
                  ESP_LOGI(TAG, "time as int: %u", g_time);
                }
              }
              break;

            case GGA_LAT_TERM:
              {
                if ((term_end - term_start) > 0)
                {
                  gps_parse_degrees(term_start, &g_lat);
                  ESP_LOGI(TAG, "lat: %d.%d", g_lat.deg, g_lat.billionths);
                }
              }
              break;

            case GGA_LAT_DIR_TERM:
              {
                if (*term_start == 'S')
                  g_lat.negative = true;
              }
              break;

            case GGA_LNG_TERM:
              {
                if ((term_end - term_start) > 0)
                {
                  gps_parse_degrees(term_start, &g_lng);
                  ESP_LOGI(TAG, "lng: %d.%d", g_lng.deg, g_lng.billionths);
                }
              }
              break;

            case GGA_LNG_DIR_TERM:
              {
                if (*term_start == 'W')
                  g_lng.negative = true;
              }
              break;

            case GGA_SAT_TERM:
              {
                /* Parse integer. */
                g_nb_sat = atol((char *)term_start);
                ESP_LOGI(TAG, "sat: %d", g_nb_sat);
              }
              break;

            case GGA_HDOP_TERM:
              {
                g_hdop = gps_parse_decimal_2(term_start);
                ESP_LOGI(TAG, "hdop: %d", g_hdop);
              }
              break;

            case GGA_ALT_TERM:
              {
                g_alt = gps_parse_decimal_2(term_start);
                ESP_LOGI(TAG, "alt: %d", g_alt);
              }

            /* ... */
            default:
              break;
          }

          /* Parse next term. */
          term_start = term_end + 1;
          term_end = term_start;
          term_pos++;
        }
        break;

      default:
        {
          term_end++;
        }
        break;
    }
  }
}


/**
 * gps_parse_gnrmc_sentence()
 * 
 * @brief Parse a NMEA GNRMC sentence.
 * @param nmea_rmc_terms: pointer to a text string containing the terms to parse
 **/

static void gps_parse_gnrmc_sentence(uint8_t *nmea_rmc_terms)
{
  int term_pos, term_length;
  int32_t i32_time;
  uint8_t *term_start, *term_end;
  bool b_valid_data = false;
  
  /* We start with first term. */
  term_pos = 0;

  /* Parse terms from start. */
  term_start = nmea_rmc_terms;
  term_end = term_start;

  /* Loop until null char is met. */
  while (*term_end != 0)
  {
    /* Process character. */
    switch(*term_end)
    {
      case ',':
        {
          /* Term separator found, parse term. */
          *term_end = 0;

          switch (term_pos)
          {
            case RMC_TIME_TERM:
              {
                if ((term_end - term_start) > 0)
                {
                  /* Parse decimal number as time. */
                  g_time = gps_parse_decimal_3(term_start);
                  ESP_LOGI(TAG, "time as int: %u", g_time);
                }
              }
              break;

            case RMC_VALID_TERM:
              {
                b_valid_data = (*term_start == 'A');
              }
              break;

            case RMC_LAT_TERM:
              {
                if (b_valid_data)
                {
                  if ((term_end - term_start) > 0) 
                  {
                    gps_parse_degrees(term_start, &g_lat);
                    ESP_LOGI(TAG, "lat: %d.%d", g_lat.deg, g_lat.billionths);
                  }
                }
              }
              break;

            case RMC_LAT_DIR_TERM:
              {
                if (b_valid_data)
                {
                  if (*term_start == 'S')
                    g_lat.negative = true;
                }
              }
              break;

            case RMC_LNG_TERM:
              {
                if (b_valid_data)
                {
                  if ((term_end - term_start) > 0)
                  {
                    gps_parse_degrees(term_start, &g_lng);
                    ESP_LOGI(TAG, "lng: %d.%d", g_lng.deg, g_lng.billionths);
                  }
                }
              }
              break;

            case RMC_LNG_DIR_TERM:
              {
                if (b_valid_data)
                {
                  if (*term_start == 'W')
                    g_lng.negative = true;
                }
              }
              break;

            case RMC_SPEED_TERM:
              {
                if (b_valid_data)
                {
                  /* Parse integer. */
                  g_speed = gps_parse_decimal_2(term_start);
                  ESP_LOGI(TAG, "speed: %d", g_speed);
                }
              }
              break;

            case RMC_DATE_TERM:
              {
                if (b_valid_data)
                {
                  g_date = gps_parse_decimal_2(term_start)/100;
                  ESP_LOGI(TAG, "date: %d", g_date);
                }
              }
              break;

            /* ... */
            default:
              break;
          }

          /* Parse next term. */
          term_start = term_end + 1;
          term_end = term_start;
          term_pos++;
        }
        break;

      default:
        {
          term_end++;
        }
        break;
    }
  }
}


/**
 * gps_parse_sentence()
 * 
 * @brief Parse a given NMEA sentence
 * @param nmea_sentence: pointer to a NMEA sentence
 **/

static void gps_parse_sentence(uint8_t *nmea_sentence)
{
  if (nmea_sentence[NMEA_PREFIX_LENGTH] == ',')
  {
    /* Only process GNGGA et GNRMC sentences. */
    if (!strncmp((char *)nmea_sentence, (char *)NMEA_GNGGA_PREFIX, NMEA_PREFIX_LENGTH))
    {
      /* Process GNGGA sentence. */
      gps_parse_gngga_sentence(&nmea_sentence[NMEA_PREFIX_LENGTH + 1]);
      //gps_parse_gngga_sentence((uint8_t *)g_gga_test_terms);
    }
    else if (!strncmp((char *)nmea_sentence, (char *)NMEA_GNRMC_PREFIX, NMEA_PREFIX_LENGTH))
    {
      /* Process GNRMC sentence. */
      gps_parse_gnrmc_sentence(&nmea_sentence[NMEA_PREFIX_LENGTH + 1]);
      //gps_parse_gnrmc_sentence(g_rmc_test_terms);
    }
  }
}

/**
 * gps_process_rx()
 * 
 * @brief Wait data from GPS module and process it.
 * @param nmea_rmc_terms: pointer to a text string containing the terms to parse
 * @return number of bytes read from GPS.
 **/

static void gps_process_rx()
{
  uart_event_t uart_evt;
  int nb_rx_bytes;
  int i;
  uint8_t parity = 0;
  int checksum;

  int sentence_start = -1;
  int sentence_end = -1;


  /* Wait for UART event. */
  if(twatch_uart_wait_event(&uart_evt))
  {
    //ESP_LOGI(TAG, "uart[%d] event: %d", EX_UART_NUM, uart_evt.type);
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
                            //ESP_LOGI(TAG, "received valid sentence: %s", &gps_rx_buffer[sentence_start]);
                            gps_parse_sentence(&gps_rx_buffer[sentence_start + 1]); /* Skip the '$' marker */
                          }
                          else
                          {
                            gps_rx_buffer[i] = '\0';
                            //ESP_LOGI(TAG, "received invalid sentence: %s", &gps_rx_buffer[sentence_start]);
                          }
                        }
                        else
                        {
                          //ESP_LOGI(TAG, "checksum error for sentence: %s", gps_rx_buffer);
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
}


/**
 * twatch_gps_control()
 * 
 * @brief Manage the GPS state machine
 * @param pvParameters: pointer to extra parameters (none used)
 **/

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

#endif /* CONFIG_TWATCH_V2 */


/**
 * twatch_gps_init()
 * 
 * @brief Initialize our GPS module
 * @return ESP_FAIL on failure, ESP_OK on success.
 **/

esp_err_t twatch_gps_init(void)
{
  /* Initialize GPS data. */
  memset(&g_lat, 0, sizeof(gps_raw_degrees_t));
  memset(&g_lng, 0, sizeof(gps_raw_degrees_t));
  g_speed = 0;
  g_date = 0;
  g_time = 0;
  g_alt = 0;
  g_nb_sat = 0;
  g_hdop = 0;

#ifdef CONFIG_TWATCH_V2
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
#else
  return ESP_FAIL;
#endif
}


/**
 * twatch_gps_on()
 * 
 * @brief Enable GPS
 * @return ESP_FAIL on error, ESP_OK on success.
 **/

esp_err_t twatch_gps_on(void)
{

  esp_err_t result = ESP_FAIL;

#ifdef CONFIG_TWATCH_V2
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
#endif

  return result;
}


/**
 * twatch_gps_on()
 * 
 * @brief Disable GPS
 * @return ESP_FAIL on error, ESP_OK on success.
 **/

esp_err_t twatch_gps_off(void)
{
  esp_err_t result = ESP_FAIL;

#ifdef CONFIG_TWATCH_V2
  /* Power off GPS through LDO4. */
  ESP_LOGI(TAG, "switching off GPS ...");
  result = twatch_pmu_gps_power(false);

  /* Mark GPS as powered off. */
  if (result == ESP_OK)
    g_gps_state = GPS_IDLE;
#endif

  /* Return result. */
  return result;
}

/**
 * gps_get_lat_lng()
 * 
 * @brief Retrieve GPS latitude/longitude information
 * @param p_lat: pointer to a `gps_raw_degrees_t` structure representing the latitude
 * @param p_lng: pointer to a `gps_raw_degrees_t` structure representing the longitude
 **/

void gps_get_lat_lng(gps_raw_degrees_t *p_lat, gps_raw_degrees_t *p_lng)
{
  /* Copy without calling memcpy(). */
  p_lat->deg = g_lat.deg;
  p_lat->billionths = g_lat.billionths;
  p_lat->negative = g_lat.negative;
  p_lng->deg = g_lng.deg;
  p_lng->billionths = g_lng.billionths;
  p_lng->negative = g_lng.negative;
}

/**
 * gps_get_day()
 * 
 * @brief Get the current day of month
 * @return day in month starting from 1
 **/

int gps_get_day(void)
{
  return (g_date/1000000)%100;
}


/**
 * gps_get_month()
 * 
 * @brief Get the current month
 * @return month number starting from 1
 **/

int gps_get_month(void)
{
  return (g_date/10000)%100;
}


/**
 * gps_get_year()
 * 
 * @brief Get the current year
 * @return current year (2000+)
 **/

int gps_get_year(void)
{
  return (g_date/100)%100 + 2000;
}


/**
 * gps_get_secs()
 * 
 * @brief Get the number of seconds from the last retrieved time value
 * @return seconds (0-59)
 **/

int gps_get_secs(void)
{
  return (g_time/1000)%100;
}

/**
 * gps_get_mins()
 * 
 * @brief Get the number of minutes from the last retrieved time value
 * @return seconds (0-59)
 **/

int gps_get_mins(void)
{
  return (g_time/100000)%100;
}


/**
 * gps_get_hours()
 * 
 * @brief Get the number of hours from the last retrieved time value
 * @return hours (0-23)
 **/

int gps_get_hours(void)
{
  return (g_time/10000000)%100;
}


/**
 * gps_get_speed()
 * 
 * @brief Get the speed from the last retrieved value
 * @return speed in knots
 **/

float gps_get_speed(void)
{
  return g_speed/100.0;
}


/**
 * gps_get_satellites()
 * 
 * @brief Get the number of satellites reported by the GPS module
 * @return number of satellites
 **/

int gps_get_satellites(void)
{
  return g_nb_sat;
}


/**
 * gps_get_hdop()
 * 
 * @brief Get the Horizontal Dilution Of Precision
 * @return hdop value
 **/

float gps_get_hdop(void)
{
  return g_hdop/100.0;
}


/**
 * gps_get_altitude()
 * 
 * @brief Get the current altitude
 * @return altitude (supposedly in meters)
 **/

float gps_get_alt(void)
{
  return g_alt/100.0;
}