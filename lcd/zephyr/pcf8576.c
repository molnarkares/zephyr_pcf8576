/*
* Copyright (c) 2022 Karoly Molnar
*
* SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_pcf8576

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>

#include "lcd.h"
#include "pcf8576.h"
#include <errno.h>
#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>

#define LOG_LEVEL CONFIG_LCD_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pcf8576_lcd);

#define PCF8576_SLAVE_ADDRESS 0b0111000
#define PCF8576_CMD_CONTINUE 0x80
#define PCF8576_CMD_LAST 0x00

#define PCF8576_CMD_MODE_SET 0b01000000
#define PCF8576_CMD_LOAD_DP 0b00000000
#define PCF8576_CMD_DEVICE_SELECT 0b01100000

#define DIGIT_BLANK (30)
#define DIGIT_DP (20)
#define DIGIT_NEG (10)
#define SEGMENT_NONE (40) >> 1


struct pcf8576_cfg {
  struct i2c_dt_spec i2c;
};

struct pcf8576_data {
  uint8_t device_select;
  uint8_t display_ram[20];
  int lock_ctr;
};

static const uint8_t segment_data[11] = {
    0b1111110, // 0
    0b0110000, // 1
    0b1101101, // 2
    0b1111001, // 3
    0b0110011, // 4
    0b1011011, // 5
    0b1011111, // 6
    0b1110000, // 7
    0b1111111, // 8
    0b1111011, // 9
    0b0000001  // DIGIT_NEG
};

static void _pcf8576_set(const struct device *dev, const uint8_t data[2]);
static void _pcf8576_clear(const struct device *dev, const uint8_t data[2]);
static size_t _pcf8576_count_int_digits(int32_t number);
static size_t _pcf8576_count_frac_digits(float number, size_t max_digits, uint32_t *fr_part);

static void _pcf8576_int_to_digits(int32_t val, uint8_t digits[], size_t no_digits);

void _pcf8576_set_digit(const struct device *dev, const uint8_t segment[][2],
                        uint8_t value) {
  int idx = 7;
  if (value == DIGIT_BLANK) {
    do {
      _pcf8576_clear(dev, segment[--idx]);
    } while (idx > 0);
  } else {
    if (segment[7][1] < SEGMENT_NONE) {
      if (value >= DIGIT_DP) {
        _pcf8576_set(dev, segment[7]);
        value -= DIGIT_DP;
      } else {
        _pcf8576_clear(dev, segment[7]);
      }
    }
    idx = 7;
    uint8_t tmp_digit = segment_data[value];
    do {
      if (tmp_digit & 1) {
        _pcf8576_set(dev, segment[--idx]);
      } else {
        _pcf8576_clear(dev, segment[--idx]);
      }
      tmp_digit >>= 1;
    } while (idx > 0);
  }
}

void pcf8576_flush(const struct device *dev) {
  const struct pcf8576_cfg *cfg = dev->config;
  struct pcf8576_data *data = dev->data;
  uint8_t sub_address = (DT_INST_PROP(0, sub_address) & 0x07);

  uint8_t cmd = PCF8576_CMD_CONTINUE | PCF8576_CMD_LOAD_DP | 0;

  data->device_select =
      PCF8576_CMD_LAST | PCF8576_CMD_DEVICE_SELECT | sub_address;

  if (i2c_burst_write_dt(&cfg->i2c, cmd, &data->device_select,
                         sizeof(data->display_ram) + 1)) {
    LOG_ERR("Writing to PCF8576 device @%d on bus %s has failed", cfg->i2c.addr,
            cfg->i2c.bus->name);
  }
  LOG_HEXDUMP_DBG(data->display_ram, sizeof(data->display_ram), "display_ram");
}

static int pcf8576_initialize(const struct device *dev) {
  LOG_DBG("initializing...");
  const struct pcf8576_cfg *cfg = dev->config;
  struct pcf8576_data *data = dev->data;

  if (cfg->i2c.bus == NULL) {
    LOG_ERR("Failed to get pointer to %s device!", dev->name);
    return -EINVAL;
  }

  /* setting up slave address */
  if ((cfg->i2c.addr & 0xfe) != PCF8576_SLAVE_ADDRESS) {
    LOG_ERR("Invalid I2C address for device %s. Expected 0x%x found 0x%x",
            dev->name, PCF8576_SLAVE_ADDRESS, cfg->i2c.addr);
    return -EINVAL;
  }

  uint8_t mode = (DT_INST_PROP(0, powersave_mode) << 4) | 1 << 3 | // enable
                 (DT_ENUM_IDX(DT_INST(0, nxp_pcf8576), lcd_bias) << 2) |
                 (DT_INST_PROP(0, backplane_mux) & 0x03);

  uint8_t sub_address = (DT_INST_PROP(0, sub_address) & 0x07);
  uint8_t buf[3];
  buf[0] = PCF8576_CMD_CONTINUE | PCF8576_CMD_MODE_SET | mode;
  buf[1] = PCF8576_CMD_CONTINUE | PCF8576_CMD_LOAD_DP;
  buf[2] = PCF8576_CMD_LAST | PCF8576_CMD_DEVICE_SELECT | sub_address;
  if (i2c_write_dt(&cfg->i2c, buf, 3) < 0) {
    LOG_ERR("Failed to initialize device!");
    return -EIO;
  }

  memset(data->display_ram, 0x0, sizeof(data->display_ram));
  pcf8576_flush(dev);
  LOG_DBG("initialization OK.");
  return 0;
}

