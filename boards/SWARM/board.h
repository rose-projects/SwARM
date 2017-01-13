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

//#define STM32_LSE_BYPASS
//#define STM32_HSE_BYPASS

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
#define LINE_DWM_CSn           	   PAL_LINE(GPIOA, 15U)

#define LINE_MTR_TRA_R             PAL_LINE(GPIOB, 0U)
#define LINE_LED_IR_C              PAL_LINE(GPIOB, 1U)
#define LINE_DWM_WAKEUP            PAL_LINE(GPIOB, 3U)
#define LINE_MPU_INT               PAL_LINE(GPIOB, 7U)
#define LINE_MPU_I2C_SCL           PAL_LINE(GPIOB, 8U)
#define LINE_MPU_I2C_SDA           PAL_LINE(GPIOB, 9U)
#define LINE_LED_IR_R              PAL_LINE(GPIOB, 10U)
#define LINE_LED_IR_L              PAL_LINE(GPIOB, 11U)
#define LINE_LED_CK                PAL_LINE(GPIOB, 13U)
#define LINE_LED_DO                PAL_LINE(GPIOB, 15U)

#define LINE_MTR_LED_R             PAL_LINE(GPIOC, 4U)
#define LINE_DWM_CLK               PAL_LINE(GPIOC, 10U)
#define LINE_DWM_MISO              PAL_LINE(GPIOC, 11U)
#define LINE_DWM_MOSI              PAL_LINE(GPIOC, 12U)

#define LINE_DWM_IRQ               PAL_LINE(GPIOD, 2U)

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

// [MODE OTYPE OSPEED PUPDR]

