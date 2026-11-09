#include "fwupdate.h"
#include "nexlink_proto.h"
#include "main.h"
#include "iwdg.h"
#include <string.h>

/* Bootloader 区：扇区 0/1/2。扇区 3 从 0x0800C000 开始，是配置区，不能碰 */
#define FW_BOOT_BASE     0x08000000u
#define FW_BOOT_LIMIT    0x0800C000u

#define FW_BUF_SIZE      512u
#define FW_HDR_UNKNOWN   0xFFFFu

enum
{
    FW_IDLE = 0,
    FW_RECEIVING
};

typedef struct
{
    uint8_t  state;
    uint16_t hdr_len;      /* 0xFFFF = 还没判断 */
    uint32_t total;        /* 文件总长（含头） */
    uint32_t received;     /* 已收到的文件字节数 */
    uint32_t payload_len;  /* 文件去掉头以后的长度 */
    uint32_t written;      /* 已接受（含补齐）的 payload 字节数 */
    uint32_t badr;         /* 缓冲区第一个字节要写的 flash 地址 */
    uint32_t crc;          /* payload 的 CRC32（累加中） */
    uint16_t blen;
    uint8_t  buf[FW_BUF_SIZE];
} fw_ctx_t;

static fw_ctx_t fw;

static uint32_t crc32_step(uint32_t crc, const uint8_t *p, uint32_t n)
{
    while (n--)
    {
        crc ^= *p++;

        for (uint8_t k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
    }

    return crc;
}

/* 头长度：addheader.bat 写的是 "appheader vX.xx t...\r\n"，按第一个 \n 切 */
static uint16_t fw_header_len(const uint8_t *p, uint16_t n)
{
    if (n >= 10 && memcmp(p, "appheader", 9) == 0)
    {
        for (uint16_t i = 9; i < n && i < 64; i++)
            if (p[i] == '\n')
                return (uint16_t)(i + 1);

        return 32; /* 理论上到不了这：头只有 30 来个字节 */
    }

    return 0;
}

static uint8_t fw_erase_boot(void)
{
    FLASH_EraseInitTypeDef e;
    uint32_t bad = 0;

    HAL_FLASH_Unlock();

    for (uint32_t s = FLASH_SECTOR_0; s <= FLASH_SECTOR_2; s++)
    {
        /* 16KB 扇区擦除 ~250ms，IWDG 是 512ms，一个扇区喂一次 */
        HAL_IWDG_Refresh(&hiwdg);

        e.TypeErase = FLASH_TYPEERASE_SECTORS;
        e.Sector = s;
        e.NbSectors = 1;
        e.VoltageRange = FLASH_VOLTAGE_RANGE_3;

        if (HAL_FLASHEx_Erase(&e, &bad) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 0;
        }
    }

    HAL_IWDG_Refresh(&hiwdg);
    return 1;
}

static uint8_t fw_flush(void)
{
    if (fw.blen == 0)
        return 1;

    while (fw.blen & 3u)
    {
        fw.buf[fw.blen++] = 0xFF;
        fw.written++;
    }

    /* 512 字节 = 128 个字，编程 5ms 左右，顺手喂狗 */
    HAL_IWDG_Refresh(&hiwdg);

    for (uint32_t i = 0; i < fw.blen; i += 4)
    {
        uint32_t w;

        memcpy(&w, fw.buf + i, 4);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, fw.badr + i, w) != HAL_OK)
            return 0;
    }

    fw.blen = 0;
    return 1;
}

static uint8_t fw_push(const uint8_t *p, uint32_t n)
{
    while (n)
    {
        uint32_t room, take;

        if (fw.blen == 0)
            fw.badr = FW_BOOT_BASE + fw.written;

        room = FW_BUF_SIZE - fw.blen;
        take = (n < room) ? n : room;

        memcpy(fw.buf + fw.blen, p, take);
        fw.blen += (uint16_t)take;
        fw.written += take;
        p += take;
        n -= take;

        if (fw.blen == FW_BUF_SIZE && !fw_flush())
            return 0;
    }

    return 1;
}

