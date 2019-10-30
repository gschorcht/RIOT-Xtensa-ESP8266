/*
 * Copyright (C) 2018 Acutam Automation, LLC
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_extend_gpio
 *
 * @{
 *
 * @file
 * @brief       Default/Example GPIO extension device list
 *
 * If the `extend_gpio` module is enabled, a list of GPIO extension devices
 * of type @ref gpio_dev_t has to be provided as a configuration.
 *
 * This file defines a default list containing only the device with the
 * default not-supported GPIO extension functions `gpio_ext_*_notsup`.
 *
 * The board has to override this file with the real GPIO extension device
 * list in file `boards/.../include/gpio_ext_conf.h`.
 *
 * @author      Matthew Blue <matthew.blue.neuro@gmail.com>
 *
 * @}
 */

#ifndef GPIO_EXT_CONF_H
#define GPIO_EXT_CONF_H

#include <stddef.h>

#include "extend/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default/Example GPIO extension device driver
 */
extern const gpio_driver_t gpio_ext_notsup_driver;

/**
 * @brief   Default/Example GPIO extension device list
 */
static const gpio_dev_t gpio_ext[] =
{
    {
        .driver = &gpio_ext_notsup_driver,
        .dev = NULL,
    },
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_EXT_CONF_H */
/** @} */