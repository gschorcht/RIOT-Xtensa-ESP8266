/*
 * Copyright (C) 2018 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_pcf857x PCF857X I2C I/O expanders
 * @ingroup     drivers_misc
 * @ingroup     drivers_saul
 * @brief       Device driver for Texas Instruments PCF857X I2C I/O expanders
 *
 * ## Overview
 *
 * Texas Instruments PCF857X I2C I/O expanders provide general purpose I/O
 * extension via I2C bus. The driver supports the following PCF857X I2C I/O
 * expander variants:
 *
 * <center>
 * Expander | Type                    | Pseudomodule
 * :--------|:------------------------|:------------
 * PCF8574  |  8-bit I2C I/O expander | `pcf8574`
 * PCF8574A |  8-bit I2C I/O expander | `pcf8574a`
 * PCF8575  | 16-bit I2C I/O expander | `pcf8575`
 * </center><br>
 *
 * For each of these PCF857X I2C I/O expanders variants, the driver defines
 * a separate pseudomodule. Multiple PCF857X I2C I/O expanders and different
 * variants can be used at the same time. The application has to specify used
 * PCF857X I2C I/O expander variants as a list of used pseudomodules. For
 * example, to use PCF8574A and PCF857X I/O expanders at the same time, the
 * make command would be:
 *
 *      USEMODULE="pcf8574a pcf8575" make -C tests/driver_pcf857x BOARD=...
 *
 * At least one PCF857X I2C I/O expander variant has to be specified. The
 * driver module `pcf857x` is then enabled implicitly.
 *
 * @note While PCF8575 is working in I2C fast mode with up to 400 kHz clock
 * frequency, PCF8574 and PCF8574A are only specified for I2C normal mode
 * with up to 100 kHz clock frequency. However, they seem also to work at
 * 400 kHz clock frequency.
 *
 * The driver interface is kept as compatible as possible with the peripheral
 * GPIO interface. The only differences are that
 *
 * - functions have the prefix `pcf857x_` and
 * - functions require an additional parameter, the pointer to the expander
 *   device of type #pcf857x_t.
 *
 * ## Expander GPIOs
 *
 * The PCF857X expander devices provide a GPIO expansion over the I2C
 * interface with either
 *
 * - 1 port with 8 quasi-parallel input/output (I/O) pins (PCF8574, PCF8574A) or
 * - 2 ports with 8 quasi-parallel input/output (I/O) pins each (PCF8575).
 *
 * Each quasi-bidirectional expander I/O pin can be used as an input or output
 * without the use of a data-direction control signal. Output pins are latched
 * and have high-current drive capability for directly driving LEDs.
 * The quasi-bidirectional expander I/O pins without direction control
 * works as follows:
 *
 * - INPUT:
 *   Writing a 1 to an expander pin configures the pin as an INPUT. The pin is
 *   pulled up to HIGH high by a weak 100 μA pull-up, which corresponds to the
 *   behavior of the #GPIO_IN_PU and #GPIO_OD_PU modes.
 * - OUTPUT:
 *   Writing a 0 to an expander pin configures the pin as an OUTPUT and
 *   actively drives the pin to LOW, which corresponds to the behavior of
 *   the #GPIO_OUT and #GPIO_OD_PU modes.
 *
 * @note The driver therefore supports the #GPIO_IN_PU and #GPIO_OD modes as
 * well as the physically identical modes #GPIO_IN, #GPIO_OUT and #GPIO_OD_PU.
 * Please keep this in mind when connecting these pins to other open-drain
 * or output pins that do not generate active signals.
 *
 * After the initialization with the #pcf857x_init function, all
 * expander I/O pins are in input mode and pulled-up to HIGH.
 *
 * The expander I/O pins can be adressed as GPIO pins using the following
 * scheme:
 *
 * PCF857X pin label | Port | Pin | RIOT symbol               | Remark
 * ----------------- |:----:|:---:|:--------------------------|:-----------------
 * P00               | 0    | 0   | `PCF857X_GPIO_PIN(0, 0)`  | PCF8574, PCF8574A and PCF8575
 * P01               | 0    | 1   | `PCF857X_GPIO_PIN(0, 1)`  | PCF8574, PCF8574A and PCF8575
 * ...               | ...  | ... | ...                       | ...
 * P07               | 0    | 7   | `PCF857X_GPIO_PIN(0, 7)`  | PCF8574, PCF8574A and PCF8575
 * P10               | 0    | 8   | `PCF857X_GPIO_PIN(0, 8)`  | PCF8575 only
 * P11               | 0    | 9   | `PCF857X_GPIO_PIN(0, 9)`  | PCF8575 only
 * ...               | ...  | ... | ...                       | ...
 * P17               | 0    | 15  | `PCF857X_GPIO_PIN(0, 15)` | PCF8575 only
 *
 * ## Interrupts
 *
 * PCF857X expander has an open-drain, low-active interrupt (INT) signal, which
 * generates an interrupt by any rising or falling edge of the expander pins
 * in the input mode. Using this expander interrupt signal, the following
 * features become available:
 *
 * - An interrupt service function can be attached to an expander input pin with
 *   the #pcf857x_gpio_init_int function. This interrupt service function is
 *   then called on any rising and/or falling edge of the expander input pin.
 *
 * - The driver can keep an internal up-to-date state of all expander input
 *   pins. Using this internal up-to-date state of all expander input pins
 *   avoids to read all expander input pins via I2C each time a single input
 *   value of an expander GPIO pin is read with #pcf857x_gpio_read.
 *
 * Since interrupts are handled in the context of a separate event thread,
 * enabling interrupts requires more RAM. Therefore, interrupts have to be
 * activated explicitly with the module `pcf857x_irq`. Furthermore, the MCU
 * interrupt pin to which the PCF857X INT signal is connected has to be
 * defined by the parameter pcf857x_params_t::int_pin either in the parameter
 * file or at the command line, e.g.
 *
 *      CFLAGS="-DPCF857X_PARAM_INT_PIN=\(GPIO_PIN\(0,6\)\)" \
 *      USEMODULE="pcf8575 pcf857x_irq" make -C tests/driver_pcf857x BOARD=...
 *
 * <br>
 * @note If an output of the expander is connected to an input of the same
 * expander, there is no interrupt triggered by the input when the
 * output changes. Therefore, a write operation to an output with
 * #pcf857x_gpio_write, #pcf857x_gpio_clear, #pcf857x_gpio_set or
 * #pcf857x_gpio_toggle leads to an additional read-after-write operation,
 * if interrupts are used.<br><br>
 * The use of interrupts therefore increases the read performance considerably,
 * since I2C read operations are required only when the inputs change. But the
 * write performance is reduced to the half.
 *
 * ## The Interrupt Context Problem
 *
 * Handling an interrupt of a PCF857x expander requires the driver to access
 * the device directly via I2C. However, the mutex-based synchronization of
 * I2C accesses does not work in the interrupt context. Therefore the ISR must
 * not access the PCF857x expander device directly. Rather, the ISR must only
 * indicate the occurrence of the interrupt which has to be handled
 * asynchronously in the thread context.
 *
 * For this purpose the event thread module `event_thread_medium` is used when
 * interrupts are enabled by the module `pcf857x_irq`. The driver then
 * handles the interrupts in the context of the event thread with medium
 * priority.
 *
 * ## SAUL Capabilities
 *
 * The driver provides SAUL capabilities that are compatible to the SAUL
 * capabilities of peripheral GPIOs. Each PCF857X expander I/O pin can be
 * mapped directly to SAUL by defining an according entry in
 * \c PCF857X_SAUL_GPIO_PARAMS. Please refer file
 * `$RIOTBASE/drivers/pcf857x/include/pcf857x_params.h` for an example.
 *
 * @note Module `saul_gpio` has to be added to the
 * project to enable SAUL capabilities of the PCF857X driver, e.g.,
 *
 *      USEMODULE="pcf8575 saul_gpio" make -C tests/saul BOARD=...
 *
 * ## Using Multiple Devices
 *
 * It is possible to used multiple devices and different variants of PCF857X
 * I/O expanders at the same time. The application has to specify used PCF857X
 * I/O expander variants by a list of used pseudomodules. For example, to use
 * one PCF8574A and one PCF857X I/O expander the make command would be:
 *
 *      USEMODULE="pcf8574a pcf8575" make -C tests/driver_pcf857x BOARD=...
 *
 * Furthermore, used devices have to be configured by defining the
 * configuration parameter array `pcf857x_params` of type #pcf857x_params_t.
 * The default configuration for one device is defined in
 * `drivers/pcf857x/pcf857x_params.h`. The application can override it by by
 * placing a file `pcf857x_params.h` in the application directory `$APPDIR`.
 * For example, the definition of the configuration parameter array for the
 * two devices above could be:
 *
 *      static const pcf857x_params_t pcf857x_params[] = {
 *          {
 *              .dev = I2C_DEV(0),
 *              .addr = 0,
 *              .exp = PCF857X_EXP_PCF8574A,
 *              .int_pin = GPIO_PIN(0,1),
 *          },
 *          {
 *              .dev = I2C_DEV(0),
 *              .addr = 0,
 *              .exp = PCF857X_EXP_PCF8575,
 *              .int_pin = GPIO_PIN(0,2),
 *          },
 *      };
 *
 * @{
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @file
 */

