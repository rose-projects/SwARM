#ifndef BOARD_H
#define BOARD_H

/*
 * Setup for SWARM STM32 F302R6T6 board.
 */

/*
 * Board identifier.
 */
#define BOARD_ST_STM32F3_DISCOVERY
#define BOARD_NAME                  "SWARM F302R6T6"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000U
#endif

// #define STM32_LSE_BYPASS
#define STM32_HSE_BYPASS

/*
 * MCU type as defined in the ST header.
 */
#define STM32F302x8 // closer to F302R6T6

/*
 * IO pins assignments.
 */
#define GPIOA_VBAT_PROBE            0U
#define GPIOA_MTR_LED_L             1U
#define GPIOA_MTR_PHASE_L           2U
#define GPIOA_MTR_ENABLE_L          3U
#define GPIOA_PIN4                  4U
#define GPIOA_MTR_PHASE_R           5U
#define GPIOA_MTR_ENABLE_R          6U
#define GPIOA_MTR_TRA_L             7U
#define GPIOA_PIN8                  8U
#define GPIOA_USB_VBUS              9U
#define GPIOA_USB_CONNECT           10U
#define GPIOA_USB_DM                11U
#define GPIOA_USB_DP                12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_DWM_SPI_CSn           15U

#define GPIOB_MTR_TRA_R             0U
#define GPIOB_LED_IR_C              1U
#define GPIOB_PIN2                  2U
#define GPIOB_DWM_WAKEUP            3U
#define GPIOB_PIN4                  4U
#define GPIOB_PIN5                  5U
#define GPIOB_PIN6                  6U
#define GPIOB_MPU_INT               7U
#define GPIOB_MPU_I2C_SCL           8U
#define GPIOB_MPU_I2C_SDA           9U
#define GPIOB_LED_IR_R              10U
#define GPIOB_LED_IR_L              11U
#define GPIOB_PIN12                 12U
#define GPIOB_LED_SPI_CK            13U
#define GPIOB_PIN14                 14U
#define GPIOB_LED_SPI_DO            15U

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_MTR_LED_R             4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_DWM_SPI_CLK           10U
#define GPIOC_DWM_SPI_MISO          11U
#define GPIOC_DWM_SPI_MOSI          12U
#define GPIOC_PIN13                 13U
#define GPIOC_OSC32_IN              14U
#define GPIOC_OSC32_OUT             15U

#define GPIOD_DWM_IRQ               2U

#define GPIOF_OSC_IN                0U
#define GPIOF_OSC_OUT               1U

/*
 * IO lines assignments.
 */
#define LINE_VBAT_PROBE            PAL_LINE(GPIOA, 0U)
#define LINE_MTR_LED_L             PAL_LINE(GPIOA, 1U)
#define LINE_MTR_PHASE_L           PAL_LINE(GPIOA, 2U)
#define LINE_MTR_ENABLE_L          PAL_LINE(GPIOA, 3U)
#define LINE_MTR_PHASE_R           PAL_LINE(GPIOA, 5U)
#define LINE_MTR_ENABLE_R          PAL_LINE(GPIOA, 6U)
#define LINE_MTR_TRA_L             PAL_LINE(GPIOA, 7U)
#define LINE_USB_VBUS              PAL_LINE(GPIOA, 9U)
#define LINE_USB_CONNECT           PAL_LINE(GPIOA, 10U)
#define LINE_USB_DM                PAL_LINE(GPIOA, 11U)
#define LINE_USB_DP                PAL_LINE(GPIOA, 12U)
#define LINE_SWDIO                 PAL_LINE(GPIOA, 13U)
#define LINE_SWCLK                 PAL_LINE(GPIOA, 14U)
#define LINE_DWM_SPI_CSn           PAL_LINE(GPIOA, 15U)

