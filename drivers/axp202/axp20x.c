/////////////////////////////////////////////////////////////////
/*

    pure C port por ESP-IDF - @tixlegeek - tixlegeek.io
    10/08/20 - https://github.com/tixlegeek/C_AXP202X_Library
    Original Header:

--------------------------------------------------------------------------------

MIT License

Copyright (c) 2019 lewis he

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

axp20x.cpp - Arduino library for X-Power AXP202 chip.
Created by Lewis he on April 1, 2019.
github:https://github.com/lewisxhe/AXP202X_Libraries
*/
/////////////////////////////////////////////////////////////////

#include "drivers/axp20x.h"
#include <math.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "AXPXX"
uint8_t axpxx_irq[5];
uint8_t axpxx_chip_id;
bool axpxx_init = false;

const uint8_t axpxx_startupParams[] = {
  0b00000000,
  0b01000000,
  0b10000000,
  0b11000000
};

const uint8_t axpxx_longPressParams[] = {
  0b00000000,
  0b00010000,
  0b00100000,
  0b00110000
};

const uint8_t axpxx_shutdownParams[] = {
  0b00000000,
  0b00000001,
  0b00000010,
  0b00000011
};

const uint8_t axpxx_targetVolParams[] = {
  0b00000000,
  0b00100000,
  0b01000000,
  0b01100000
};

uint16_t _getRegistH8L5(uint8_t regh8, uint8_t regl5)
{
  uint8_t hv, lv;
  axpxx_readByte(regh8, 1, &hv);
  axpxx_readByte(regl5, 1, &lv);
  return (hv << 5) | (lv & 0x1F);
}

uint16_t _getRegistResult(uint8_t regh8, uint8_t regl4)
{
  uint8_t hv, lv;
  axpxx_readByte(regh8, 1, &hv);
  axpxx_readByte(regl4, 1, &lv);
  return (hv << 4) | (lv & 0x0F);
}

void axpxx_i2c_init()
{
  /*
  i2c_config_t i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = AXP202_SDA_PIN,
    .scl_io_num = AXP202_SCL_PIN,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };
  esp_err_t res;

  ESP_LOGD(TAG, "Setting up I2C");
  res = i2c_param_config(I2C_NUM_0, &i2c_config);
  assert(res == ESP_OK);
  res = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
  assert(res == ESP_OK);
  */
  twatch_i2c_init();
}

void axpxx_readByte(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
  esp_err_t espErr;
  i2c_cmd_handle_t cmd;

  cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (AXP202_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (AXP202_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, data , I2C_MASTER_NACK);
  i2c_master_stop(cmd);

  //espErr = i2c_master_cmd_begin(AXP202_I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
  espErr = twatch_i2c_master_cmd_begin(I2C_PRI, cmd, 10/portTICK_PERIOD_MS);
  assert(espErr==ESP_OK);
  i2c_cmd_link_delete(cmd);
}

void axpxx_writeByte(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
  esp_err_t espErr;
  i2c_cmd_handle_t cmd;
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (AXP202_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  //  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, *data, true);
  i2c_master_stop(cmd);
  //espErr = i2c_master_cmd_begin(AXP202_I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
  espErr = twatch_i2c_master_cmd_begin(I2C_PRI, cmd, 10/portTICK_PERIOD_MS);
  assert(espErr==ESP_OK);
  i2c_cmd_link_delete(cmd);
}

// Power Output Control register
uint8_t axpxx_outputReg;

int axpxx_probe_chip(void)
{
  uint8_t data;
  if (IS_AXP173) {
    //!Axp173 does not have a chip ID, read the status register to see if it reads normally
    axpxx_readByte(0x01, 1, &data);
    if (data == 0 || data == 0xFF) {
      return AXP_FAIL;
    }
    axpxx_chip_id = AXP173_CHIP_ID;
    axpxx_readByte(AXP202_LDO234_DC23_CTL, 1, &axpxx_outputReg);
    AXP_DEBUG("OUTPUT Register 0x%x\n", axpxx_outputReg);
    axpxx_init = true;
    return AXP_PASS;
  }
  axpxx_readByte(AXP202_IC_TYPE, 1, &axpxx_chip_id);
  AXP_DEBUG("chip id detect 0x%x\n", axpxx_chip_id);
  if (axpxx_chip_id == AXP202_CHIP_ID || axpxx_chip_id == AXP192_CHIP_ID) {
    AXP_DEBUG("Detect CHIP :%s\n", axpxx_chip_id == AXP202_CHIP_ID ? "AXP202" : "AXP192");
    axpxx_readByte(AXP202_LDO234_DC23_CTL, 1, &axpxx_outputReg);
    AXP_DEBUG("OUTPUT Register 0x%x\n", axpxx_outputReg);
    axpxx_init = true;
    return AXP_PASS;
  }
  return AXP_FAIL;
}
/*
int axpxx_begin(TwoWire &port, uint8_t addr, bool isAxp173)
{
_i2cPort = &port; //Grab which port the user wants us to use
_address = addr;
IS_AXP173 = isAxp173;

return _axp_probe();
}*/
/*
int axpxx_begin(axp_com_fptr_t read_cb, axp_com_fptr_t write_cb, uint8_t addr, bool isAxp173)
{
if (read_cb == nullptr || write_cb == nullptr)return AXP_FAIL;
_read_cb = read_cb;
_write_cb = write_cb;
_address = addr;
IS_AXP173 = isAxp173;
return _axp_probe();
}
*/
//Only axp192 chip
bool axpxx_isDCDC1Enable()
{
  if (axpxx_chip_id == AXP192_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP192_DCDC1);
  else if (axpxx_chip_id == AXP173_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP173_DCDC1);
  return false;
}

bool axpxx_isExtenEnable()
{
  if (axpxx_chip_id == AXP192_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP192_EXTEN);
  else if (axpxx_chip_id == AXP202_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP202_EXTEN);
  else if (axpxx_chip_id == AXP173_CHIP_ID) {
    uint8_t data;
    axpxx_readByte(AXP173_EXTEN_DC2_CTL, 1, &data);
    return IS_OPEN(data, AXP173_CTL_EXTEN_BIT);
  }
  return false;
}

bool axpxx_isLDO2Enable()
{
  if (axpxx_chip_id == AXP173_CHIP_ID) {
    return IS_OPEN(axpxx_outputReg, AXP173_LDO2);
  }
  //axp192 same axp202 ldo2 bit
  return IS_OPEN(axpxx_outputReg, AXP202_LDO2);
}

bool axpxx_isLDO3Enable()
{
  if (axpxx_chip_id == AXP192_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP192_LDO3);
  else if (axpxx_chip_id == AXP202_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP202_LDO3);
  else if (axpxx_chip_id == AXP173_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP173_LDO3);
  return false;
}

bool axpxx_isLDO4Enable()
{
  if (axpxx_chip_id == AXP202_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP202_LDO4);
  if (axpxx_chip_id == AXP173_CHIP_ID)
  return IS_OPEN(axpxx_outputReg, AXP173_LDO4);
  return false;
}

bool axpxx_isDCDC2Enable()
{
  if (axpxx_chip_id == AXP173_CHIP_ID) {
    uint8_t data;
    axpxx_readByte(AXP173_EXTEN_DC2_CTL, 1, &data);
    return IS_OPEN(data, AXP173_CTL_DC2_BIT);
  }
  //axp192 same axp202 dc2 bit
  return IS_OPEN(axpxx_outputReg, AXP202_DCDC2);
}

bool axpxx_isDCDC3Enable()
{
  if (axpxx_chip_id == AXP173_CHIP_ID)
  return false;
  //axp192 same axp202 dc3 bit
  return IS_OPEN(axpxx_outputReg, AXP202_DCDC3);
}

int axpxx_setPowerOutPut(uint8_t ch, bool en)
{
  uint8_t data;
  uint8_t val = 0;
  if (!axpxx_init)
  return AXP_NOT_INIT;

  //! Axp173 cannot use the REG12H register to control
  //! DC2 and EXTEN. It is necessary to control REG10H separately.
  if (axpxx_chip_id == AXP173_CHIP_ID) {
    axpxx_readByte(AXP173_EXTEN_DC2_CTL, 1, &data);
    if (ch & AXP173_DCDC2) {
      data = en ? data | BIT_MASK(AXP173_CTL_DC2_BIT) : data & (~BIT_MASK(AXP173_CTL_DC2_BIT));
      ch &= (~BIT_MASK(AXP173_DCDC2));
      axpxx_writeByte(AXP173_EXTEN_DC2_CTL, 1, &data);
    } else if (ch & AXP173_EXTEN) {
      data = en ? data | BIT_MASK(AXP173_CTL_EXTEN_BIT) : data & (~BIT_MASK(AXP173_CTL_EXTEN_BIT));
      ch &= (~BIT_MASK(AXP173_EXTEN));
      axpxx_writeByte(AXP173_EXTEN_DC2_CTL, 1, &data);
    }
  }

  axpxx_readByte(AXP202_LDO234_DC23_CTL, 1, &data);
  if (en) {
    data |= (1 << ch);
  } else {
    data &= (~(1 << ch));
  }

  if (axpxx_chip_id == AXP202_CHIP_ID) {
    FORCED_OPEN_DCDC3(data); //! Must be forced open in T-Watch
  }

  axpxx_writeByte(AXP202_LDO234_DC23_CTL, 1, &data);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  axpxx_readByte(AXP202_LDO234_DC23_CTL, 1, &val);
  if (data == val) {
    axpxx_outputReg = val;
    return AXP_PASS;
  }
  return AXP_FAIL;
}

bool axpxx_isChargeing()
{
  uint8_t reg;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_MODE_CHGSTATUS, 1, &reg);
  return IS_OPEN(reg, 6);
}

bool axpxx_isBatteryConnect()
{
  uint8_t reg;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_MODE_CHGSTATUS, 1, &reg);
  return IS_OPEN(reg, 5);
}

