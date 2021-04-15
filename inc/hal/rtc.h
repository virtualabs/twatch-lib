#ifndef __INC_TWATCH_RTC_H
#define __INC_TWATCH_RTC_H

#include "drivers/pcf8563.h"

/* Types aliases. */
typedef pcf8563_datetime_t rtc_datetime_t;
typedef pcf8563_alarm_t rtc_alarm_t;

esp_err_t twatch_rtc_init(void);

/* Exported functions. */
esp_err_t twatch_rtc_set_date_time(rtc_datetime_t *p_datetime_t);
esp_err_t twatch_rtc_get_date_time(rtc_datetime_t *p_datetime_t);

/* Set/get/enable/disable alarm. */
esp_err_t twatch_rtc_set_alarm(rtc_alarm_t *p_alarm);
esp_err_t twatch_rtc_get_alarm(rtc_alarm_t *p_alarm);
void twatch_rtc_enable_alarm(bool enable);


#endif /* __INC_TWATCH_RTC_H */