#define LINE_MTR_TRA_R             PAL_LINE(GPIOB, 0U)
#define LINE_LED_IR_C              PAL_LINE(GPIOB, 1U)
#define LINE_DWM_WAKEUP            PAL_LINE(GPIOB, 3U)
#define LINE_MPU_INT               PAL_LINE(GPIOB, 7U)
#define LINE_MPU_I2C_SCL           PAL_LINE(GPIOB, 8U)
#define LINE_MPU_I2C_SDA           PAL_LINE(GPIOB, 9U)
#define LINE_LED_IR_R              PAL_LINE(GPIOB, 10U)
#define LINE_LED_IR_L              PAL_LINE(GPIOB, 11U)
#define LINE_LED_SPI_CK            PAL_LINE(GPIOB, 13U)
#define LINE_LED_SPI_DO            PAL_LINE(GPIOB, 15U)

#define LINE_MTR_LED_R             PAL_LINE(GPIOC, 4U)
#define LINE_DWM_SPI_CLK           PAL_LINE(GPIOC, 10U)
#define LINE_DWM_SPI_MISO          PAL_LINE(GPIOC, 11U)
#define LINE_DWM_SPI_MOSI          PAL_LINE(GPIOC, 12U)
#define LINE_OSC32_IN              PAL_LINE(GPIOC, 14U)
#define LINE_OSC32_OUT             PAL_LINE(GPIOC, 15U)

#define LINE_DWM_IRQ               PAL_LINE(GPIOD, 2U)

#define LINE_OSC_IN                PAL_LINE(GPIOF, 0U)
#define LINE_OSC_OUT               PAL_LINE(GPIOF, 1U)

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