float axpxx_getAcinVoltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_ACIN_VOL_H8, AXP202_ACIN_VOL_L4) * AXP202_ACIN_VOLTAGE_STEP;
}

float axpxx_getAcinCurrent()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_ACIN_CUR_H8, AXP202_ACIN_CUR_L4) * AXP202_ACIN_CUR_STEP;
}

float axpxx_getVbusVoltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_VBUS_VOL_H8, AXP202_VBUS_VOL_L4) * AXP202_VBUS_VOLTAGE_STEP;
}

float axpxx_getVbusCurrent()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_VBUS_CUR_H8, AXP202_VBUS_CUR_L4) * AXP202_VBUS_CUR_STEP;
}

float axpxx_getTemp()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_INTERNAL_TEMP_H8, AXP202_INTERNAL_TEMP_L4) * AXP202_INTERNAL_TEMP_STEP;
}

float axpxx_getTSTemp()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_TS_IN_H8, AXP202_TS_IN_L4) * AXP202_TS_PIN_OUT_STEP;
}

float axpxx_getGPIO0Voltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_GPIO0_VOL_ADC_H8, AXP202_GPIO0_VOL_ADC_L4) * AXP202_GPIO0_STEP;
}

float axpxx_getGPIO1Voltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_GPIO1_VOL_ADC_H8, AXP202_GPIO1_VOL_ADC_L4) * AXP202_GPIO1_STEP;
}

/*
Note: the battery power formula:
Pbat =2* register value * Voltage LSB * Current LSB / 1000.
(Voltage LSB is 1.1mV; Current LSB is 0.5mA, and unit of calculation result is mW.)
*/
float axpxx_getBattInpower()
{
  float rslt;
  uint8_t hv, mv, lv;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_BAT_POWERH8, 1, &hv);
  axpxx_readByte(AXP202_BAT_POWERM8, 1, &mv);
  axpxx_readByte(AXP202_BAT_POWERL8, 1, &lv);
  rslt = (hv << 16) | (mv << 8) | lv;
  rslt = 2 * rslt * 1.1 * 0.5 / 1000;
  return rslt;
}

float axpxx_getBattVoltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_BAT_AVERVOL_H8, AXP202_BAT_AVERVOL_L4) * AXP202_BATT_VOLTAGE_STEP;
}

