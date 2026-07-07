#ifndef STM32F429_39XX_H
#define STM32F429_39XX_H

#ifdef __cplusplus
extern "C" {
#endif

#define	STM32_GPIO_PORT_A 0
#define	STM32_GPIO_PORT_B 1
#define	STM32_GPIO_PORT_C 2
#define	STM32_GPIO_PORT_D 3
#define	STM32_GPIO_PORT_E 4
#define	STM32_GPIO_PORT_F 5
#define	STM32_GPIO_PORT_G 6
#define	STM32_GPIO_PORT_H 7
#define	STM32_GPIO_PORT_I 8

#define STM32_GPIO_PIN_0 0
#define STM32_GPIO_PIN_1 1
#define STM32_GPIO_PIN_2 2
#define STM32_GPIO_PIN_3 3
#define STM32_GPIO_PIN_4 4
#define STM32_GPIO_PIN_5 5
#define STM32_GPIO_PIN_6 6
#define STM32_GPIO_PIN_7 7
#define STM32_GPIO_PIN_8 8
#define STM32_GPIO_PIN_9 9
#define STM32_GPIO_PIN_10 10
#define STM32_GPIO_PIN_11 11
#define STM32_GPIO_PIN_12 12
#define STM32_GPIO_PIN_13 13
#define STM32_GPIO_PIN_14 14
#define STM32_GPIO_PIN_15 15

typedef struct {
    unsigned int port : 16;
    unsigned int pin : 16;
}stm32_gpio_dsc_t;

typedef struct {
    unsigned int mode : 2;
    unsigned int otype : 1;
    unsigned int speed : 2;
    unsigned int pupd : 2;
    unsigned int af : 4;
}stm32_gpio_ctl_t;

typedef struct
{
    unsigned int CR;            /*!< RCC clock control register,                                  Address offset: 0x00 */
    unsigned int PLLCFGR;       /*!< RCC PLL configuration register,                              Address offset: 0x04 */
    unsigned int CFGR;          /*!< RCC clock configuration register,                            Address offset: 0x08 */
    unsigned int CIR;           /*!< RCC clock interrupt register,                                Address offset: 0x0C */
    unsigned int AHB1RSTR;      /*!< RCC AHB1 peripheral reset register,                          Address offset: 0x10 */
    unsigned int AHB2RSTR;      /*!< RCC AHB2 peripheral reset register,                          Address offset: 0x14 */
    unsigned int AHB3RSTR;      /*!< RCC AHB3 peripheral reset register,                          Address offset: 0x18 */
    unsigned int RESERVED0;     /*!< Reserved, 0x1C                                                                    */
    unsigned int APB1RSTR;      /*!< RCC APB1 peripheral reset register,                          Address offset: 0x20 */
    unsigned int APB2RSTR;      /*!< RCC APB2 peripheral reset register,                          Address offset: 0x24 */
    unsigned int RESERVED1[2];  /*!< Reserved, 0x28-0x2C                                                               */
    unsigned int AHB1ENR;       /*!< RCC AHB1 peripheral clock register,                          Address offset: 0x30 */
    unsigned int AHB2ENR;       /*!< RCC AHB2 peripheral clock register,                          Address offset: 0x34 */
    unsigned int AHB3ENR;       /*!< RCC AHB3 peripheral clock register,                          Address offset: 0x38 */
    unsigned int RESERVED2;     /*!< Reserved, 0x3C                                                                    */
    unsigned int APB1ENR;       /*!< RCC APB1 peripheral clock enable register,                   Address offset: 0x40 */
    unsigned int APB2ENR;       /*!< RCC APB2 peripheral clock enable register,                   Address offset: 0x44 */
    unsigned int RESERVED3[2];  /*!< Reserved, 0x48-0x4C                                                               */
    unsigned int AHB1LPENR;     /*!< RCC AHB1 peripheral clock enable in low power mode register, Address offset: 0x50 */
    unsigned int AHB2LPENR;     /*!< RCC AHB2 peripheral clock enable in low power mode register, Address offset: 0x54 */
    unsigned int AHB3LPENR;     /*!< RCC AHB3 peripheral clock enable in low power mode register, Address offset: 0x58 */
    unsigned int RESERVED4;     /*!< Reserved, 0x5C                                                                    */
    unsigned int APB1LPENR;     /*!< RCC APB1 peripheral clock enable in low power mode register, Address offset: 0x60 */
    unsigned int APB2LPENR;     /*!< RCC APB2 peripheral clock enable in low power mode register, Address offset: 0x64 */
    unsigned int RESERVED5[2];  /*!< Reserved, 0x68-0x6C                                                               */
    unsigned int BDCR;          /*!< RCC Backup domain control register,                          Address offset: 0x70 */
    unsigned int CSR;           /*!< RCC clock control & status register,                         Address offset: 0x74 */
    unsigned int RESERVED6[2];  /*!< Reserved, 0x78-0x7C                                                               */
    unsigned int SSCGR;         /*!< RCC spread spectrum clock generation register,               Address offset: 0x80 */
    unsigned int PLLI2SCFGR;    /*!< RCC PLLI2S configuration register,                           Address offset: 0x84 */
    unsigned int PLLSAICFGR;    /*!< RCC PLLSAI configuration register,                           Address offset: 0x88 */
    unsigned int DCKCFGR;       /*!< RCC Dedicated Clocks configuration register,                 Address offset: 0x8C */
} RCC_TypeDef;

#define PERIPH_BASE             ((unsigned int)0x40000000) /*!< Peripheral base address in the alias region                                */
#define AHB1PERIPH_BASE         (PERIPH_BASE + 0x00020000)
#define RCC_BASE                (AHB1PERIPH_BASE + 0x3800)
#define RCC                     ((RCC_TypeDef *) RCC_BASE)

#ifdef __cplusplus
}
#endif

#endif // STM32F429_39XX_H