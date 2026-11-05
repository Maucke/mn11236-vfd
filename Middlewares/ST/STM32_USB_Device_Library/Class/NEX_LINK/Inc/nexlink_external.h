#pragma once

#include "spi.h"
#include "usart.h"
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    SPI_HandleTypeDef *hspi;
    uint32_t clock_hz;
    uint8_t mode;
} nl_spi_bus_t;

typedef struct
{
    UART_HandleTypeDef *huart;
    uint32_t baudrate;
} nl_uart_bus_t;

typedef enum
{
    PIN_FREE = 0,
    PIN_I2C,
    PIN_SPI,
    PIN_UART,
    PIN_GPIO
} pin_owner_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    pin_owner_t owner;
} nl_pin_t;

#ifdef __cplusplus
}
#endif