float axpxx_getBattChargeCurrent()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    return _getRegistResult(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L4) * AXP202_BATT_CHARGE_CUR_STEP;
    case AXP192_CHIP_ID:
    return _getRegistH8L5(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L5) * AXP202_BATT_CHARGE_CUR_STEP;
    default:
    return AXP_FAIL;
  }
}

float axpxx_getBattDischargeCurrent()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistH8L5(AXP202_BAT_AVERDISCHGCUR_H8, AXP202_BAT_AVERDISCHGCUR_L5) * AXP202_BATT_DISCHARGE_CUR_STEP;
}

float axpxx_getSysIPSOUTVoltage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  return _getRegistResult(AXP202_APS_AVERVOL_H8, AXP202_APS_AVERVOL_L4);
}

/*
Coulomb calculation formula:
C= 65536 * current LSB *（charge coulomb counter value - discharge coulomb counter value） /
3600 / ADC sample rate. Refer to REG84H setting for ADC sample rate；the current LSB is
0.5mA；unit of the calculation result is mAh. ）
*/
uint32_t axpxx_getBattChargeCoulomb()
{
  uint8_t buffer[4];
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(0xB0, 4, buffer);
  return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

uint32_t axpxx_getBattDischargeCoulomb()
{
  uint8_t buffer[4];
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(0xB4, 4, buffer);
  return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

float axpxx_getCoulombData()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint32_t charge = axpxx_getBattChargeCoulomb(), discharge = axpxx_getBattDischargeCoulomb();
  uint8_t rate = axpxx_getAdcSamplingRate();
  float result = 65536.0 * 0.5 * (charge - discharge) / 3600.0 / rate;
  return result;
}


//-------------------------------------------------------
// New Coulomb functions  by MrFlexi
//-------------------------------------------------------

uint8_t axpxx_getCoulombRegister()
{
  uint8_t buffer;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_COULOMB_CTL, 1, &buffer);
  return buffer;
}


int axpxx_setCoulombRegister(uint8_t val)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_writeByte(AXP202_COULOMB_CTL, 1, &val);
  return AXP_PASS;
}


int axpxx_EnableCoulombcounter(void)
{

  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val = 0x80;
  axpxx_writeByte(AXP202_COULOMB_CTL, 1, &val);
  return AXP_PASS;
}

int axpxx_DisableCoulombcounter(void)
{

  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val = 0x00;
  axpxx_writeByte(AXP202_COULOMB_CTL, 1, &val);
  return AXP_PASS;
}

int axpxx_StopCoulombcounter(void)
{

  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val = 0xB8;
  axpxx_writeByte(AXP202_COULOMB_CTL, 1, &val);
  return AXP_PASS;
}


int axpxx_ClearCoulombcounter(void)
{

  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val = 0xA0;
  axpxx_writeByte(AXP202_COULOMB_CTL, 1, &val);
  return AXP_PASS;
}

//-------------------------------------------------------
// END
//-------------------------------------------------------



uint8_t axpxx_getAdcSamplingRate()
{
  //axp192 same axp202 aregister address 0x84
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_SPEED, 1, &val);
  return 25 * (int)pow(2, (val & 0xC0) >> 6);
}

int axpxx_setAdcSamplingRate(axp_adc_sampling_rate_t rate)
{
  //axp192 same axp202 aregister address 0x84
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (rate > AXP_ADC_SAMPLING_RATE_200HZ)
  return AXP_FAIL;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_SPEED, 1, &val);
  uint8_t rw = rate;
  val &= 0x3F;
  val |= (rw << 6);
  axpxx_writeByte(AXP202_ADC_SPEED, 1, &val);
  return AXP_PASS;
}

int axpxx_setTSfunction(axp_ts_pin_function_t func)
{
  //axp192 same axp202 aregister address 0x84
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (func > AXP_TS_PIN_FUNCTION_ADC)
  return AXP_FAIL;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_SPEED, 1, &val);
  uint8_t rw = func;
  val &= 0xFA;
  val |= (rw << 2);
  axpxx_writeByte(AXP202_ADC_SPEED, 1, &val);
  return AXP_PASS;
}

int axpxx_setTScurrent(axp_ts_pin_current_t current)
{
  //axp192 same axp202 aregister address 0x84
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (current > AXP_TS_PIN_CURRENT_80UA)
  return AXP_FAIL;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_SPEED, 1, &val);
  uint8_t rw = current;
  val &= 0xCF;
  val |= (rw << 4);
  axpxx_writeByte(AXP202_ADC_SPEED, 1, &val);
  return AXP_PASS;
}

int axpxx_setTSmode(axp_ts_pin_mode_t mode)
{
  //axp192 same axp202 aregister address 0x84
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (mode > AXP_TS_PIN_MODE_ENABLE)
  return AXP_FAIL;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_SPEED, 1, &val);
  uint8_t rw = mode;
  val &= 0xFC;
  val |= rw;
  axpxx_writeByte(AXP202_ADC_SPEED, 1, &val);

  // TS pin ADC function enable/disable
  if (mode == AXP_TS_PIN_MODE_DISABLE)
  axpxx_adc1Enable(AXP202_TS_PIN_ADC1, false);
  else
  axpxx_adc1Enable(AXP202_TS_PIN_ADC1, true);
  return AXP_PASS;
}

int axpxx_adc1Enable(uint16_t params, bool en)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_EN1, 1, &val);
  if (en)
  val |= params;
  else
  val &= ~(params);
  axpxx_writeByte(AXP202_ADC_EN1, 1, &val);
  return AXP_PASS;
}

int axpxx_adc2Enable(uint16_t params, bool en)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val;
  axpxx_readByte(AXP202_ADC_EN2, 1, &val);
  if (en)
  val |= params;
  else
  val &= ~(params);
  axpxx_writeByte(AXP202_ADC_EN2, 1, &val);
  return AXP_PASS;
}

