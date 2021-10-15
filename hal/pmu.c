#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/pmu.h"
#include "hal/screen.h" // DEBUG ONLY

#define AXP_CHECK(x) if(x != AXP_PASS) return ESP_FAIL

/* User button handling. */
volatile bool b_axpxx_irq_triggered = false;
volatile int userbtn_int_count = 0;
portMUX_TYPE userbtn_mux = portMUX_INITIALIZER_UNLOCKED;

/* USB charge monitoring. */
volatile bool b_usb_plugged = false;

/**
 * _axpxx_interrupt_handler()
 *
 * Internal interrupt handler for AXP202 IRQ.
 **/

void IRAM_ATTR _axpxx_interrupt_handler(void *parameter)
{
  /* Mark AXP202 IRQ as triggered. */
  b_axpxx_irq_triggered = true;
}

/**
 * twatch_pmu_init()
 *
 * Initializes the watch Power Management Unit (AXP202) and its associated
 * I2C channel.
 **/

esp_err_t twatch_pmu_init(void)
{
  gpio_config_t irq_conf;

  /* Initialize AXP202 IRQ pin as input pin. */
  rtc_gpio_deinit(GPIO_NUM_35);
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;
  irq_conf.pin_bit_mask = (1ULL << 35);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_down_en = 0;
  irq_conf.pull_up_en = 1;
  gpio_config(&irq_conf);


  /* Install our user button interrupt handler. */
  if (gpio_install_isr_service(0) != ESP_OK)
    printf("[pmu::isr] Error while installing service\r\n");
  gpio_isr_handler_add(GPIO_NUM_35, _axpxx_interrupt_handler, NULL);

  /* Initialize I2C master communication. */
  axpxx_i2c_init();

  /* Initialize AXP202. */
  if (axpxx_probe_chip() == AXP_PASS)
  {
    /* Enable PEK short press IRQ. */
    axpxx_enableIRQ((1 << AXP202_IRQ_POKSH) | (1 << AXP202_IRQ_USBIN) | (1 << AXP202_IRQ_USBRE), true);
    axpxx_clearIRQ();

    /* Determine if USB is connected. */
    b_usb_plugged = axpxx_isVBUSPlug();

    /* Success. */
    return ESP_OK;
  }
  else
  {
    /* AXP202 not detected, we have an error. */
    return ESP_FAIL;
  }
}


/**
 * twatch_pmu_audio_power()
 *
 * Enable or disable power for T-Watch audio circuit (audio DAC)
 **/

esp_err_t twatch_pmu_audio_power(bool enable)
{
  if (axpxx_setLDO3Mode(1) == AXP_PASS)
  {
    if (axpxx_setPowerOutPut(AXP202_LDO3, enable) == AXP_PASS)
      return ESP_OK;
    else
      return ESP_FAIL;
  }
  else
    return ESP_FAIL;
}


/**
 * twatch_pmu_screen_power()
 *
 * Enable or disable power for T-Watch TFT screen.
 **/

esp_err_t twatch_pmu_screen_power(bool enable)
{
  /* Set LDO2 to 3.3V */
  ESP_LOGI("[pmu]","Set LDO2 voltage");
  if (axpxx_setLDO2Voltage(3300) != AXP_PASS)
  {
    return ESP_FAIL;
  }

  /* Enable LDO2 */
  if (axpxx_setPowerOutPut(AXP202_LDO2, (enable?1:0)) != AXP_PASS)
  {
    return ESP_FAIL;
  }

  #ifdef CONFIG_TWATCH_V1
    /* Enable LDO2 */
    if (axpxx_setPowerOutPut(AXP202_LDO2, (enable?1:0)) == AXP_PASS)
      return ESP_OK;
    else
      return ESP_FAIL;
  #else
    #ifdef CONFIG_TWATCH_V2
      if (enable)
      {
        /* Enable LDO3 */
        if (axpxx_setPowerOutPut(AXP202_LDO3, 0) != AXP_PASS)
        {
          ESP_LOGE("[pmu::screen]", "Cannot disable LDO3");
          return ESP_FAIL;
        }

        /* Set LDO3 voltage */
        if (axpxx_setLDO3Voltage(3300) != AXP_PASS)
        {
          ESP_LOGE("[pmu::screen]", "Cannot set LDO3 voltage to 3.3V");
          return ESP_FAIL;
        }
      }

      /* Enable LDO3 */
      if (axpxx_setPowerOutPut(AXP202_LDO3, (enable?1:0)) != AXP_PASS)
      {
        ESP_LOGE("[pmu::screen]", "Cannot enable LDO3");
        return ESP_FAIL;
      }

      twatch_pmu_reset_touchscreen();
    #endif
  #endif

  return ESP_OK;
}


/**
 * twatch_pmu_reset()
 * 
 * @brief Perform an hardware reset on the capacitive touch controller
 **/

void twatch_pmu_reset_touchscreen(void)
{
#ifdef CONFIG_TWATCH_V2
  /* Hardware reset on FT6236. */
  axpxx_setPowerOutPut(AXP202_EXTEN, true);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  axpxx_setPowerOutPut(AXP202_EXTEN, false);
  vTaskDelay(8 / portTICK_PERIOD_MS);
  axpxx_setPowerOutPut(AXP202_EXTEN, true);
#endif
}