#ifndef PCF857X_H
#define PCF857X_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#include "periph/gpio.h"
#include "periph/i2c.h"

#if MODULE_SAUL_GPIO || DOXYGEN
#include "saul/periph.h"
#endif /* MODULE_SAUL_GPIO */

#if MODULE_PCF857X_IRQ || DOXYGEN
#include "event.h"
#endif /* MODULE_PCF857X_IRQ */

#if !MODULE_PCF8574 && !MODULE_PCF8574A && !MODULE_PCF8575
#error "Please provide a list of pcf857x variants used by the application (pcf8574, pcf8574a or pcf8575)"
#endif

/**
 * @name    PCF857X I2C slave addresses
 *
 * PCF857X I2C slave addresses are defined as an offset to a base address,
 * which depends on the expander used. The address offset is in the range
 * of 0 to 7.
 * @{
 */
#define PCF8575_BASE_ADDR   (0x20)  /**< PCF8575 I2C slave base address.
                                         Addresses are then in range from
                                         0x20 to 0x27 */
#define PCF8574_BASE_ADDR   (0x20)  /**< PCF8574 I2C slave base address.
                                         Addresses are then in range from
                                         0x20 to 0x27 */
#define PCF8574A_BASE_ADDR  (0x38)  /**< PCF8574A I2C slave base address.
                                         Addresses are then in range from
                                         0x38 to 0x3f */