int axpxx_enableIRQ(uint64_t params, bool en)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val, val1;
  if (params & 0xFF) {
    val1 = params & 0xFF;
    axpxx_readByte(AXP202_INTEN1, 1, &val);
    if (en)
    val |= val1;
    else
    val &= ~(val1);
    AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN1, val);
    axpxx_writeByte(AXP202_INTEN1, 1, &val);
  }
  if (params & 0xFF00) {
    val1 = params >> 8;
    axpxx_readByte(AXP202_INTEN2, 1, &val);
    if (en)
    val |= val1;
    else
    val &= ~(val1);
    AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN2, val);
    axpxx_writeByte(AXP202_INTEN2, 1, &val);
  }

  if (params & 0xFF0000) {
    val1 = params >> 16;
    axpxx_readByte(AXP202_INTEN3, 1, &val);
    if (en)
    val |= val1;
    else
    val &= ~(val1);
    AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN3, val);
    axpxx_writeByte(AXP202_INTEN3, 1, &val);
  }

  if (params & 0xFF000000) {
    val1 = params >> 24;
    axpxx_readByte(AXP202_INTEN4, 1, &val);
    if (en)
    val |= val1;
    else
    val &= ~(val1);
    AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN4, val);
    axpxx_writeByte(AXP202_INTEN4, 1, &val);
  }

  if (params & 0xFF00000000) {
    val1 = params >> 32;
    axpxx_readByte(AXP202_INTEN5, 1, &val);
    if (en)
    val |= val1;
    else
    val &= ~(val1);
    AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN5, val);
    axpxx_writeByte(AXP202_INTEN5, 1, &val);
  }
  return AXP_PASS;
}

int axpxx_readIRQ()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP192_CHIP_ID:
    for (int i = 0; i < 4; ++i) {
      axpxx_readByte(AXP192_INTSTS1 + i, 1, &axpxx_irq[i]);
    }
    axpxx_readByte(AXP192_INTSTS5, 1, &axpxx_irq[4]);
    return AXP_PASS;

    case AXP202_CHIP_ID:
    for (int i = 0; i < 5; ++i) {
      axpxx_readByte(AXP202_INTSTS1 + i, 1, &axpxx_irq[i]);
    }
    return AXP_PASS;
    default:
    return AXP_FAIL;
  }
}

void axpxx_clearIRQ()
{
  uint8_t val = 0xFF;
  switch (axpxx_chip_id) {
    case AXP192_CHIP_ID:
    for (int i = 0; i < 3; i++) {
      axpxx_writeByte(AXP192_INTSTS1 + i, 1, &val);
    }
    axpxx_writeByte(AXP192_INTSTS5, 1, &val);
    break;
    case AXP202_CHIP_ID:
    for (int i = 0; i < 5; i++) {
      axpxx_writeByte(AXP202_INTSTS1 + i, 1, &val);
    }
    break;
    default:
    break;
  }
  memset(axpxx_irq, 0, sizeof(axpxx_irq));
}

bool axpxx_isAcinOverVoltageIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(7));
}

bool axpxx_isAcinPlugInIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(6));
}

bool axpxx_isAcinRemoveIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(5));
}

bool axpxx_isVbusOverVoltageIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(4));
}

bool axpxx_isVbusPlugInIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(3));
}

bool axpxx_isVbusRemoveIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(2));
}

bool axpxx_isVbusLowVHOLDIRQ()
{
  return (bool)(axpxx_irq[0] & BIT_MASK(1));
}

bool axpxx_isBattPlugInIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(7));
}
bool axpxx_isBattRemoveIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(6));
}
bool axpxx_isBattEnterActivateIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(5));
}
bool axpxx_isBattExitActivateIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(4));
}
bool axpxx_isChargingIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(3));
}
bool axpxx_isChargingDoneIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(2));
}
bool axpxx_isBattTempLowIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(1));
}
bool axpxx_isBattTempHighIRQ()
{
  return (bool)(axpxx_irq[1] & BIT_MASK(0));
}

bool axpxx_isPEKShortPressIRQ()
{
  return (bool)(axpxx_irq[2] & BIT_MASK(1));
}

bool axpxx_isPEKLongtPressIRQ()
{
  return (bool)(axpxx_irq[2] & BIT_MASK(0));
}

bool axpxx_isTimerTimeoutIRQ()
{
  return (bool)(axpxx_irq[4] & BIT_MASK(7));
}

bool axpxx_isVBUSPlug()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t reg;
  axpxx_readByte(AXP202_STATUS, 1, &reg);
  return IS_OPEN(reg, 5);
}

int axpxx_setDCDC2Voltage(uint16_t mv)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (mv < 700) {
    AXP_DEBUG("DCDC2:Below settable voltage:700mV~2275mV");
    mv = 700;
  }
  if (mv > 2275) {
    AXP_DEBUG("DCDC2:Above settable voltage:700mV~2275mV");
    mv = 2275;
  }
  uint8_t val = (mv - 700) / 25;
  //! axp173/192/202 same register
  axpxx_writeByte(AXP202_DC2OUT_VOL, 1, &val);
  return AXP_PASS;
}

uint16_t axpxx_getDCDC2Voltage()
{
  uint8_t val = 0;
  //! axp173/192/202 same register
  axpxx_readByte(AXP202_DC2OUT_VOL, 1, &val);
  return val * 25 + 700;
}

uint16_t axpxx_getDCDC3Voltage()
{
  if (!axpxx_init)
  return 0;
  if (axpxx_chip_id == AXP173_CHIP_ID)return AXP_NOT_SUPPORT;
  uint8_t val = 0;
  axpxx_readByte(AXP202_DC3OUT_VOL, 1, &val);
  return val * 25 + 700;
}

int axpxx_setDCDC3Voltage(uint16_t mv)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP173_CHIP_ID)return AXP_NOT_SUPPORT;
  if (mv < 700) {
    AXP_DEBUG("DCDC3:Below settable voltage:700mV~3500mV");
    mv = 700;
  }
  if (mv > 3500) {
    AXP_DEBUG("DCDC3:Above settable voltage:700mV~3500mV");
    mv = 3500;
  }
  uint8_t val = (mv - 700) / 25;
  axpxx_writeByte(AXP202_DC3OUT_VOL, 1, &val);
  return AXP_PASS;
}