/**
 * twatch_pmu_power()
 *
 * Enable/disable power for whole watch.
 **/

esp_err_t twatch_pmu_power(bool enable)
{
  if (enable)
  {
    /**
     * Initialize AXP202, based on official TTGO Arduino lib.
     * By default, audio circuit is off.
     **/

    AXP_CHECK(axpxx_setShutdownTime(AXP_POWER_OFF_TIME_4S));
    AXP_CHECK(axpxx_setChgLEDMode(AXP20X_LED_OFF));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_EXTEN, false));
    AXP_CHECK(axpxx_setChargeControlCur(300));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO2, AXP202_ON));
  }
  else
  {
    /* Shut power down, based on official TTGO Arduino lib. */
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_EXTEN, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO4, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_DCDC2, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO3, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO2, false));
  }

  /* Success. */
  return ESP_OK;
}

/**
 * twatch_pmu_read_irq()
 * 
 * Fetch IRQ data from AXP202.
 * 
 **/

void twatch_pmu_read_irq(void)
{
  if (b_axpxx_irq_triggered)
  {
    /* Read IRQ. */
    if (axpxx_readIRQ() == AXP_PASS)
    {
      /* Check if user button has been pressed. */
      if (axpxx_isPEKShortPressIRQ())
      {
        userbtn_int_count = 1;
      }
      if (axpxx_isVbusPlugInIRQ())
      {
        b_usb_plugged = true;
      }
      if (axpxx_isVbusRemoveIRQ())
      {
        b_usb_plugged = false;
      }
    }

    /* Clear IRQ. */
    axpxx_clearIRQ();
    b_axpxx_irq_triggered = false;
  }
}

/**
 * twatch_pmu_is_userbtn_pressed(void)
 *
 * Determines if the user button has been pressed.
 *
 * @return true if user button has been pressed, false otherwise.
 **/

bool twatch_pmu_is_userbtn_pressed(void)
{
  bool result = false;

  /* Fetch IRQ data if there is any. */
  twatch_pmu_read_irq();

  /* If user button has been (short) pressed, return true. */
  if (userbtn_int_count > 0)
  {
    result = true;
    
    /* Reset interrupt counter. */
    userbtn_int_count = 0;    
  }

  return result;
}

/**
 * twatch_pmu_is_usb_plugged()
 * 
 * Determines if a USB connector is plugged in and charging.
 * 
 * @return: true if usb connector is plugged in, false otherwise.
 **/

bool twatch_pmu_is_usb_plugged(bool b_query_irq)
{
  /* Read IRQ if required. */
  if (b_query_irq)
    twatch_pmu_read_irq();

  /* Return usb state. */
  return b_usb_plugged;
}


/**
 * twatch_pmu_deepsleep()
 *
 * Enable deep sleep. Configure GPIO 35 (AXP202 IRQ) to wake up
 * the chip.
 **/

void twatch_pmu_deepsleep(void)
{
  /* Shutdown screen. */
  twatch_pmu_screen_power(false);
  
  #ifdef CONFIG_TWATCH_V1
  twatch_pmu_audio_power(false);
  #endif

  /* Set GPIO 35 as wakeup signal. */
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);

  /* Go into deep sleep mode. */
  esp_deep_sleep_start();
}


/**
 * twatch_pmu_get_battery_level()
 * 
 * Get battery percentage from AXP202.
 * @return: percentage (0-100)
 **/

int twatch_pmu_get_battery_level(void)
{
  float voltage;
  int level=-1;

  if (axpxx_isChargeing())
  {
    /* Read level, if level == 0x7f then percentage cannot be trusted. */
    level = axpxx_getBattPercentage();
    if (level == 0x7F)
      level = -1;
  }

  if (level < 0)
  {
    voltage = axpxx_getBattVoltage();
    level = ((voltage - 3200)*100)/1000;
    if (level < 0)
      level = 0;
    if (level > 100)
      level = 100;
  }
  
  return level;
}


/**
 * twatch_pmu_vibration()
 * 
 * @brief Set DRV2605L EN pin to high (enabled) or low (disabled)
 * @param enable: enable DRV2605L
 * @return: ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t twatch_pmu_vibration(bool enable)
{
  /* Only for T-Watch 2020 v2. */
  #ifdef CONFIG_TWATCH_V2
    return (axpxx_setGPIOMode(
      AXP_GPIO_0,
      enable ? AXP_IO_OUTPUT_HIGH_MODE : AXP_IO_OUTPUT_LOW_MODE
    ) == AXP_PASS);
  #endif
}


esp_err_t twatch_pmu_gps_power(bool enable)
{
  #ifdef CONFIG_TWATCH_V2
    /* Configure LOD4 for GPS power supply (3.3V). */
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO4, AXP202_OFF));
    axpxx_setLDO4Voltage(AXP202_LDO4_3300MV);
    
    if (enable)
    {
      AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO4, AXP202_ON));
    }
  #endif

  /* Success. */
  return ESP_OK;
}