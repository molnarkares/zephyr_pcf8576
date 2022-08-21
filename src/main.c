/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zephyr.h>

#include <pcf8576.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lcdtest);

#define LCD_DEV_NODELABEL DT_NODELABEL(lcd_drv)

pcf8576_num_declare(num_small);
pcf8576_num_define(num_small);
pcf8576_num_declare(num_large);
pcf8576_num_define(num_large);

pcf8576_bar_declare(bar_battery);
pcf8576_bar_define(bar_battery);

pcf8576_bar_declare(bar_antenna);
pcf8576_bar_define(bar_antenna);

void main(void) {
  LOG_INF("Demo started");

  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  if (dev == NULL) {
    LOG_ERR("Could not get PCF8576 device\n");
    return;
  } else {
    LOG_INF("PCF8576 device found\n");
  }
  if (!device_is_ready(dev)) {
    LOG_ERR("PCF8576 device is not ready!\n");
    return;
  }else {
    LOG_INF("PCF8576 device is ready\n");
  }

  int countlarge = -50;
  int counsmall = 999;
  while (1) {
    pcf8576_num(dev, num_large, (double)countlarge++);
    pcf8576_num(dev, num_small, (double)counsmall--);
    pcf8576_flush(dev);
    k_msleep(100);
  }
}