int axpxx_setLDO2Voltage(uint16_t mv)
{
  uint8_t rVal, wVal;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (mv < 1800) {
    AXP_DEBUG("LDO2:Below settable voltage:1800mV~3300mV");
    mv = 1800;
  }
  if (mv > 3300) {
    AXP_DEBUG("LDO2:Above settable voltage:1800mV~3300mV");
    mv = 3300;
  }
  wVal = (mv - 1800) / 100;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    axpxx_readByte(AXP202_LDO24OUT_VOL, 1, &rVal);
    rVal &= 0x0F;
    rVal |= (wVal << 4);
    axpxx_writeByte(AXP202_LDO24OUT_VOL, 1, &rVal);
    return AXP_PASS;
  } else if (axpxx_chip_id == AXP192_CHIP_ID || axpxx_chip_id == AXP173_CHIP_ID) {
    axpxx_readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    rVal &= 0x0F;
    rVal |= (wVal << 4);
    axpxx_writeByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    return AXP_PASS;
  }
  return AXP_FAIL;
}

uint16_t axpxx_getLDO2Voltage()
{
  uint8_t rVal;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    axpxx_readByte(AXP202_LDO24OUT_VOL, 1, &rVal);
    rVal &= 0xF0;
    rVal >>= 4;
    return rVal * 100 + 1800;
  } else if (axpxx_chip_id == AXP192_CHIP_ID || axpxx_chip_id == AXP173_CHIP_ID ) {
    axpxx_readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    AXP_DEBUG("get result:%x\n", rVal);
    rVal &= 0xF0;
    rVal >>= 4;
    return rVal * 100 + 1800;
  }
  return 0;
}

int axpxx_setLDO3Voltage(uint16_t mv)
{
  uint8_t rVal;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP202_CHIP_ID && mv < 700) {
    AXP_DEBUG("LDO3:Below settable voltage:700mV~3500mV");
    mv = 700;
  } else if (axpxx_chip_id == AXP192_CHIP_ID && mv < 1800) {
    AXP_DEBUG("LDO3:Below settable voltage:1800mV~3300mV");
    mv = 1800;
  }

  if (axpxx_chip_id == AXP202_CHIP_ID && mv > 3500) {
    AXP_DEBUG("LDO3:Above settable voltage:700mV~3500mV");
    mv = 3500;
  } else if (axpxx_chip_id == AXP192_CHIP_ID && mv > 3300) {
    AXP_DEBUG("LDO3:Above settable voltage:1800mV~3300mV");
    mv = 3300;
  }

  if (axpxx_chip_id == AXP202_CHIP_ID) {
    axpxx_readByte(AXP202_LDO3OUT_VOL, 1, &rVal);
    rVal &= 0x80;
    rVal |= ((mv - 700) / 25);
    axpxx_writeByte(AXP202_LDO3OUT_VOL, 1, &rVal);
    return AXP_PASS;
  } else if (axpxx_chip_id == AXP192_CHIP_ID || axpxx_chip_id == AXP173_CHIP_ID) {
    axpxx_readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    rVal &= 0xF0;
    rVal |= ((mv - 1800) / 100);
    axpxx_writeByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    return AXP_PASS;
  }
  return AXP_FAIL;
}

uint16_t axpxx_getLDO3Voltage()
{
  uint8_t rVal;
  if (!axpxx_init)
  return AXP_NOT_INIT;

  if (axpxx_chip_id == AXP202_CHIP_ID) {
    axpxx_readByte(AXP202_LDO3OUT_VOL, 1, &rVal);
    if (rVal & 0x80) {
      //! According to the hardware N_VBUSEN Pin selection
      return axpxx_getVbusVoltage() * 1000;
    } else {
      return (rVal & 0x7F) * 25 + 700;
    }
  } else if (axpxx_chip_id == AXP192_CHIP_ID || axpxx_chip_id == AXP173_CHIP_ID) {
    axpxx_readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
    rVal &= 0x0F;
    return rVal * 100 + 1800;
  }
  return 0;
}

//! Only axp173 support
/*int axpxx_setLDO4Voltage(uint16_t mv)
{
if (!axpxx_init)
return AXP_NOT_INIT;
if (axpxx_chip_id != AXP173_CHIP_ID)
return AXP_FAIL;

if (mv < 700) {
AXP_DEBUG("LDO4:Below settable voltage:700mV~3500mV");
mv = 700;
}
if (mv > 3500) {
AXP_DEBUG("LDO4:Above settable voltage:700mV~3500mV");
mv = 3500;
}
uint8_t val = (mv - 700) / 25;
axpxx_writeByte(AXP173_LDO4_VLOTAGE, 1, &val);
return AXP_PASS;
}*/

uint16_t axpxx_getLDO4Voltage()
{
  const uint16_t ldo4_table[] = {1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2500, 2700, 2800, 3000, 3100, 3200, 3300};
  if (!axpxx_init)
  return 0;
  uint8_t val = 0;
  switch (axpxx_chip_id) {
    case AXP173_CHIP_ID:
    axpxx_readByte(AXP173_LDO4_VLOTAGE, 1, &val);
    return val * 25 + 700;
    case AXP202_CHIP_ID:
    axpxx_readByte(AXP202_LDO24OUT_VOL, 1, &val);
    val &= 0xF;
    return ldo4_table[val];
    break;
    case AXP192_CHIP_ID:
    default:
    break;
  }
  return 0;
}


//! Only axp202 support
int axpxx_setLDO4Voltage(axp_ldo4_table_t param)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    if (param >= AXP202_LDO4_MAX)
    return AXP_INVALID;
    uint8_t val;
    axpxx_readByte(AXP202_LDO24OUT_VOL, 1, &val);
    val &= 0xF0;
    val |= param;
    axpxx_writeByte(AXP202_LDO24OUT_VOL, 1, &val);
    return AXP_PASS;
  }
  return AXP_FAIL;
}

//! Only AXP202 support
// 0 : LDO  1 : DCIN
int axpxx_setLDO3Mode(uint8_t mode)
{
  uint8_t val;
  if (axpxx_chip_id != AXP202_CHIP_ID)
  return AXP_FAIL;
  axpxx_readByte(AXP202_LDO3OUT_VOL, 1, &val);
  if (mode) {
    val |= BIT_MASK(7);
  } else {
    val &= (~BIT_MASK(7));
  }
  axpxx_writeByte(AXP202_LDO3OUT_VOL, 1, &val);
  return AXP_PASS;
}