static void _pcf8576_int_to_digits(int32_t val, uint8_t digits[], size_t no_digits) {
  uint8_t sign;
  if(val < 0) {
    sign = DIGIT_NEG;
    val *= -1;
  }else {
    sign = DIGIT_BLANK;
  }
  int idx;
  for(idx = no_digits-1; idx >= 0; idx--) {
    digits[idx] = val%10;
    val/=10;
    if(val == 0) {
      break ;
    }
  }
  if(--idx >=0) {
    digits[idx--] = sign;
    for(; idx >= 0; idx--) {
      digits[idx] = DIGIT_BLANK;
    }
  }
}

void _pcf8576_num_ovf(uint8_t digits[], size_t no_digits) {
  for(int idx = 0; idx < no_digits; idx++) {
    digits[idx] = DIGIT_NEG;
  }
}

void _pcf8576_float_to_digits(float val, uint8_t digits[], size_t no_digits){

  int32_t intpart = (int32_t)val;
  bool is_negative = intpart < 0;

  size_t int_digits = _pcf8576_count_int_digits(intpart);

  if(int_digits > no_digits) {
    _pcf8576_num_ovf(digits, no_digits);
  }else {
    float fr_float = (val - (float)intpart);
//    int32_t fracpart = (int32_t)((val - (float)intpart)+0.5);
    int32_t fracpart = (int32_t)(fr_float+0.5);
    if((fracpart > 0) && (int_digits < no_digits)) {
      fracpart = is_negative ? fracpart * -1 : fracpart;
      size_t frac_digits = _pcf8576_count_int_digits(fracpart);
      frac_digits = (int_digits+frac_digits) > no_digits ? \
          no_digits - int_digits : frac_digits;
      size_t dp_pos = no_digits - int_digits;
      _pcf8576_int_to_digits(intpart, digits, int_digits);
      _pcf8576_int_to_digits(fracpart, &digits[dp_pos], frac_digits);
      digits[dp_pos] += DIGIT_DP;
    }else {
      _pcf8576_int_to_digits(intpart, digits, no_digits);
    }
  }
}

static void _pcf8576_set(const struct device *dev, const uint8_t data[2]) {
  ((struct pcf8576_data *)dev->data)->display_ram[data[1]] |= data[0];
}

static void _pcf8576_clear(const struct device *dev, const uint8_t data[2]) {
  ((struct pcf8576_data *)dev->data)->display_ram[data[1]] &= ~data[0];
}

static size_t _pcf8576_count_int_digits(int32_t number) {
  size_t cnt;
  if(number < 0) {
    number *= -1;
    cnt = 1;  // +1 position for the '-' sign
  }else {
    cnt = 0;
  }
  cnt +=  (number >= 1000000000) ? 10
        : (number >= 100000000) ? 9
        : (number >= 10000000) ? 8
        : (number >= 1000000) ? 7
        : (number >= 100000) ? 6
        : (number >= 10000) ? 5
        : (number >= 1000) ? 4
        : (number >= 100) ? 3
        : (number >= 10) ? 2
        : 1;
  return cnt;
}
//static size_t _pcf8576_count_frac_digits(float number, size_t max_digits, uint32_t *fr_part) {
//  number -= (int32_t)number;
//}

static const struct lcd_driver_api pcf8576_lcds_api = {.flush = pcf8576_flush};

#define PCF8576_INSTANTIATE(id)                                                \
  static const struct pcf8576_cfg pcf8576_##id##_cfg = {                       \
      .i2c = I2C_DT_SPEC_INST_GET(id)};                                        \
  static struct pcf8576_data pcf8576_##id##_data;                              \
  DEVICE_DT_INST_DEFINE(id, &pcf8576_initialize, NULL, &pcf8576_##id##_data,   \
                        &pcf8576_##id##_cfg, APPLICATION,                      \
                        CONFIG_LCD_INIT_PRIORITY, &pcf8576_lcds_api)

DT_INST_FOREACH_STATUS_OKAY(PCF8576_INSTANTIATE); // @suppress("Unused variable
                                                  // declaration in file scope")
