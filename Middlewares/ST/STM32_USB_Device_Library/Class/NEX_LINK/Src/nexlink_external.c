#include "nexlink_external.h"
#include "nexlink_app.h"
#include "nexlink_proto.h"
#include <string.h>

#define NL_SPI_MAX 1
#define NL_UART_MAX 2

static nl_spi_bus_t g_spi[NL_SPI_MAX] = {
    {
        .hspi = &hspi2,
        .clock_hz = 1000000,
        .mode = 0,
    },
};
static nl_pin_t g_pins[] =
{
    {GPIOB, GPIO_PIN_6, PIN_UART},   
    {GPIOB, GPIO_PIN_7, PIN_UART},  

    {GPIOD, GPIO_PIN_5, PIN_UART},   
    {GPIOD, GPIO_PIN_6, PIN_UART},  

    {GPIOC, GPIO_PIN_9, PIN_I2C},  
    {GPIOA, GPIO_PIN_8, PIN_I2C}, 

    {GPIOB, GPIO_PIN_8, PIN_I2C},   
    {GPIOB, GPIO_PIN_9, PIN_I2C},  

    {GPIOA, GPIO_PIN_4, PIN_SPI},  
    {GPIOB, GPIO_PIN_3, PIN_SPI},   
    {GPIOB, GPIO_PIN_4, PIN_SPI},  
    {GPIOA, GPIO_PIN_7, PIN_SPI},   
};
static pin_owner_t get_pin_owner(GPIO_TypeDef *port, uint16_t pin)
{
    for (int i = 0; i < sizeof(g_pins)/sizeof(g_pins[0]); i++)
    {
        if (g_pins[i].port == port &&
            g_pins[i].pin == pin)
        {
            return g_pins[i].owner;
        }
    }

    return PIN_FREE;
}
static GPIO_TypeDef* port_from_id(uint8_t id)
{
    switch (id)
    {
#ifdef GPIOA
    case 0: return GPIOA;
#endif

#ifdef GPIOB
    case 1: return GPIOB;
#endif

#ifdef GPIOC
    case 2: return GPIOC;
#endif

#ifdef GPIOD
    case 3: return GPIOD;
#endif

#ifdef GPIOE
    case 4: return GPIOE;
#endif

#ifdef GPIOF
    case 5: return GPIOF;
#endif

#ifdef GPIOG
    case 6: return GPIOG;
#endif

#ifdef GPIOH
    case 7: return GPIOH;
#endif

    default:
        return NULL;
    }
}
static void handle_gpio_config(nl_packet_t *pkt)
{
	if (pkt->length < 5)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    uint8_t portId = pkt->payload[0];

    uint16_t pin;
    memcpy(&pin, &pkt->payload[1], 2);

    uint8_t mode = pkt->payload[3];

    uint8_t pull = pkt->payload[4];

    GPIO_TypeDef *port = port_from_id(portId);
    if (!port)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    if (get_pin_owner(port, pin) != PIN_FREE)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_BUSY);
        return;
    }

    GPIO_InitTypeDef init = {0};

    init.Pin = pin;

    if (mode == 0)
        init.Mode = GPIO_MODE_OUTPUT_PP;
    else if (mode == 1)
        init.Mode = GPIO_MODE_INPUT;
    else
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    if (pull == 0)
        init.Pull = GPIO_NOPULL;
    else if (pull == 1)
        init.Pull = GPIO_PULLUP;
    else if (pull == 2)
        init.Pull = GPIO_PULLDOWN;
    else
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    HAL_GPIO_Init(port, &init);

    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}
static void handle_gpio_write(nl_packet_t *pkt)
{
    uint8_t portId = pkt->payload[0];

    uint16_t pin;
    memcpy(&pin, &pkt->payload[1], 2);

    uint8_t val = pkt->payload[3];

    GPIO_TypeDef *port = port_from_id(portId);

    if (get_pin_owner(port, pin) != PIN_FREE &&
        get_pin_owner(port, pin) != PIN_GPIO)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_BUSY);
        return;
    }

    HAL_GPIO_WritePin(port, pin,
                      val ? GPIO_PIN_SET : GPIO_PIN_RESET);

    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}