int axpxx_setStartupTime(uint8_t param)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (param > sizeof(axpxx_startupParams) / sizeof(axpxx_startupParams[0]))
  return AXP_INVALID;
  axpxx_readByte(AXP202_POK_SET, 1, &val);
  val &= (~0b11000000);
  val |= axpxx_startupParams[param];
  axpxx_writeByte(AXP202_POK_SET, 1, &val);
  return AXP_PASS;
}

int axpxx_setlongPressTime(uint8_t param)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (param > sizeof(axpxx_longPressParams) / sizeof(axpxx_longPressParams[0]))
  return AXP_INVALID;
  axpxx_readByte(AXP202_POK_SET, 1, &val);
  val &= (~0b00110000);
  val |= axpxx_longPressParams[param];
  axpxx_writeByte(AXP202_POK_SET, 1, &val);
  return AXP_PASS;
}

int axpxx_setShutdownTime(uint8_t param)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (param > sizeof(axpxx_shutdownParams) / sizeof(axpxx_shutdownParams[0]))
  return AXP_INVALID;
  axpxx_readByte(AXP202_POK_SET, 1, &val);
  val &= (~0b00000011);
  val |= axpxx_shutdownParams[param];
  axpxx_writeByte(AXP202_POK_SET, 1, &val);
  return AXP_PASS;
}

int axpxx_setTimeOutShutdown(bool en)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_POK_SET, 1, &val);
  if (en)
  val |= (1 << 3);
  else
  val &= (~(1 << 3));
  axpxx_writeByte(AXP202_POK_SET, 1, &val);
  return AXP_PASS;
}

int axpxx_shutdown()
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_OFF_CTL, 1, &val);
  val |= (1 << 7);
  axpxx_writeByte(AXP202_OFF_CTL, 1, &val);
  return AXP_PASS;
}

float axpxx_getSettingChargeCurrent()
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_CHARGE1, 1, &val);
  val &= 0b00000111;
  float cur = 300.0 + val * 100.0;
  AXP_DEBUG("Setting Charge current : %.2f mA\n", cur);
  return cur;
}

bool axpxx_isChargeingEnable()
{
  uint8_t val;
  if (!axpxx_init)
  return false;
  axpxx_readByte(AXP202_CHARGE1, 1, &val);
  if (val & (1 << 7)) {
    AXP_DEBUG("Charging enable is enable\n");
    val = true;
  } else {
    AXP_DEBUG("Charging enable is disable\n");
    val = false;
  }
  return val;
}

int axpxx_enableChargeing(bool en)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  axpxx_readByte(AXP202_CHARGE1, 1, &val);
  val |= (1 << 7);
  axpxx_writeByte(AXP202_CHARGE1, 1, &val);
  return AXP_PASS;
}

int axpxx_setChargingTargetVoltage(axp_chargeing_vol_t param)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (param > sizeof(axpxx_targetVolParams) / sizeof(axpxx_targetVolParams[0]))
  return AXP_INVALID;
  axpxx_readByte(AXP202_CHARGE1, 1, &val);
  val &= ~(0b01100000);
  val |= axpxx_targetVolParams[param];
  axpxx_writeByte(AXP202_CHARGE1, 1, &val);
  return AXP_PASS;
}

int axpxx_getBattPercentage()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id != AXP202_CHIP_ID)
  return AXP_NOT_SUPPORT;
  uint8_t val;
  if (!axpxx_isBatteryConnect())
  return 0;
  axpxx_readByte(AXP202_BATT_PERCENTAGE, 1, &val);
  if (!(val & BIT_MASK(7))) {
    return val & (~BIT_MASK(7));
  }
  return 0;
}

int axpxx_setChgLEDMode(axp_chgled_mode_t mode)
{
  uint8_t val;
  axpxx_readByte(AXP202_OFF_CTL, 1, &val);
  val &= 0b11001111;
  val |= BIT_MASK(3);
  switch (mode) {
    case AXP20X_LED_OFF:
    axpxx_writeByte(AXP202_OFF_CTL, 1, &val);
    break;
    case AXP20X_LED_BLINK_1HZ:
    val |= 0b00010000;
    axpxx_writeByte(AXP202_OFF_CTL, 1, &val);
    break;
    case AXP20X_LED_BLINK_4HZ:
    val |= 0b00100000;
    axpxx_writeByte(AXP202_OFF_CTL, 1, &val);
    break;
    case AXP20X_LED_LOW_LEVEL:
    val |= 0b00110000;
    axpxx_writeByte(AXP202_OFF_CTL, 1, &val);
    break;
    default:
    return AXP_FAIL;
  }
  return AXP_PASS;
}

int axpxx_debugCharging()
{
  uint8_t val;
  axpxx_readByte(AXP202_CHARGE1, 1, &val);
  AXP_DEBUG("SRC REG:0x%x\n", val);
  if (val & (1 << 7)) {
    AXP_DEBUG("Charging enable is enable\n");
  } else {
    AXP_DEBUG("Charging enable is disable\n");
  }
  AXP_DEBUG("Charging target-voltage : 0x%x\n", ((val & 0b01100000) >> 5) & 0b11);
  if (val & (1 << 4)) {
    AXP_DEBUG("end when the charge current is lower than 15%% of the set value\n");
  } else {
    AXP_DEBUG(" end when the charge current is lower than 10%% of the set value\n");
  }
  val &= 0b00000111;
  AXP_DEBUG("Charge current : %.2f mA\n", 300.0 + val * 100.0);
  return AXP_PASS;
}

int axpxx_debugStatus()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val, val1, val2;
  axpxx_readByte(AXP202_STATUS, 1, &val);
  axpxx_readByte(AXP202_MODE_CHGSTATUS, 1, &val1);
  axpxx_readByte(AXP202_IPS_SET, 1, &val2);
  AXP_DEBUG("AXP202_STATUS:   AXP202_MODE_CHGSTATUS   AXP202_IPS_SET\n");
  AXP_DEBUG("0x%x\t\t\t 0x%x\t\t\t 0x%x\n", val, val1, val2);
  return AXP_PASS;
}

int axpxx_limitingOff()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  uint8_t val;
  axpxx_readByte(AXP202_IPS_SET, 1, &val);
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    val |= 0x03;
  } else {
    val &= ~(1 << 1);
  }
  axpxx_writeByte(AXP202_IPS_SET, 1, &val);
  return AXP_PASS;
}