/*
 * GPIOA setup:
 *
 * PA0  - VBAT_PROBE                (input floating).
 * PA1  - MTR_LED_L                 (input pullup).
 * PA2  - MTR_PHASE_L               (input pullup).
 * PA3  - MTR_ENABLE_L              (input pullup).
 * PA4  - PIN4                      (input pullup).
 * PA5  - MTR_PHASE_R               (alternate 5).
 * PA6  - MTR_ENABLE_R              (alternate 5).
 * PA7  - MTR_TRA_L                 (alternate 5).
 * PA8  - PIN8                      (input pullup).
 * PA9  - USB_VBUS                  (input pullup).
 * PA10 - USB_CONNECT               (input pullup).
 * PA11 - USB_DM                    (alternate 14).
 * PA12 - USB_DP                    (alternate 14).
 * PA13 - SWDIO                     (alternate 0).
 * PA14 - SWCLK                     (alternate 0).
 * PA15 - DWM_SPI_CSn               (input pullup).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_VBAT_PROBE) |     \
                                     PIN_MODE_INPUT(GPIOA_MTR_LED_L) |      \
                                     PIN_MODE_INPUT(GPIOA_MTR_PHASE_L) |    \
                                     PIN_MODE_INPUT(GPIOA_MTR_ENABLE_L) |   \
                                     PIN_MODE_INPUT(GPIOA_PIN4) |           \
                                     PIN_MODE_ALTERNATE(GPIOA_MTR_PHASE_R) |\
                                     PIN_MODE_ALTERNATE(GPIOA_MTR_ENABLE_R) |\
                                     PIN_MODE_ALTERNATE(GPIOA_MTR_TRA_L) |  \
                                     PIN_MODE_INPUT(GPIOA_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOA_DWM_SFD) |        \
                                     PIN_MODE_INPUT(GPIOA_DWM_WAKE_UP) |    \
                                     PIN_MODE_ALTERNATE(GPIOA_USB_DM) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_USB_DP) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |      \
                                     PIN_MODE_INPUT(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_VBAT_PROBE) | \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_LED_L) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_PHASE_L) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_ENABLE_L) \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_PHASE_R) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_ENABLE_R) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_MTR_TRA_L) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_DWM_SFD) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_DWM_WAKE_UP) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_DM) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_DP) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN15))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOA_VBAT_PROBE) | \
                                     PIN_OSPEED_VERYLOW(GPIOA_MTR_LED_L) |  \
                                     PIN_OSPEED_VERYLOW(GPIOA_MTR_PHASE_L) |\
                                     PIN_OSPEED_VERYLOW(GPIOA_MTR_ENABLE_L) \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN4) |       \
                                     PIN_OSPEED_HIGH(GPIOA_MTR_PHASE_R) |   \
                                     PIN_OSPEED_HIGH(GPIOA_MTR_ENABLE_R) |  \
                                     PIN_OSPEED_HIGH(GPIOA_MTR_TRA_L) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN8) |       \
                                     PIN_OSPEED_VERYLOW(GPIOA_DWM_SFD) |    \
                                     PIN_OSPEED_VERYLOW(GPIOA_DWM_WAKE_UP) |\
                                     PIN_OSPEED_HIGH(GPIOA_USB_DM) |        \
                                     PIN_OSPEED_VERYLOW(GPIOA_USB_DP) |     \
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |         \
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK) |         \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_VBAT_PROBE) | \
                                     PIN_PUPDR_PULLUP(GPIOA_MTR_LED_L) |    \
                                     PIN_PUPDR_PULLUP(GPIOA_MTR_PHASE_L) |  \
                                     PIN_PUPDR_PULLUP(GPIOA_MTR_ENABLE_L) | \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN4) |         \
                                     PIN_PUPDR_FLOATING(GPIOA_MTR_PHASE_R) |\
                                     PIN_PUPDR_PULLUP(GPIOA_MTR_ENABLE_R) | \
                                     PIN_PUPDR_FLOATING(GPIOA_MTR_TRA_L) |  \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_DWM_SFD) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_DWM_WAKE_UP) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_USB_DM) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_USB_DP) |     \
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN15))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_VBAT_PROBE) |       \
                                     PIN_ODR_HIGH(GPIOA_MTR_LED_L) |        \
                                     PIN_ODR_HIGH(GPIOA_MTR_PHASE_L) |      \
                                     PIN_ODR_HIGH(GPIOA_MTR_ENABLE_L) |     \
                                     PIN_ODR_HIGH(GPIOA_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOA_MTR_PHASE_R) |      \
                                     PIN_ODR_HIGH(GPIOA_MTR_ENABLE_R) |     \
                                     PIN_ODR_HIGH(GPIOA_MTR_TRA_L) |        \
                                     PIN_ODR_HIGH(GPIOA_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOA_DWM_SFD) |          \
                                     PIN_ODR_HIGH(GPIOA_DWM_WAKE_UP) |      \
                                     PIN_ODR_HIGH(GPIOA_USB_DM) |           \
                                     PIN_ODR_HIGH(GPIOA_USB_DP) |           \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |            \
                                     PIN_ODR_HIGH(GPIOA_SWCLK) |            \
                                     PIN_ODR_HIGH(GPIOA_PIN15))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_VBAT_PROBE, 0U) |    \
                                     PIN_AFIO_AF(GPIOA_MTR_LED_L, 0U) |     \
                                     PIN_AFIO_AF(GPIOA_MTR_PHASE_L, 0U) |   \
                                     PIN_AFIO_AF(GPIOA_MTR_ENABLE_L, 0U) |  \
                                     PIN_AFIO_AF(GPIOA_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_MTR_PHASE_R, 5U) |   \
                                     PIN_AFIO_AF(GPIOA_MTR_ENABLE_R, 5U) |  \
                                     PIN_AFIO_AF(GPIOA_MTR_TRA_L, 5U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_DWM_SFD, 0U) |       \
                                     PIN_AFIO_AF(GPIOA_DWM_WAKE_UP, 0U) |   \
                                     PIN_AFIO_AF(GPIOA_USB_DM, 14U) |       \
                                     PIN_AFIO_AF(GPIOA_USB_DP, 14U) |       \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_PIN15, 0U))

/*
 * GPIOB setup:
 *
 * PB0  - MTR_TRA_R                 (input pullup).
 * PB1  - LED_IR_C                  (input pullup).
 * PB2  - PIN2                      (input pullup).
 * PB3  - DWM_WAKEUP                (alternate 0).
 * PB4  - PIN4                      (input pullup).
 * PB5  - PIN5                      (input pullup).
 * PB6  - PIN6                      (alternate 4).
 * PB7  - MPU_INT                   (alternate 4).
 * PB8  - MPU_I2C_SCL               (input pullup).
 * PB9  - MPU_I2C_SDA               (input pullup).
 * PB10 - LED_IR_R                  (input pullup).
 * PB11 - LED_IR_L                  (input pullup).
 * PB12 - PIN12                     (input pullup).
 * PB13 - LED_SPI_CK                (input pullup).
 * PB14 - PIN14                     (input pullup).
 * PB15 - LED_SPI_DO                (input pullup).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_INPUT(GPIOB_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOB_LED_IR_C) |       \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_DWM_WAKEUP) | \
                                     PIN_MODE_INPUT(GPIOB_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN5) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_PIN6) |       \
                                     PIN_MODE_ALTERNATE(GPIOB_MPU_INT) |    \
                                     PIN_MODE_INPUT(GPIOB_MPU_I2C_SCL) |    \
                                     PIN_MODE_INPUT(GPIOB_MPU_I2C_SDA) |    \
                                     PIN_MODE_INPUT(GPIOB_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOB_LED_IR_L) |       \
                                     PIN_MODE_INPUT(GPIOB_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOB_LED_SPI_CK) |     \
                                     PIN_MODE_INPUT(GPIOB_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED_IR_C) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_DWM_WAKEUP) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN5) |       \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_PIN6) |      \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_MPU_INT) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_MPU_I2C_SCL) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_MPU_I2C_SDA) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED_IR_L) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED_SPI_CK) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOB_PIN0) |       \
                                     PIN_OSPEED_VERYLOW(GPIOB_LED_IR_C) |   \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN2) |       \
                                     PIN_OSPEED_HIGH(GPIOB_DWM_WAKEUP) |    \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN4) |       \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN5) |       \
                                     PIN_OSPEED_HIGH(GPIOB_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOB_MPU_INT) |       \
                                     PIN_OSPEED_VERYLOW(GPIOB_MPU_I2C_SCL) |\
                                     PIN_OSPEED_VERYLOW(GPIOB_MPU_I2C_SDA) |\
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN10) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_LED_IR_L) |   \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN12) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_LED_SPI_CK) | \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN14) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLUP(GPIOB_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_LED_IR_C) |     \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN2) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_DWM_WAKEUP) | \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN5) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_MPU_INT) |    \
                                     PIN_PUPDR_PULLUP(GPIOB_MPU_I2C_SCL) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_MPU_I2C_SDA) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_LED_IR_L) |     \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_LED_SPI_CK) |   \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOB_LED_IR_C) |         \
                                     PIN_ODR_HIGH(GPIOB_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOB_DWM_WAKEUP) |       \
                                     PIN_ODR_HIGH(GPIOB_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOB_MPU_INT) |          \
                                     PIN_ODR_HIGH(GPIOB_MPU_I2C_SCL) |      \
                                     PIN_ODR_HIGH(GPIOB_MPU_I2C_SDA) |      \
                                     PIN_ODR_HIGH(GPIOB_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOB_LED_IR_L) |         \
                                     PIN_ODR_HIGH(GPIOB_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOB_LED_SPI_CK) |       \
                                     PIN_ODR_HIGH(GPIOB_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_DWM_WAKEUP, 0U) |    \
                                     PIN_AFIO_AF(GPIOB_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN6, 4U) |          \
                                     PIN_AFIO_AF(GPIOB_MPU_INT,  4U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_MPU_I2C_SCL, 0U) |   \
                                     PIN_AFIO_AF(GPIOB_MPU_I2C_SDA, 0U) |   \
                                     PIN_AFIO_AF(GPIOB_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_LED_IR_L, 0U) |      \
                                     PIN_AFIO_AF(GPIOB_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_LED_SPI_CK, 0U) |    \
                                     PIN_AFIO_AF(GPIOB_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_LED_SPI, 0U))

/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (input pullup).
 * PC1  - PIN1                      (input pullup).
 * PC2  - PIN2                      (input pullup).
 * PC3  - PIN3                      (input pullup).
 * PC4  - MTR_LED_R                 (input pullup).
 * PC5  - PIN5                      (input pullup).
 * PC6  - PIN6                      (input pullup).
 * PC7  - PIN7                      (input pullup).
 * PC8  - PIN8                      (input pullup).
 * PC9  - PIN9                      (input pullup).
 * PC10 - DWM_SPI_CLK               (input pullup).
 * PC11 - DWM_SPI_MISO              (input pullup).
 * PC12 - DWM_SPI_MOSI              (input pullup).
 * PC13 - PIN13                     (input pullup).
 * PC14 - OSC32_IN                  (input floating).
 * PC15 - OSC32_OUT                 (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_MTR_LED_R) |      \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_DWM_SPI_CLK) |    \
                                     PIN_MODE_INPUT(GPIOC_DWM_SPI_MISO) |   \
                                     PIN_MODE_INPUT(GPIOC_DWM_SPI_MOSI) |   \
                                     PIN_MODE_INPUT(GPIOC_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOC_OSC32_IN) |       \
                                     PIN_MODE_INPUT(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_MTR_LED_R) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_DWM_SPI_CLK) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_DWM_SPI_MISO) \
                                     PIN_OTYPE_PUSHPULL(GPIOC_DWM_SPI_MOSI) \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_IN) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOC_PIN0) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN1) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN2) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN3) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_MTR_LED_R) |  \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN5) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN6) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN7) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN8) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN9) |       \
                                     PIN_OSPEED_VERYLOW(GPIOC_DWM_SPI_CLK) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_DWM_SPI_MISO) \
                                     PIN_OSPEED_VERYLOW(GPIOC_DWM_SPI_MOSI) \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN13) |      \
                                     PIN_OSPEED_HIGH(GPIOC_OSC32_IN) |      \
                                     PIN_OSPEED_HIGH(GPIOC_OSC32_OUT))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_MTR_LED_R) |    \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_DWM_SPI_CLK) |  \
                                     PIN_PUPDR_PULLUP(GPIOC_DWM_SPI_MISO) | \
                                     PIN_PUPDR_PULLUP(GPIOC_DWM_SPI_MOSI) | \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN13) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_IN) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_OUT))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_MTR_LED_R) |        \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_DWM_SPI_CLK) |      \
                                     PIN_ODR_HIGH(GPIOC_DWM_SPI_MISO) |     \
                                     PIN_ODR_HIGH(GPIOC_DWM_SPI_MOSI) |     \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOC_OSC32_IN) |         \
                                     PIN_ODR_HIGH(GPIOC_OSC32_OUT))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_MTR_LED_R, 0U) |     \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_DWM_SPI_CLK, 0U) |   \
                                     PIN_AFIO_AF(GPIOC_DWM_SPI_MISO, 0U) |  \
                                     PIN_AFIO_AF(GPIOC_DWM_SPI_MOSI, 0U) |  \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_OSC32_IN, 0U) |      \
                                     PIN_AFIO_AF(GPIOC_OSC32_OUT, 0U))

/*
 * GPIOD setup:
 *
 * PD2  - DWM_IRQ                   (input pullup).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_DWM_IRQ)
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_DWM_IRQ)
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOD_DWM_IRQ)
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_DWM_IRQ)
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_DWM_IRQ)
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_DWM_IR, 0U))

/*
 * GPIOF setup:
 *
 * PF0  - OSC_IN                    (input floating).
 * PF1  - OSC_OUT                   (input floating).
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOF_OSC_OUT))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_OSC_OUT))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_HIGH(GPIOF_OSC_IN) |        \
                                     PIN_OSPEED_HIGH(GPIOF_OSC_OUT))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_OSC_OUT))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOF_OSC_OUT))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_OSC_IN, 0U)  |       \
                                     PIN_AFIO_AF(GPIOF_OSC_OUT, 0U))

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
