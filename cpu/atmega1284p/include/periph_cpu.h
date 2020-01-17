/*
 * Copyright (C) 2015 HAW Hamburg
 *               2016 Freie Universität Berlin
 *               2018 Matthew Blue
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_atmega1284p
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          René Herthel <rene-herthel@outlook.de>
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author          Matthew Blue <matthew.blue.neuro@gmail.com>
 */

#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include "periph_cpu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Available ports on the ATmega1284p family
 */
enum {
    PORT_A = 0,       /**< port A */
    PORT_B = 1,       /**< port B */
    PORT_C = 2,       /**< port C */
    PORT_D = 3,       /**< port D */
    GPIO_EXT_PORT = 4 /**< first GPIO extender port */
};

/**
 * @brief   Available ports on the ATmega1284p family as GPIO register definitions
 */
#define GPIO_CPU_PORTS \
    GPIO_CPU_PORT(0x22), /* port A */ \
    GPIO_CPU_PORT(0x25), /* port B */ \
    GPIO_CPU_PORT(0x28), /* port C */ \
    GPIO_CPU_PORT(0x2b), /* port D */

/**
 * @brief   Available external interrupt pins on the ATmega1284p family
 *
 * In order of their interrupt number
 */
#define CPU_ATMEGA_EXT_INTS    { GPIO_PIN(PORT_D, 2), \
                                 GPIO_PIN(PORT_D, 3), \
                                 GPIO_PIN(PORT_B, 2) }

/**
 * @name   Defines for the I2C interface
 * @{
 */
#define I2C_PORT_REG            PORTC
#define I2C_PIN_MASK            (1 << PORTC0) | (1 << PORTC1)
/** @} */

/**
 * @name    EEPROM configuration
 * @{
 */
#define EEPROM_SIZE                (4096U)  /* 4kB */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
/** @} */