uint8_t FwUpdate_Begin(uint8_t target, uint32_t total_len)
{
    /* 上一次没收完（上位机中途跑了）就作废重来，别把 flash 一直开着 */
    HAL_FLASH_Lock();

    if (target != FW_TARGET_BOOT)
        return NL_ERR_UNSUPPORTED;

    if (total_len < 1024u || total_len > (FW_BOOT_LIMIT - FW_BOOT_BASE) + 64u)
        return NL_ERR_INVALID_PARAM;

    memset(&fw, 0, sizeof(fw));

    fw.state = FW_RECEIVING;
    fw.hdr_len = FW_HDR_UNKNOWN;
    fw.total = total_len;
    fw.crc = 0xFFFFFFFFu;

    return NL_ERR_OK;
}

uint8_t FwUpdate_Data(uint32_t offset, const uint8_t *data, uint16_t len)
{
    uint32_t file_len = len;

    if (fw.state != FW_RECEIVING)
        return NL_ERR_NOT_READY;

    if (len == 0 || offset != fw.received || fw.received + len > fw.total)
        return NL_ERR_INVALID_PARAM;

    /* 第一个包：认头 + 向量表检查，都过了才擦 flash */
    if (fw.hdr_len == FW_HDR_UNKNOWN)
    {
        uint16_t h = fw_header_len(data, len);
        uint32_t sp, pc;

        if (len < (uint32_t)h + 8u)
            return NL_ERR_INVALID_PARAM;

        memcpy(&sp, data + h, 4);
        memcpy(&pc, data + h + 4, 4);

        if (sp < 0x20000000u || sp > 0x20010000u ||
            pc < FW_BOOT_BASE || pc >= FW_BOOT_LIMIT || (pc & 1u) == 0u)
            return NL_ERR_INVALID_PARAM;

        if ((fw.total - h) > (FW_BOOT_LIMIT - FW_BOOT_BASE))
            return NL_ERR_INVALID_PARAM;

        fw.hdr_len = h;
        fw.payload_len = fw.total - h;
				
				fw_erase_boot();
        if (!fw_erase_boot())
        {
            fw.state = FW_IDLE;
            return NL_ERR_INTERNAL;
        }
    }

    /* 头那几十个字节不写 flash */
    if (fw.received < fw.hdr_len)
    {
        uint32_t skip = fw.hdr_len - fw.received;

        if (skip >= len)
        {
            fw.received += len;
            return NL_ERR_OK;
        }

        data += skip;
        len -= (uint16_t)skip;
    }

    fw.crc = crc32_step(fw.crc, data, len);

    if (!fw_push(data, len))
    {
        HAL_FLASH_Lock();
        fw.state = FW_IDLE;
        return NL_ERR_INTERNAL;
    }

    fw.received += file_len;
    return NL_ERR_OK;
}

uint8_t FwUpdate_End(uint32_t crc32)
{
    uint8_t err = NL_ERR_OK;

    if (fw.state != FW_RECEIVING)
        return NL_ERR_NOT_READY;

    if (fw.received != fw.total || fw.hdr_len == FW_HDR_UNKNOWN)
    {
        HAL_FLASH_Lock();
        fw.state = FW_IDLE;
        return NL_ERR_INVALID_PARAM;
    }

    if (!fw_flush())
        err = NL_ERR_INTERNAL;

    HAL_FLASH_Lock();
    fw.state = FW_IDLE;

    if (err == NL_ERR_OK)
    {
        fw.crc ^= 0xFFFFFFFFu;

        if (fw.crc != crc32)
            err = NL_ERR_CRC;
        else if ((crc32_step(0xFFFFFFFFu, (const uint8_t *)FW_BOOT_BASE,
                             fw.payload_len) ^ 0xFFFFFFFFu) != crc32)
            err = NL_ERR_CRC; /* 回读不一致：写坏了 */
    }

    return err;
}