/**
 * @name    PCF857X I/O expander pin number
 */
#define PCF8575_GPIO_PIN_NUM    (16) /**< PCF8575 has 16 I/O pins */
#define PCF8574_GPIO_PIN_NUM    (8)  /**< PCF8574 has 8 I/O pins */
#define PCF8574A_GPIO_PIN_NUM   (8)  /**< PCF8574A has 8 I/O pins */


/** conversion of (port x : pin y) to a pin number */
#define PCF857X_GPIO_PIN(x,y)  ((gpio_t)((x & 0) | y))

/**
 * @name   Module dependent definitions and declarations
 * @{
 */
#if MODULE_PCF8575 || DOXYGEN

/**
 * @brief   Maximum number of GPIO pins
 *
 * Defines the maximum number of GPIO pins of all PCF857X I/O expanders
 * used. If a PCF8575 is used, the maximum number is 16 I/O pins.
 */
#define PCF857X_GPIO_PIN_NUM   (16)

/**
 * @brief   Data type that can mask all expander pins
 *
 * If a PCF8575 is used, the 16 I/O pins have to be masked.
 */
typedef uint16_t pcf857x_data_t;

#else /* MODULE_PCF8575 || DOXYGEN */

#define PCF857X_GPIO_PIN_NUM   (8)  /**< PCF8574, PCF8574 provide 8 I/O pins */
typedef uint8_t pcf857x_data_t;     /**< type that can mask all expander pins */

#endif /* MODULE_PCF8575 || DOXYGEN */
/** @} */

/** Definition of PCF857X driver error codes */
typedef enum {
    PCF857X_OK,                 /**< success */
    PCF857X_ERROR_I2C,          /**< I2C communication error */
    PCF857X_ERROR_INV_EXP,      /**< invalid expander variant */
    PCF857X_ERROR_INV_MODE,     /**< invalid pin mode */
    PCF857X_ERROR_INV_FLANK,    /**< invalid interrupt flank */
} pcf857x_error_codes_t;

/**
 * @brief Definition of PCF857X expander variants
 *
 * It is used in configuration parameters to specify the PCF857X expander
 * used by device.
 *
 * @note Expander variants known by the driver depend on enabled pseudomodules
 * `pcf8574`, `pcf8574a` and `pcf8575`.
 */
