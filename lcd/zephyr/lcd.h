/*
* Copyright (c) 2022 Karoly Molnar
* SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_LCD_H_
#define ZEPHYR_INCLUDE_DRIVERS_LCD_H_

/**
 * @brief LCD Interface
 * @defgroup lcd_interface LCD Interface
 * @ingroup io_interfaces
 * @{
 */

#include <zephyr/device.h>
#include <zephyr/types.h>

/**
 * @typedef lcd_api_flush()
 * @brief API for transfering display data to device's RAM
 *
 */
typedef void (*lcd_api_flush)(const struct device *dev);
/**
 * @brief LCD driver API
 *
 * This is the mandatory API any LCD driver needs to expose.
 */
__subsystem struct lcd_driver_api {
  lcd_api_flush flush;
};

#endif /* ZEPHYR_INCLUDE_DRIVERS_LCD_H_ */
