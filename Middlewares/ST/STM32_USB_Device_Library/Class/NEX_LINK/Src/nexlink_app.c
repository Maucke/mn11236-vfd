#include "nexlink_app.h"
#include "nexlink_proto.h"
#include "nexlink_tx.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include "stm32_rx8900.h"
#include "mn11236.h"
#include "easy_ui_user_app.h"

#define NEXLINK_LOG_MAX_LEN 96

static uint8_t rx_buf[NL_MAX_PAYLOAD];
static uint16_t rx_len;

static uint64_t mcu_time_ms(void)
{
    TickType_t tick = xTaskGetTickCount();
    return (uint64_t)tick * (1000 / configTICK_RATE_HZ);
}

static void send_resp_internal(
    uint16_t cmd,
    uint16_t seq,
    uint8_t  status,
    const void *payload,
    uint16_t len)
{
    /* RESP payload = status(1) + data */
    if (len > NL_MAX_PAYLOAD - 1)
        return;

    uint16_t payload_len = 1 + len;
    uint16_t frame_len   = HEAD_LEN + payload_len;

    if (frame_len > TX_BUF_SIZE)
        return;

    uint8_t *tx = tx_buf_alloc();
    if (!tx)
        return;

    nl_packet_t *pkt = (nl_packet_t *)tx;
    pkt->magic  = NL_MAGIC;
    pkt->type   = NL_PKT_RESP;
    pkt->cmd    = cmd;
    pkt->seq    = seq;
    pkt->length = payload_len;

    uint8_t *p = pkt->payload;
    p[0] = status;

    if (len && payload)
        memcpy(p + 1, payload, len);

    nexlink_tx_send(tx, frame_len);
}

static void send_resp_err(
    uint16_t cmd,
    uint16_t seq,
    nl_err_t err)
{
    send_resp_internal(cmd, seq, (uint8_t)err, NULL, 0);
}

static void send_resp_ok(
    uint16_t cmd,
    uint16_t seq,
    const void *payload,
    uint16_t len)
{
    send_resp_internal(cmd, seq, NL_ERR_OK, payload, len);
}

static void send_event(
    uint16_t cmd,
    const void *payload,
    uint16_t len)
{
    if (len > NL_MAX_PAYLOAD)
        return;

    uint16_t frame_len = HEAD_LEN + len;
    if (frame_len > TX_BUF_SIZE)
        return;

    uint8_t *tx = tx_buf_alloc();
    if (!tx)
        return;

    nl_packet_t *pkt = (nl_packet_t *)tx;
    pkt->magic  = NL_MAGIC;
    pkt->type   = NL_PKT_EVENT;
    pkt->cmd    = cmd;
    pkt->seq    = 0;
    pkt->length = len;

    if (len && payload)
        memcpy(pkt->payload, payload, len);

    nexlink_tx_isr(tx, frame_len);
}

void nexlink_log(const char *fmt, ...)
{
    static char buf[NEXLINK_LOG_MAX_LEN];
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n <= 0)
        return;

    if (n >= sizeof(buf))
        n = sizeof(buf) - 1;

    send_event(EVT_LOG, buf, (uint16_t)n);
}

static void nexlink_cmd_loopback(const nl_packet_t *req)
{
    nl_err_t err = NL_ERR_OK;

    if (req->length > NL_MAX_PAYLOAD)
    {
        err = NL_ERR_INVALID_PARAM;
        send_resp_err(req->cmd, req->seq, err);
        return;
    }

    send_resp_ok(req->cmd, req->seq,
                 req->payload, req->length);
}

static const nl_display_info_t g_display_info =
{
    .display_count = 1,
    .displays =
    {
        {
            .width   = VFD_WIDTH,
            .height  = VFD_HEIGHT,
            .bpp     = Dual2Color,
            .refresh = 60,
        }
    }
};

extern uint8_t opnEnter, opnExit, opnUp, opnDown;

static uint16_t frame_width = 0;
static uint16_t frame_height = 0;
static uint8_t  frame_bpp = 0;

static uint32_t frame_expected_size = 0;
static uint32_t frame_received = 0;

#define MAX_FRAME_SIZE 1024   
static uint8_t frame_buffer[MAX_FRAME_SIZE];

static void handle_frame_start(nl_packet_t *pkt)
{
    if (pkt->length < 5)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }
		ExternUI_Init();
    memcpy(&frame_width,  pkt->payload + 0, 2);
    memcpy(&frame_height, pkt->payload + 2, 2);
    frame_bpp = pkt->payload[4];

    if (frame_bpp != Dual2Color)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }


    frame_expected_size = ((frame_width + 3) / 4) * frame_height;
    frame_received = 0;
	// nexlink_log("frame_expected_size: %d, frame_width: %d, frame_height: %d",frame_expected_size,frame_width,frame_height);

    if (frame_expected_size > MAX_FRAME_SIZE)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_NO_MEM);
        return;
    }

    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}

