/*
* Copyright (c) 2022 Karoly Molnar
* SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zephyr.h>

#include <pcf8576.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#if defined(CONFIG_ZTEST)
#include <zephyr/ztest.h>
#include <string.h>
#endif
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

#if (CONFIG_ZTEST != 1)
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

  double countlarge = -3.09;
  int counsmall = 999;
  while (1) {
    pcf8576_num(dev, num_large, countlarge);
    pcf8576_num(dev, num_small, (double)counsmall--);
    countlarge = countlarge + 1.07;
    pcf8576_flush(dev);
    k_msleep(100);
  }
}
#else
ZTEST_SUITE(lcd_tests, NULL, NULL, NULL, NULL, NULL);
/**
 * @brief Test Asserts
 *
 * This test verifies various assert macros provided by ztest.
 *
 */
ZTEST(lcd_tests, test_init)
{
  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  zassert_not_null(dev, "LCD dev struct was null");
  zassert_true(device_is_ready(dev), "LCD device not ready");
  size_t num_large_arr_len = sizeof(digitarray_num_large);
  zassert_equal(6,num_large_arr_len, "array size mismatch!");
}
ZTEST(lcd_tests, test_large_num_int_pos)
{
  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  double countlarge = 1;
  pcf8576_num(dev, num_large, countlarge);
  uint8_t ref[] = {30,30,30,30,30,1};
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");


  countlarge = 1984;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 30;
  ref[2] = 1;
  ref[3] = 9;
  ref[4] = 8;
  ref[5] = 4;
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

  countlarge = 123456;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 1;
  ref[1] = 2;
  ref[2] = 3;
  ref[3] = 4;
  ref[4] = 5;
  ref[5] = 6;
  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD memory");

  countlarge = 0;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 30;
  ref[2] = 30;
  ref[3] = 30;
  ref[4] = 30;
  ref[5] = 0;

  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

}

ZTEST(lcd_tests, test_large_num_int_neg)
{
  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  memset(digitarray_num_large, 0, 6);
  double countlarge = -1;
  pcf8576_num(dev, num_large, countlarge);
  uint8_t ref[6]={30,30,30,30,10,1};
  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

  countlarge = -1984;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 10;
  ref[2] = 1;
  ref[3] = 9;
  ref[4] = 8;
  ref[5] = 4;

  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

  countlarge = -0;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 30;
  ref[2] = 30;
  ref[3] = 30;
  ref[4] = 30;
  ref[5] = 0;

  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

  countlarge = -19845;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 10;
  ref[1] = 1;
  ref[2] = 9;
  ref[3] = 8;
  ref[4] = 4;
  ref[5] = 5;

  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

}

ZTEST(lcd_tests, test_large_num_float_pos)
{
  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  double countlarge = 1.2;
  pcf8576_num(dev, num_large, countlarge);
  uint8_t ref[] = {30,30,30,30,21,2};
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

  countlarge = 1984.1;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 1;
  ref[2] = 9;
  ref[3] = 8;
  ref[4] = 24;
  ref[5] = 1;
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

  countlarge = 0.1;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 30;
  ref[2] = 30;
  ref[3] = 30;
  ref[4] = 20;
  ref[5] = 1;
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

  countlarge = -0.1;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 30;
  ref[2] = 30;
  ref[3] = 10;
  ref[4] = 20;
  ref[5] = 1;
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

  countlarge = -0.002;
  pcf8576_num(dev, num_large, countlarge);
  ref[0] = 30;
  ref[1] = 10;
  ref[2] = 20;
  ref[3] = 0;
  ref[4] = 0;
  ref[5] = 2;
  zassert_mem_equal(digitarray_num_large, ref, 6,"Mismatch in LCD memory");

}



ZTEST(lcd_tests, test_large_num_ovf)
{
  const struct device *dev = DEVICE_DT_GET(LCD_DEV_NODELABEL);
  memset(digitarray_num_large, 0, 6);
  double countlarge = 1234567;
  pcf8576_num(dev, num_large, countlarge);
  uint8_t ref[6]={10,10,10,10,10,10};
  zassert_mem_equal(digitarray_num_large, ref, 6, "Mismatch in LCD values");

  countlarge = -123456;
  pcf8576_num(dev, num_large, countlarge);

  countlarge = -123456.8;
  pcf8576_num(dev, num_large, countlarge);

}

#endif