// Only AXP129 chip and AXP173
int axpxx_setDCDC1Voltage(uint16_t mv)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id != AXP192_CHIP_ID && axpxx_chip_id != AXP173_CHIP_ID)
  return AXP_FAIL;
  if (mv < 700) {
    AXP_DEBUG("DCDC1:Below settable voltage:700mV~3500mV");
    mv = 700;
  }
  if (mv > 3500) {
    AXP_DEBUG("DCDC1:Above settable voltage:700mV~3500mV");
    mv = 3500;
  }
  uint8_t val = (mv - 700) / 25;
  //! axp192 and axp173 dc1 control register same
  axpxx_writeByte(AXP192_DC1_VLOTAGE, 1, &val);
  return AXP_PASS;
}

// Only AXP129 chip and AXP173
uint16_t axpxx_getDCDC1Voltage()
{
  if (axpxx_chip_id != AXP192_CHIP_ID && axpxx_chip_id != AXP173_CHIP_ID)
  return AXP_FAIL;
  uint8_t val = 0;
  //! axp192 and axp173 dc1 control register same
  axpxx_readByte(AXP192_DC1_VLOTAGE, 1, &val);
  return val * 25 + 700;
}


/***********************************************
*              !!! TIMER FUNCTION !!!
* *********************************************/

int axpxx_setTimer(uint8_t minutes)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    if (minutes > 63) {
      return AXP_ARG_INVALID;
    }
    axpxx_writeByte(AXP202_TIMER_CTL, 1, &minutes);
    return AXP_PASS;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx_offTimer()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    uint8_t minutes = 0x80;
    axpxx_writeByte(AXP202_TIMER_CTL, 1, &minutes);
    return AXP_PASS;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx_clearTimerStatus()
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id == AXP202_CHIP_ID) {
    uint8_t val;
    axpxx_readByte(AXP202_TIMER_CTL, 1, &val);
    val |= 0x80;
    axpxx_writeByte(AXP202_TIMER_CTL, 1, &val);
    return AXP_PASS;
  }
  return AXP_NOT_SUPPORT;
}

/***********************************************
*              !!! GPIO FUNCTION !!!
* *********************************************/

int axpxx__axp192_gpio_0_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
    return 0b101;
    case AXP_IO_INPUT_MODE:
    return 0b001;
    case AXP_IO_LDO_MODE:
    return 0b010;
    case AXP_IO_ADC_MODE:
    return 0b100;
    case AXP_IO_FLOATING_MODE:
    return 0b111;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
    return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
    case AXP_IO_PWM_OUTPUT_MODE:
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp192_gpio_1_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
    return 0b101;
    case AXP_IO_INPUT_MODE:
    return 0b001;
    case AXP_IO_ADC_MODE:
    return 0b100;
    case AXP_IO_FLOATING_MODE:
    return 0b111;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
    return 0;
    case AXP_IO_PWM_OUTPUT_MODE:
    return 0b010;
    case AXP_IO_OUTPUT_HIGH_MODE:
    case AXP_IO_LDO_MODE:
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}


int axpxx__axp192_gpio_3_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_EXTERN_CHARGING_CTRL_MODE:
    return 0;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
    return 1;
    case AXP_IO_INPUT_MODE:
    return 2;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp192_gpio_4_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_EXTERN_CHARGING_CTRL_MODE:
    return 0;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
    return 1;
    case AXP_IO_INPUT_MODE:
    return 2;
    case AXP_IO_ADC_MODE:
    return 3;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}


int axpxx__axp192_gpio_set(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
  int rslt;
  uint8_t val;
  switch (gpio) {
    case AXP_GPIO_0: {
      rslt = _axp192_gpio_0_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP192_GPIO0_CTL, 1, &val);
      val &= 0xF8;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP192_GPIO0_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_1: {
      rslt = _axp192_gpio_1_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP192_GPIO1_CTL, 1, &val);
      val &= 0xF8;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP192_GPIO1_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_2: {
      rslt = _axp192_gpio_1_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP192_GPIO2_CTL, 1, &val);
      val &= 0xF8;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP192_GPIO2_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_3: {
      rslt = _axp192_gpio_3_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP192_GPIO34_CTL, 1, &val);
      val &= 0xFC;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP192_GPIO34_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_4: {
      rslt = _axp192_gpio_4_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP192_GPIO34_CTL, 1, &val);
      val &= 0xF3;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP192_GPIO34_CTL, 1, &val);
      return AXP_PASS;
    }
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp202_gpio_0_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
    return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
    return 1;
    case AXP_IO_INPUT_MODE:
    return 2;
    case AXP_IO_LDO_MODE:
    return 3;
    case AXP_IO_ADC_MODE:
    return 4;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp202_gpio_1_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
    return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
    return 1;
    case AXP_IO_INPUT_MODE:
    return 2;
    case AXP_IO_ADC_MODE:
    return 4;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp202_gpio_2_select( axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
    return 0;
    case AXP_IO_INPUT_MODE:
    return 2;
    case AXP_IO_FLOATING_MODE:
    return 1;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}


