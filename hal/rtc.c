#include "hal/rtc.h"

esp_err_t twatch_rtc_init(void)
{ 
  if (pcf8563_init() == ESP_OK)
  {
    if (pcf8563_probe() == ESP_OK)
    {
      return ESP_OK;
    }
  }

  return ESP_FAIL;
}

esp_err_t twatch_rtc_set_date_time(rtc_datetime_t *p_datetime)
{
  return pcf8563_set_date_time((pcf8563_datetime_t *) p_datetime);
}

esp_err_t twatch_rtc_get_date_time(rtc_datetime_t *p_datetime)
{
  return pcf8563_get_date_time((pcf8563_datetime_t *) p_datetime);
}

esp_err_t twatch_rtc_set_alarm(rtc_alarm_t *p_alarm)
{
  return pcf8563_set_alarm((pcf8563_alarm_t *)p_alarm);
}

esp_err_t twatch_rtc_get_alarm(rtc_alarm_t *p_alarm)
{
  return pcf8563_get_alarm((pcf8563_alarm_t *)p_alarm);
}

void twatch_rtc_enable_alarm(bool enable)
{
  /* TODO ! */
}