typedef enum {
#if MODULE_PCF8574 || DOXYGEN
    PCF857X_EXP_PCF8574,    /**< PCF8574 8 bit I/O expander used */
#endif
#if MODULE_PCF8574A || DOXYGEN
    PCF857X_EXP_PCF8574A,   /**< PCF8574A 8 bit I/O expander */
#endif
#if MODULE_PCF8575 || DOXYGEN
    PCF857X_EXP_PCF8575,    /**< PCF8575 16 bit I/O expander */
#endif
    PCF857X_EXP_MAX,
} pcf857x_exp_t;

/**
 * @brief   PCF857X device initialization parameters
 */
typedef struct {

    i2c_t    dev;       /**< I2C device (default I2C_DEV(0)) */
    uint16_t addr;      /**< I2C slave address offset to the PCF7857X base
                             address (default 0) */
    pcf857x_exp_t exp;  /**< PCF857X expander variant used by the device
                             (default depends on used pseudomodules */

#if MODULE_PCF857X_IRQ || DOXYGEN
    gpio_t   int_pin;   /**< MCU interrupt pin or #GPIO_UNDEF if not used (default).
                             Using interrupt pin has the advantage that inputs
                             have to be read from expander only if any input
                             value changes.
                             @note To use interrupts for expander inputs, this
                             pin has to be defined. */
#endif /* MODULE_PCF857X_IRQ */
} pcf857x_params_t;

#if MODULE_PCF857X_IRQ || DOXYGEN
/**
 * @brief   IRQ event type
 *
 * Handling an interrupt of a PCF857x expander requires the driver to access
 * the device directly via I2C. However, the mutex-based synchronization of
 * I2C accesses does not work in the interrupt context. Therefore the ISR must
 * not access the PCF857x expander device directly. Rather, the ISR must only
 * indicate the occurrence of the interrupt which has to be handled
 * asynchronously in the thread context.
 *
 * The type defines the data structure that is part of each device data
 * structure to indicate that an interrupt of the device occured. Since there
 * is only one interrupt source, only one interrupt can be pending per device.
 * Thus, only one object of this type per device is required.
 */
typedef struct {
    event_t event;      /**< inherited event data structure */
    void *dev;          /**< PCF857X device reference */
} pcf857x_irq_event_t;

#endif /* MODULE_PCF857X_IRQ */

/**
 * @brief   PCF857X device data structure type
 */
typedef struct {
    pcf857x_params_t params;  /**< device initialization parameters */

    unsigned int pin_num;     /**< number of I/O pins, depends on used expander
                                   variant */
    pcf857x_data_t modes;     /**< expander pin modes */
    pcf857x_data_t in;        /**< expander intput pin values */
    pcf857x_data_t out;       /**< expander output pin values */

#if MODULE_PCF857X_IRQ || DOXYGEN
    gpio_isr_ctx_t isr[PCF857X_GPIO_PIN_NUM];  /**< ISR with arg for each expander pin */
    gpio_flank_t flank[PCF857X_GPIO_PIN_NUM];  /**< interrupt flank for each expander pin */
    bool enabled[PCF857X_GPIO_PIN_NUM];        /**< enabled flag for each expander pin */
    pcf857x_irq_event_t irq_event;  /**< IRQ event object used for the device */
#endif /* MODULE_PCF857X_IRQ */

} pcf857x_t;

#if MODULE_SAUL_GPIO || DOXYGEN
/**
 * @brief   PCF857X configuration structure for mapping expander pins to SAUL
 *
 * This data structure is an extension of the GPIO configuration structure for
 * mapping GPIOs to SAUL. The only additional information required is a
 * reference to the according PCF857X device.
 *
 * @note To use PCF857X with SAUL, module `saul_gpio` has to be added to the
 * project.
 */
typedef struct {
    unsigned int dev;           /**< PCF857X device index */
    saul_gpio_params_t gpio;    /**< GPIO configuration for mapping to SAUL */
} pcf857x_saul_gpio_params_t;
#endif

/**
 * @brief   Initialize the PCF857X I/O expander
 *
 * All expander pins are set to be input and are pulled up.
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   params  configuration parameters, see #pcf857x_params_t
 *
 * @retval  PCF857X_OK      on success
 * @retval  PCF857X_ERROR_* a negative error code on error,
 *                           see #pcf857x_error_codes_t
 */
int pcf857x_init(pcf857x_t *dev, const pcf857x_params_t *params);

/**
 * @brief   Initialize a PCF857X pin
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to initialize, use PCF857X_GPIO_PIN(x,y) to specify
 * @param[in]   mode    mode of the pin, see #gpio_t
 *
 * @note Only #GPIO_IN, #GPIO_OUT and #GPIO_IN_PU are supported as \p mode.
 * For all other modes, the function returns with #PCF857X_ERROR_INV_MODE.
 *
 * @retval  PCF857X_OK          on success
 * @retval  PCF857X_ERROR_*     a negative error code on error,
 *                              see #pcf857x_error_codes_t
 */