int axpxx__axp202_gpio_3_select(axp_gpio_mode_t mode)
{
  switch (mode) {
    case AXP_IO_INPUT_MODE:
    return 1;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
    return 0;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp202_gpio_set(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
  uint8_t val;
  int rslt;
  switch (gpio) {
    case AXP_GPIO_0: {
      rslt = _axp202_gpio_0_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP202_GPIO0_CTL, 1, &val);
      val &= 0b11111000;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP202_GPIO0_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_1: {
      rslt = _axp202_gpio_1_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP202_GPIO1_CTL, 1, &val);
      val &= 0b11111000;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP202_GPIO1_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_2: {
      rslt = _axp202_gpio_2_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP202_GPIO2_CTL, 1, &val);
      val &= 0b11111000;
      val |= (uint8_t)rslt;
      axpxx_writeByte(AXP202_GPIO2_CTL, 1, &val);
      return AXP_PASS;
    }
    case AXP_GPIO_3: {
      rslt = _axp202_gpio_3_select(mode);
      if (rslt < 0)return rslt;
      axpxx_readByte(AXP202_GPIO3_CTL, 1, &val);
      val = rslt ? (val | BIT_MASK(2)) : (val & (~BIT_MASK(2)));
      axpxx_writeByte(AXP202_GPIO3_CTL, 1, &val);
      return AXP_PASS;
    }
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}


int axpxx_setGPIOMode(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    return _axp202_gpio_set(gpio, mode);
    break;
    case AXP192_CHIP_ID:
    return _axp192_gpio_set(gpio, mode);
    break;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}


int axpxx_irq_mask(axp_gpio_irq_t irq)
{
  switch (irq) {
    case AXP_IRQ_NONE:
    return 0;
    case AXP_IRQ_RISING:
    return BIT_MASK(7);
    case AXP_IRQ_FALLING:
    return BIT_MASK(6);
    case AXP_IRQ_DOUBLE_EDGE:
    return 0b1100000;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx__axp202_gpio_irq_set(axp_gpio_t gpio, axp_gpio_irq_t irq)
{
  uint8_t reg;
  uint8_t val;
  int mask;
  mask = _axpxx_irq_mask(irq);

  if (mask < 0)return mask;
  switch (gpio) {
    case AXP_GPIO_0:
    reg = AXP202_GPIO0_CTL;
    break;
    case AXP_GPIO_1:
    reg = AXP202_GPIO1_CTL;
    break;
    case AXP_GPIO_2:
    reg = AXP202_GPIO2_CTL;
    break;
    case AXP_GPIO_3:
    reg = AXP202_GPIO3_CTL;
    break;
    default:
    return AXP_NOT_SUPPORT;
  }
  axpxx_readByte(reg, 1, &val);
  val = mask == 0 ? (val & 0b00111111) : (val | mask);
  axpxx_writeByte(reg, 1, &val);
  return AXP_PASS;
}


int axpxx_setGPIOIrq(axp_gpio_t gpio, axp_gpio_irq_t irq)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    return _axp202_gpio_irq_set(gpio, irq);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
    return AXP_NOT_SUPPORT;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx_setLDO5Voltage(axp_ldo5_table_t vol)
{
  const uint8_t params[] = {
    0b11111000, //1.8V
    0b11111001, //2.5V
    0b11111010, //2.8V
    0b11111011, //3.0V
    0b11111100, //3.1V
    0b11111101, //3.3V
    0b11111110, //3.4V
    0b11111111, //3.5V
  };
  if (!axpxx_init)
  return AXP_NOT_INIT;
  if (axpxx_chip_id != AXP202_CHIP_ID)
  return AXP_NOT_SUPPORT;
  if (vol > sizeof(params) / sizeof(params[0]))
  return AXP_ARG_INVALID;
  uint8_t val = 0;
  axpxx_readByte(AXP202_GPIO0_VOL, 1, &val);
  val &= 0b11111000;
  val |= params[vol];
  axpxx_writeByte(AXP202_GPIO0_VOL, 1, &val);
  return AXP_PASS;
}


int axpxx__axp202_gpio_write(axp_gpio_t gpio, uint8_t val)
{
  uint8_t reg;
  uint8_t wVal = 0;
  switch (gpio) {
    case AXP_GPIO_0:
    reg = AXP202_GPIO0_CTL;
    break;
    case AXP_GPIO_1:
    reg = AXP202_GPIO1_CTL;
    break;
    case AXP_GPIO_2:
    reg = AXP202_GPIO2_CTL;
    if (val) {
      return AXP_NOT_SUPPORT;
    }
    break;
    case AXP_GPIO_3:
    if (val) {
      return AXP_NOT_SUPPORT;
    }
    axpxx_readByte(AXP202_GPIO3_CTL, 1, &wVal);
    wVal &= 0b11111101;
    axpxx_writeByte(AXP202_GPIO3_CTL, 1, &wVal);
    return AXP_PASS;
    default:
    return AXP_NOT_SUPPORT;
  }
  axpxx_readByte(reg, 1, &wVal);
  wVal = val ? (wVal | 1) : (wVal & 0b11111000);
  axpxx_writeByte(reg, 1, &wVal);
  return AXP_PASS;
}

int axpxx__axp202_gpio_read(axp_gpio_t gpio)
{
  uint8_t val;
  uint8_t reg = AXP202_GPIO012_SIGNAL;
  uint8_t offset;
  switch (gpio) {
    case AXP_GPIO_0:
    offset = 4;
    break;
    case AXP_GPIO_1:
    offset = 5;
    break;
    case AXP_GPIO_2:
    offset = 6;
    break;
    case AXP_GPIO_3:
    reg = AXP202_GPIO3_CTL;
    offset = 0;
    break;
    default:
    return AXP_NOT_SUPPORT;
  }
  axpxx_readByte(reg, 1, &val);
  return val & BIT_MASK(offset) ? 1 : 0;
}

int axpxx_gpioWrite(axp_gpio_t gpio, uint8_t val)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    return _axp202_gpio_write(gpio, val);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
    return AXP_NOT_SUPPORT;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx_gpioRead(axp_gpio_t gpio)
{
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    return _axp202_gpio_read(gpio);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
    return AXP_NOT_SUPPORT;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}



int axpxx_getChargeControlCur()
{
  int cur;
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    axpxx_readByte(AXP202_CHARGE1, 1, &val);
    val &= 0x0F;
    cur =  val * 100 + 300;
    if (cur > 1800 || cur < 300)return 0;
    return cur;
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
    axpxx_readByte(AXP202_CHARGE1, 1, &val);
    return val & 0x0F;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}

int axpxx_setChargeControlCur(uint16_t mA)
{
  uint8_t val;
  if (!axpxx_init)
  return AXP_NOT_INIT;
  switch (axpxx_chip_id) {
    case AXP202_CHIP_ID:
    axpxx_readByte(AXP202_CHARGE1, 1, &val);
    val &= 0b11110000;
    mA -= 300;
    val |= (mA / 100);
    axpxx_writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
    axpxx_readByte(AXP202_CHARGE1, 1, &val);
    val &= 0b11110000;
    if(mA > AXP1XX_CHARGE_CUR_1320MA)
    mA = AXP1XX_CHARGE_CUR_1320MA;
    val |= mA;
    axpxx_writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
    default:
    break;
  }
  return AXP_NOT_SUPPORT;
}
