#include "drivers/ft6236.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "drivers/i2c.h"

#define TAG "FT6X36"

bool ft6236_initialized = false;
uint8_t current_dev_addr;
FT6X36_IRQ_HANDLER ft6236_irq_handler = NULL;

/**
 * _touch_interrupt_handler()
 *
 * Called when the screen is touched, and notify a waiting call to
 * ft6x36_read_touch_data() that it should fetch the report.
 **/

void IRAM_ATTR _touch_interrupt_handler(void *parameter)
{
  ft6236_touch_t touch_data;

  /* Forward to our IRQ Handler. */
  if (ft6236_irq_handler != NULL)
  {
    /* Call our callback with the read touch points. */
    ft6236_irq_handler();
  }
}

/**
 * ft6x06_i2c_read8()
 *
 * Read a 8-bit value from I2C register.
 *
 * @param slave_addr: I2C slave address
 * @param register_addr: register address
 * @param data_buf: pointer to a byte buffer
 * @retval  ESP_OK on success, ESP_FAIL otherwise.
 **/

esp_err_t ft6x06_i2c_read8(uint8_t slave_addr, uint8_t register_addr, uint8_t *data_buf) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, register_addr, I2C_MASTER_ACK);

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (slave_addr << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(i2c_cmd, data_buf, I2C_MASTER_NACK);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = twatch_i2c_master_cmd_begin(I2C_SEC, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    return ret;
}

esp_err_t ft6x06_i2c_read(uint8_t slave_addr, uint8_t register_addr, uint8_t *data_buf, int length) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, register_addr, I2C_MASTER_ACK);

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (slave_addr << 1) | I2C_MASTER_READ, true);

    if (length > 1)
    {
        i2c_master_read(i2c_cmd, data_buf, length-1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(i2c_cmd, data_buf+length-1, I2C_MASTER_NACK);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = twatch_i2c_master_cmd_begin(I2C_SEC, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    return ret;
}


esp_err_t ft6x06_i2c_write8(uint8_t slave_addr, uint8_t register_addr, uint8_t data_buf) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, register_addr, I2C_MASTER_ACK);
    i2c_master_write_byte(i2c_cmd, data_buf, I2C_MASTER_ACK);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = twatch_i2c_master_cmd_begin(I2C_SEC, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    return ret;
}

/**
  * @brief  Initialize for FT6x36 communication via I2C
  * @param  dev_addr: Device address on communication Bus (I2C slave address of FT6X36).
  * @retval None
  */
void ft6x36_init(uint16_t dev_addr, FT6X36_IRQ_HANDLER pfn_handler) {
  gpio_config_t irq_conf;

  /* Initialize FT6x36 IRQ pin as input pin. */
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;
  irq_conf.pin_bit_mask = (1ULL << 38);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_down_en = 0;
  irq_conf.pull_up_en = 1;
  gpio_config(&irq_conf);

  /* Install our user button interrupt handler. */
  if (gpio_install_isr_service(0) != ESP_OK)
    printf("[isr2] Error while installing service\r\n");
  gpio_isr_handler_add(GPIO_NUM_38, _touch_interrupt_handler, NULL);

  /* Save IRQ Handler. */
  ft6236_irq_handler = pfn_handler;

  if (!ft6236_initialized) {
    /* Make sure I2C is configured and ready. */
    twatch_i2c_init();
    ft6236_initialized = true;
    current_dev_addr = dev_addr;
    uint8_t data_buf;
    esp_err_t ret;
    ESP_LOGI(TAG, "Found touch panel controller");
    if ((ret = ft6x06_i2c_read8(dev_addr, FT6X36_PANEL_ID_REG, &data_buf) != ESP_OK))
        ESP_LOGE(TAG, "Error reading from device: %s",
                 esp_err_to_name(ret));    // Only show error the first time
    ESP_LOGI(TAG, "\tDevice ID: 0x%02x", data_buf);

    ft6x06_i2c_read8(dev_addr, FT6X36_CHIPSELECT_REG, &data_buf);
    ESP_LOGI(TAG, "\tChip ID: 0x%02x", data_buf);

    ft6x06_i2c_read8(dev_addr, FT6X36_DEV_MODE_REG, &data_buf);
    ESP_LOGI(TAG, "\tDevice mode: 0x%02x", data_buf);

    ft6x06_i2c_read8(dev_addr, FT6X36_FIRMWARE_ID_REG, &data_buf);
    ESP_LOGI(TAG, "\tFirmware ID: 0x%02x", data_buf);

    ft6x06_i2c_read8(dev_addr, FT6X36_RELEASECODE_REG, &data_buf);
    ESP_LOGI(TAG, "\tRelease code: 0x%02x", data_buf);

    ft6x06_i2c_read8(dev_addr, FT6X36_OPMODE_REG, &data_buf);
    ESP_LOGI(TAG, "\tOperating mode: 0x%02x", data_buf);

    /*
    ft6x36_enable_active_mode();
    ft6x36_set_active_period(0xff);
    ft6x36_set_touch_threshold(15);
    */
  }
}

bool ft6x36_read(ft6236_touch_t *touch) {
  uint8_t report[FT6X36_REPORT_SIZE];

  /* Read report (optimized, report read in one I2C transaction). */
  ft6x06_i2c_read(current_dev_addr, FT6X36_DEV_MODE_REG, report, FT6X36_REPORT_SIZE);

  /* Copy gesture id. */
  touch->gest_id = report[FT6X36_GEST_ID_REG];

  /* And number of touchpoints. */
  touch->tp_count = report[FT6X36_TD_STAT_REG] & 0x0f;

  /* Do we have at least one touch point ? */
  if (touch->tp_count > 0)
  {
    /* Update first touchpoint. */
    touch->touches[0].x = ((report[FT6X36_P1_XH_REG] & FT6X36_MSB_MASK) << 8) | report[FT6X36_P1_XL_REG];
    touch->touches[0].y = ((report[FT6X36_P1_YH_REG] & FT6X36_MSB_MASK) << 8) | report[FT6X36_P1_YL_REG];
    touch->touches[0].weight = 0;
    touch->touches[0].event = report[FT6X36_P1_XH_REG] >> 6;

    if (touch->tp_count > 1)
    {
      /* Update second touchpoint. */
      touch->touches[1].x = ((report[FT6X36_P2_XH_REG] & FT6X36_MSB_MASK) << 8) | report[FT6X36_P2_XL_REG];
      touch->touches[1].y = ((report[FT6X36_P2_YH_REG] & FT6X36_MSB_MASK) << 8) | report[FT6X36_P2_YL_REG];
      touch->touches[1].weight = 0;
      touch->touches[1].event = report[FT6X36_P2_XH_REG] >> 6;
    }
    else
    {
      /* Fill second touchpoint with zeroes. */
      memset(&touch->touches[1], 0, sizeof(ft6236_touchpoint_t));
    }

    return true;
  }
  else
  {
    /* Fill both touchpoints with zeroes. */
    memset(&touch->touches, 0, 2*sizeof(ft6236_touchpoint_t));

    if (touch->gest_id != FT6X36_GEST_ID_NO_GESTURE)
      return true;
  }

  // Read X value
  return false;
}


/**
 * @brief Enable active mode
 * @retval ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t ft6x36_enable_active_mode(void)
{
  uint8_t mode = FT6X36_CTRL_KEEP_ACTIVE_MODE;
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_CTRL_REG, mode);
}


/**
 * @brief Enable monitor mode
 * @retval ESP_OK on success, ESP_FAIL otherwise
 **/

esp_err_t ft6x36_enable_monitor_mode(void)
{
  uint8_t mode = FT6X36_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE;
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_CTRL_REG, mode);
}