static void handle_frame_data(nl_packet_t *pkt)
{
	// nexlink_log("frame_expected_size: %d, frame_received: %d, pkt->length: %d",frame_expected_size,frame_received,pkt->length);
    if (frame_received + pkt->length > frame_expected_size)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }

    memcpy(frame_buffer + frame_received,
           pkt->payload,
           pkt->length);

    frame_received += pkt->length;

//    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}

static void handle_frame_end(nl_packet_t *pkt)
{
    if (frame_received != frame_expected_size)
    {
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
        return;
    }
		mn11236_clear_buffers();
    uint32_t index = 0;

    for (uint16_t y = 0; y < frame_height; y++)
    {
        for (uint16_t x = 0; x < frame_width; x += 4)
        {
            uint8_t byte = frame_buffer[index++];

            for (int i = 0; i < 4; i++)
            {
                uint8_t color = (byte >> (6 - i * 2)) & 0x03;

                uint16_t px = x + i;
                if (px < frame_width)
                {
									if(color)
									{
										if(color&1)
											mn11236_draw_pixel(px, y, MN11236_COLOR_RED);
										if(color&2)
											mn11236_draw_pixel(px, y, MN11236_COLOR_BLUE);
									}
									else
									{
                    mn11236_clear_pixel(px, y, MN11236_COLOR_RED);
                    mn11236_clear_pixel(px, y, MN11236_COLOR_BLUE);
									}
                }
            }
        }
    }
		mn11236_send_buffer();
    send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
}

static void handle_cmd(nl_packet_t *pkt)
{
    switch (pkt->cmd)
    {
    case CMD_HW_RESET:
        send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
				HAL_NVIC_SystemReset();
        break;
    case CMD_PING:
        send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
        break;

    case CMD_LOOPBACK:
        nexlink_cmd_loopback(pkt);
        break;

    case CMD_SYNC_TIME:
    {
        uint64_t pc_time = 0;

        if (pkt->length == sizeof(uint64_t))
            memcpy(&pc_time, pkt->payload, sizeof(uint64_t));

        uint64_t mcu_time = mcu_time_ms();

        struct tm *tm_local;
        long timestamp_s;
				timestamp_s = pc_time / 1000;
        timestamp_s += 3600 * 8;
        tm_local = localtime((const time_t *)&timestamp_s); 
        RX8900_SetTime(tm_local);
        send_resp_ok(pkt->cmd, pkt->seq, &mcu_time, sizeof(mcu_time));
        break;
    }
    case CMD_GET_VERSION:
    {
        nl_version_t ver = {
            .major = NL_VERSION_MAJOR,
            .minor = NL_VERSION_MINOR,
            .patch = NL_VERSION_PATCH,
            .build = NL_VERSION_BUILD,
        };

        send_resp_ok(pkt->cmd, pkt->seq, &ver, sizeof(ver));
        break;
    }

    case CMD_KEY:
				switch(pkt->payload[0])
				{
					case UpArrow:opnUp = true;break;
					case DownArrow:opnDown = true;break;
					case RightArrow:opnEnter = true;break;
					case LeftArrow:opnExit = true;break;
					default:
						send_resp_err(pkt->cmd, pkt->seq, NL_ERR_INVALID_PARAM);
					return;
				}
        send_resp_ok(pkt->cmd, pkt->seq, NULL, 0);
        break;

    case CMD_GET_DISPLAY_INFO:
        send_resp_ok(pkt->cmd, pkt->seq, &g_display_info, sizeof g_display_info);
        break;
		
		case CMD_FRAME_BEGIN:
				handle_frame_start(pkt);
				break;

		case CMD_FRAME_DATA:
				handle_frame_data(pkt);
				break;

		case CMD_FRAME_END:
				handle_frame_end(pkt);
				break;

    default:
        send_resp_err(pkt->cmd, pkt->seq, NL_ERR_UNSUPPORTED);
        break;
    }
}

void nexlink_rx_bytes(const uint8_t *data, uint16_t len)
{
    memcpy(rx_buf + rx_len, data, len);
    rx_len += len;

    while (rx_len >= HEAD_LEN)
    {
        if (rx_buf[0] != NL_MAGIC)
        {
            memmove(rx_buf, rx_buf + 1, --rx_len);
            continue;
        }

        uint16_t plen = *(uint16_t *)(rx_buf + 6);
        uint16_t total = HEAD_LEN + plen;
        if (rx_len < total)
            return;

        nl_packet_t *pkt = (nl_packet_t *)rx_buf;
        if (pkt->type == NL_PKT_CMD)
            handle_cmd(pkt);

        memmove(rx_buf, rx_buf + total, rx_len - total);
        rx_len -= total;
    }
}