static void handle_gpio_read(nl_packet_t *pkt)
{
    uint8_t portId = pkt->payload[0];

    uint16_t pin;
    memcpy(&pin, &pkt->payload[1], 2);

    GPIO_TypeDef *port = port_from_id(portId);

    if (get_pin_owner(port, pin) != PIN_FREE &&
        get_pin_owner(port, pin) != PIN_GPIO)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_BUSY);
        return;
    }

    uint8_t val =
        HAL_GPIO_ReadPin(port, pin) ? 1 : 0;

    send_resp_ok(pkt->cmd, pkt->seq, &val, 1);
}

static int spi_reconfig(uint8_t bus,
                        uint32_t new_clk,
                        uint8_t mode,
                        uint8_t bitOrder)
{
    SPI_HandleTypeDef *hspi = g_spi[bus].hspi;

    if (HAL_SPI_DeInit(hspi) != HAL_OK)
        return -1;

    /* Mode 设置 */
    switch (mode)
    {
    case 0:
        hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
        hspi->Init.CLKPhase    = SPI_PHASE_1EDGE;
        break;
    case 1:
        hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
        hspi->Init.CLKPhase    = SPI_PHASE_2EDGE;
        break;
    case 2:
        hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
        hspi->Init.CLKPhase    = SPI_PHASE_1EDGE;
        break;
    case 3:
        hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
        hspi->Init.CLKPhase    = SPI_PHASE_2EDGE;
        break;
    }

    /* Bit order */
    if (bitOrder == 0)
        hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    else
        hspi->Init.FirstBit = SPI_FIRSTBIT_LSB;

    /* 计算 PCLK */
    uint32_t pclk =
        (hspi->Instance == SPI1) ?
        HAL_RCC_GetPCLK2Freq() :
        HAL_RCC_GetPCLK1Freq();

    uint32_t div_table[] = {2,4,8,16,32,64,128,256};
    uint32_t pres_table[] = {
        SPI_BAUDRATEPRESCALER_2,
        SPI_BAUDRATEPRESCALER_4,
        SPI_BAUDRATEPRESCALER_8,
        SPI_BAUDRATEPRESCALER_16,
        SPI_BAUDRATEPRESCALER_32,
        SPI_BAUDRATEPRESCALER_64,
        SPI_BAUDRATEPRESCALER_128,
        SPI_BAUDRATEPRESCALER_256
    };

    uint32_t prescaler = SPI_BAUDRATEPRESCALER_256;

    for (int i = 0; i < 8; i++)
    {
        if ((pclk / div_table[i]) <= new_clk)
        {
            prescaler = pres_table[i];
            break;
        }
    }

    hspi->Init.BaudRatePrescaler = prescaler;

    if (HAL_SPI_Init(hspi) != HAL_OK)
        return -2;

    return 0;
}

static void handle_spi_config(nl_packet_t *pkt)
{
    if (pkt->length != 7)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    uint8_t bus = pkt->payload[0];

    if (bus >= NL_SPI_MAX)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    uint32_t clk;
    memcpy(&clk, &pkt->payload[1], 4);

    uint8_t mode = pkt->payload[5];
    uint8_t bitOrder = pkt->payload[6];

    if (mode > 3 || bitOrder > 1)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    if (spi_reconfig(bus, clk, mode, bitOrder) != 0)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INTERNAL);
        return;
    }

    g_spi[bus].clock_hz = clk;
    g_spi[bus].mode = mode;

    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}

static void handle_spi_transfer(nl_packet_t *pkt)
{
    if (pkt->length < 5)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    uint8_t *p = pkt->payload;

    uint8_t bus = *p++;
    uint8_t cs = *p++;
    uint8_t flags = *p++;

    uint16_t len;
    memcpy(&len, p, 2);
    p += 2;

    if (bus >= NL_SPI_MAX || len > 512)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    SPI_HandleTypeDef *hspi = g_spi[bus].hspi;

    uint8_t rxbuf[512];

    HAL_SPI_TransmitReceive(
        hspi,
        p,
        rxbuf,
        len,
        1000);

    send_resp_ok(pkt->cmd, pkt->seq, rxbuf, len);
}

void external_handle_cmd(nl_packet_t *pkt)
{
    switch (pkt->cmd)
    {
    default:
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_UNSUPPORTED);
        break;
    }
}