esp_err_t ft6x36_set_touch_threshold(uint8_t threshold)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_TH_GROUP_REG, threshold);
}

esp_err_t ft6x36_set_active_period(uint8_t period)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_PERIOD_ACTIVE_REG, period);
}

esp_err_t ft6x36_set_max_offset_move_lr(uint8_t offset)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_OFFSET_LEFT_RIGHT_REG, offset);
}

esp_err_t ft6x36_get_max_offset_move_lr(uint8_t *offset)
{
  return ft6x06_i2c_read8(current_dev_addr, FT6X36_OFFSET_LEFT_RIGHT_REG, offset);
}


esp_err_t ft6x36_set_max_offset_move_ud(uint8_t offset)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_OFFSET_UP_DOWN_REG, offset);
}

esp_err_t ft6x36_get_max_offset_move_ud(uint8_t *offset)
{
  return ft6x06_i2c_read8(current_dev_addr, FT6X36_OFFSET_UP_DOWN_REG, offset);
}


esp_err_t ft6x36_set_min_distance_move_lr(uint8_t distance)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_DISTANCE_LEFT_RIGHT_REG, distance);
}

esp_err_t ft6x36_get_min_distance_move_lr(uint8_t *distance)
{
  return ft6x06_i2c_read8(current_dev_addr, FT6X36_DISTANCE_LEFT_RIGHT_REG, distance);
}


esp_err_t ft6x36_set_min_distance_move_ud(uint8_t distance)
{
  return ft6x06_i2c_write8(current_dev_addr, FT6X36_DISTANCE_UP_DOWN_REG, distance);
}

esp_err_t ft6x36_get_min_distance_move_ud(uint8_t *distance)
{
  return ft6x06_i2c_read8(current_dev_addr, FT6X36_DISTANCE_LEFT_RIGHT_REG, distance);
}