int pcf857x_gpio_init(pcf857x_t *dev, gpio_t pin, gpio_mode_t mode);

#if MODULE_PCF857X_IRQ || DOXYGEN
/**
 * @brief   Initialize a PCF857X pin for external interrupt usage
 *
 * The registered callback function will be called in interrupt context every
 * time the defined flank(s) are detected. Therefore, it MUST NOT be blocking
 * or time-consuming.
 *
 * The interrupt is activated automatically after the initialization.
 *
 * @note
 * - Module `periph_gpio_irq` has to be added to the project to enable this
 *   function.
 * - The MCU interrupt pin connected to the PCF857X INT signal has to be
 *   defined by parameter pcf857x_params_t::int_pin.
 * - Only #GPIO_IN, #GPIO_OUT and #GPIO_IN_PU are supported as \p mode. For all
 *   other modes, the function returns with #PCF857X_ERROR_INV_MODE.
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to initialize, use PCF857X_GPIO_PIN(x,y) to specify
 * @param[in]   mode    mode of the pin, see #gpio_t
 * @param[in]   flank   define the active flanks, see #gpio_flank_t
 * @param[in]   isr     ISR that is called back from interrupt context
 * @param[in]   arg     optional argument passed to the callback
 *
 * @retval  PCF857X_OK          on success
 * @retval  PCF857X_ERROR_*     a negative error code on error,
 *                              see #pcf857x_error_codes_t
 */
int pcf857x_gpio_init_int(pcf857x_t *dev, gpio_t pin,
                                          gpio_mode_t mode,
                                          gpio_flank_t flank,
                                          gpio_cb_t isr,
                                          void *arg);
#endif /* MODULE_PCF857X_IRQ || DOXYGEN */

/**
 * @brief   Get the value from PCF857X input pin
 *
 * @note If the PCF857X interrupt is used, the read operation does not perform
 * an I2C read operation since the last input pin value is already read.
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to read, use PCF857X_GPIO_PIN(x,y) to specify
 */
int pcf857x_gpio_read(pcf857x_t *dev, gpio_t pin);

/**
 * @brief   Write the value to PCF857X input pin
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to write, use PCF857X_GPIO_PIN(x,y) to specify
 * @param[in]   value   value to write
 */
void pcf857x_gpio_write(pcf857x_t *dev, gpio_t pin, int value);

/**
 * @brief   Clear the PCF857X output pin
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to clear, use PCF857X_GPIO_PIN(x,y) to specify
 */
void pcf857x_gpio_clear(pcf857x_t *dev, gpio_t pin);

/**
 * @brief   Set the PCF857X output pin
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to set, use PCF857X_GPIO_PIN(x,y) to specify
 */
void pcf857x_gpio_set(pcf857x_t *dev, gpio_t pin);

/**
 * @brief   Toggle the value of the PCF857X output pin
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to toggle, use PCF857X_GPIO_PIN(x,y) to specify
 */
void pcf857x_gpio_toggle(pcf857x_t *dev, gpio_t pin);

#if MODULE_PCF857X_IRQ || DOXYGEN
/**
 * @brief   Enable pin interrupt
 *
 * @note
 * - Module `periph_gpio_irq` has to be added to the project to enable this
 *   function.
 * - The MCU interrupt pin connected to the PCF857X INT signal has to be
 *   defined by parameter pcf857x_params_t::int_pin.
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to enable the interrupt for
 */
void pcf857x_gpio_irq_enable(pcf857x_t *dev, gpio_t pin);

/**
 * @brief   Disable pin interrupt
 *
 * @note
 * - Module `periph_gpio_irq` has to be added to the project to enable this
 *   function.
 * - The MCU interrupt pin connected to the PCF857X INT signal has to be
 *   defined by parameter pcf857x_params_t::int_pin.
 *
 * @param[in]   dev     descriptor of PCF857X I/O expander device
 * @param[in]   pin     pin to enable the interrupt for
 */
void pcf857x_gpio_irq_disable(pcf857x_t *dev, gpio_t pin);
#endif /* MODULE_PCF857X_IRQ || DOXYGEN */

#ifdef __cplusplus
}
#endif

#endif /* PCF857X_H */
/** @} */