/*
 * GPIOA setup:
 *
 * PA0  - VBAT_PROBE                (analog).
 * PA1  - MTR_LED_L                 (output push-pull).
 * PA2  - MTR_PHASE_L               (output push-pull).
 * PA3  - MTR_ENABLE_L              (alternate 9).
 * PA4  - PIN4                      (input floating).
 * PA5  - MTR_PHASE_R               (output push-pull).
 * PA6  - MTR_ENABLE_R              (alternate 1).
 * PA7  - MTR_TRA_L                 (analog) comparator ADC
 * PA8  - PIN8                      (input floating).
 * PA9  - USB_VBUS                  (input floating) auto set by usb init
 * PA10 - USB_CONNECT               (input floating) auto set by usb init
 * PA11 - USB_DM                    (input floating) auto set by usb init
 * PA12 - USB_DP                    (input floating) auto set by usb init
 * PA13 - SWDIO                     (alternate 0 pull-up).
 * PA14 - SWCLK                     (alternate 0 pull-down).
 * PA15 - DWM_SPI_CSn               (output push-pull).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ANALOG(GPIOA_VBAT_PROBE) |    \
                                     PIN_MODE_OUTPUT(GPIOA_MTR_LED_L)   |   \
                                     PIN_MODE_OUTPUT(GPIOA_MTR_PHASE_L) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_MTR_ENABLE_L)|\
                                     PIN_MODE_INPUT(GPIOA_PIN4) |           \
                                     PIN_MODE_OUTPUT(GPIOA_MTR_PHASE_R) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_MTR_ENABLE_R)|\
                                     PIN_MODE_ANALOG(GPIOA_MTR_TRA_L) |     \
                                     PIN_MODE_INPUT(GPIOA_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOA_USB_VBUS) |       \
                                     PIN_MODE_INPUT(GPIOA_USB_CONNECT) |    \
                                     PIN_MODE_INPUT(GPIOA_USB_DM) |         \
                                     PIN_MODE_INPUT(GPIOA_USB_DP) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |      \
                                     PIN_MODE_OUTPUT(GPIOA_DWM_SPI_CSn))
#define VAL_GPIOA_OTYPER    0x00000000
#define VAL_GPIOA_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_PULLUP(GPIOA_SWDIO) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK))
#define VAL_GPIOA_ODR       0x00000000
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_MTR_ENABLE_L, 9U) |  \
                                     PIN_AFIO_AF(GPIOA_MTR_ENABLE_R, 1U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_SWDIO, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U))

/*
 * GPIOB setup:
 *
 * PB0  - MTR_TRA_R                 (analog).
 * PB1  - LED_IR_C                  (output push-pull).
 * PB2  - PIN2                      (input floating).
 * PB3  - DWM_WAKEUP                (output push-pull).
 * PB4  - PIN4                      (input floating).
 * PB5  - PIN5                      (input floating).
 * PB6  - PIN6                      (input floating).
 * PB7  - MPU_INT                   (input pull-down).
 * PB8  - MPU_I2C_SCL               (alternate 4 open drain).
 * PB9  - MPU_I2C_SDA               (alternate 4 open drain).
 * PB10 - LED_IR_R                  (output push-pull).
 * PB11 - LED_IR_L                  (output push-pull).
 * PB12 - PIN12                     (input floating).
 * PB13 - LED_SPI_CK                (alternate 5).
 * PB14 - PIN14                     (input floating).
 * PB15 - LED_SPI_DO                (alternate 5).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_MTR_TRA_R) |     \
                                     PIN_MODE_OUTPUT(GPIOB_LED_IR_C) |      \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |           \
                                     PIN_MODE_OUTPUT(GPIOB_DWM_WAKEUP) |    \
                                     PIN_MODE_INPUT(GPIOB_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOB_MPU_INT) |        \
                                     PIN_MODE_ALTERNATE(GPIOB_MPU_I2C_SCL) |\
                                     PIN_MODE_ALTERNATE(GPIOB_MPU_I2C_SDA) |\
                                     PIN_MODE_OUTPUT(GPIOB_LED_IR_R) |      \
                                     PIN_MODE_OUTPUT(GPIOB_LED_IR_L) |      \
                                     PIN_MODE_INPUT(GPIOB_PIN12) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_LED_SPI_CK) | \
                                     PIN_MODE_INPUT(GPIOB_PIN14) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_LED_SPI_DO))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_OPENDRAIN(GPIOB_MPU_I2C_SCL) |\
                                     PIN_OTYPE_OPENDRAIN(GPIOB_MPU_I2C_SDA))
#define VAL_GPIOB_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLDOWN(GPIOB_MPU_INT))
#define VAL_GPIOB_ODR       0x00000000
#define VAL_GPIOB_AFRL      0x00000000
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_MPU_I2C_SCL, 4U) |   \
                                     PIN_AFIO_AF(GPIOB_MPU_I2C_SDA, 4U) |   \
                                     PIN_AFIO_AF(GPIOB_LED_SPI_CK, 5U) |    \
                                     PIN_AFIO_AF(GPIOB_LED_SPI_DO, 5U))

/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (input floating).
 * PC1  - PIN1                      (input floating).
 * PC2  - PIN2                      (input floating).
 * PC3  - PIN3                      (input floating).
 * PC4  - MTR_LED_R                 (output push-pull).
 * PC5  - PIN5                      (input floating).
 * PC6  - PIN6                      (input floating).
 * PC7  - PIN7                      (input floating).
 * PC8  - PIN8                      (input floating).
 * PC9  - PIN9                      (input floating).
 * PC10 - DWM_SPI_CLK               (alternate 6).
 * PC11 - DWM_SPI_MISO              (alternate 6).
 * PC12 - DWM_SPI_MOSI              (alternate 6).
 * PC13 - PIN13                     (input floating).
 * PC14 - OSC32_IN                  (input floating).
 * PC15 - OSC32_OUT                 (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_OUTPUT(GPIOC_MTR_LED_R) |     \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_ALTERNATE(GPIOC_DWM_SPI_CLK) |\
                                     PIN_MODE_ALTERNATE(GPIOC_DWM_SPI_MISO)|\
                                     PIN_MODE_ALTERNATE(GPIOC_DWM_SPI_MOSI)|\
                                     PIN_MODE_INPUT(GPIOC_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOC_OSC32_IN) |       \
                                     PIN_MODE_INPUT(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OTYPER    0x00000000
#define VAL_GPIOC_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOC_PUPDR     0x00000000
#define VAL_GPIOC_ODR       0x00000000
#define VAL_GPIOC_AFRL      0x00000000
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_DWM_SPI_CLK, 6U) |   \
                                     PIN_AFIO_AF(GPIOC_DWM_SPI_MISO, 6U) |  \
                                     PIN_AFIO_AF(GPIOC_DWM_SPI_MOSI, 6U))

/*
 * GPIOD setup:
 *
 * PD2  - DWM_IRQ                   (input pull-down).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_DWM_IRQ))
#define VAL_GPIOD_OTYPER            0x00000000
#define VAL_GPIOD_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLDOWN(GPIOD_DWM_IRQ))
#define VAL_GPIOD_ODR               0x00000000
#define VAL_GPIOD_AFRL              0x00000000
#define VAL_GPIOD_AFRH              0x00000000

/*
 * GPIOF setup:
 *
 * PF0  - OSC_IN                    (input floating).
 * PF1  - OSC_OUT                   (input floating).
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOF_OSC_OUT))
#define VAL_GPIOF_OTYPER    0x00000000
#define VAL_GPIOF_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_OSC_OUT))
#define VAL_GPIOF_ODR       0x00000000
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_OSC_IN, 0U)  |       \
                                     PIN_AFIO_AF(GPIOF_OSC_OUT, 0U))
#define VAL_GPIOF_AFRH      0x00000000

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
