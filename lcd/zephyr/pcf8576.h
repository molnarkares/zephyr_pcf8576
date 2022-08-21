/*
* Copyright (c) 2022 Karoly Molnar
*
* SPDX-License-Identifier: Apache-2.0
*/

#ifndef ZEPHYR_INCLUDE_DISPLAY_PCF8576_H_
#define ZEPHYR_INCLUDE_DISPLAY_PCF8576_H_

#include <zephyr/device.h>
#include <zephyr/types.h>

#define PCF8576_NAME "PCF8576"

typedef uint8_t nums_t[8][2];

#define SEG2SHIFT(x, y)                                                        \
  { (1 << (7 - ((x) + (((y)&0x01) << 2)))), (y) >> 1 }

#define pcf8576_sign(dev, label, state)                                        \
  do {                                                                         \
    const uint8_t tmp_seg[] = SEG2SHIFT(                                       \
        DT_PROP_BY_IDX(DT_PHANDLE(DT_NODELABEL(label), sign), segment, 0),     \
        DT_PROP_BY_IDX(DT_PHANDLE(DT_NODELABEL(label), sign), segment, 1));    \
    if (state) {                                                               \
      _pcf8576_set(dev, tmp_seg);                                              \
    } else {                                                                   \
      _pcf8576_clear(dev, tmp_seg);                                            \
    }                                                                          \
  } while (0)

#define _BAR_CFG(item)                                                         \
  SEG2SHIFT(DT_PROP_BY_IDX(DT_PHANDLE(item, bar), segment, 0),                 \
            DT_PROP_BY_IDX(DT_PHANDLE(item, bar), segment, 1)),

#define pcf8576_bar_define(label)                                              \
  const uint8_t bararray_##label[][2] = {                                      \
      DT_FOREACH_CHILD(DT_NODELABEL(label), _BAR_CFG)};

#define pcf8576_bar_declare(label) extern const uint8_t bararray_##label[][2];

#define _NUMBERS_CFG(item)                                                     \
  {SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 0),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 0),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 1),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 1),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 2),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 2),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 3),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 3),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 4),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 4),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 5),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 5),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 6),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 6),   \
                      segment, 1)),                                            \
   SEG2SHIFT(                                                                  \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 7),   \
                      segment, 0),                                             \
       DT_PROP_BY_IDX(DT_PHANDLE_BY_IDX(DT_PHANDLE(item, number), digit, 7),   \
                      segment, 1))},

#define pcf8576_num_define(label)                                              \
  const nums_t numarray_##label[] = {                                          \
      DT_FOREACH_CHILD(DT_NODELABEL(label), _NUMBERS_CFG)};

#define pcf8576_num_declare(label) extern const nums_t numarray_##label[];

#define pcf8576_bar(dev, label, value)                                         \
  do {                                                                         \
    size_t idx = (sizeof(bararray_##label) / 2);                               \
    do {                                                                       \
      _pcf8576_clear(dev, bararray_##label[--idx]);                            \
    } while (idx > (size_t)value);                                             \
    do {                                                                       \
      _pcf8576_set(dev, bararray_##label[--idx]);                              \
    } while (idx > 0);                                                         \
  } while (0)

#define pcf8576_num(dev, label, value)                                         \
  do {                                                                         \
    uint8_t digitarray[sizeof(numarray_##label) / sizeof(nums_t)];             \
    _pcf8576_float_to_digits(value, digitarray, sizeof(digitarray));           \
    for (size_t num_idx = 0; num_idx < sizeof(digitarray); num_idx++) {        \
      _pcf8576_set_digit(dev, numarray_##label[num_idx], digitarray[num_idx]); \
    }                                                                          \
  } while (0)

void pcf8576_flush(const struct device *dev);

/* internal functions used by the lcd macros. Do not call them directly */
void _pcf8576_set_digit(const struct device *dev, const uint8_t segment[][2],
                        uint8_t value);
void _pcf8576_float_to_digits(double val, uint8_t digits[], size_t no_digits);
void _pcf8576_set(const struct device *dev, const uint8_t data[2]);
void _pcf8576_clear(const struct device *dev, const uint8_t data[2]);

#endif /* ZEPHYR_INCLUDE_DISPLAY_PCF8576_H_ */
