/*!
 * @file    animation.c
 * @brief   MOTION 界面的 8 个动画（单选，112 x 18 双通道 8 级灰度）
 *
 * 设计约束：
 *  - 屏幕是 112x18 的横幅，所有运动都顺着横向 / 纵向 / 放射三个方向做；
 *  - 每帧由调用方先 clear_gram()，本文件只负责画像素，不保留上一帧；
 *  - 只有当前选中的那一个动画在使用状态，所以所有动画共用一块 union。
 */

#include "animation.h"
#include "main.h"
#include "clockui.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define W MAX_X
#define H MAX_Y

/* ============================================================
 *  公共小工具
 * ============================================================ */

static uint8_t anim_cur = 0xFF;
static uint32_t anim_frame;

static void px(int x, int y, Color c, int g)
{
    if (g <= 0)
        return;
    draw_point_gray(x, y, c, (uint8_t)(g > 7 ? 7 : g));
}

static float rndf(float a, float b)
{
    return a + (float)(rand() % 10000) * (b - a) / 10000.0f;
}

static void draw_line(int x0, int y0, int x1, int y1, Color c, int g)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int n = dx > dy ? dx : dy;

    if (n <= 0)
    {
        px(x0, y0, c, g);
        return;
    }

    for (int i = 0; i <= n; i++)
        px(x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, c, g);
}

/* ============================================================
 *  状态（只有一个动画在跑，共用一块内存）
 * ============================================================ */

#define FLY_N 10
#define RIP_N 3
#define RAIN_N 56
#define CITY_N 26
#define STAR_N 30
#define WARP_N 46
#define BUB_N 15
#define FW_P_N 40

static union
{
    struct
    {
        float x, y, ph, w;
        uint8_t c;
    } fly[FLY_N];

    struct
    {
        float x, y, r;
    } rip[RIP_N];

    struct
    {
        float y, sp;
        uint8_t len;
    } rain[RAIN_N];

    struct
    {
        int8_t b[W];
        uint16_t t;
    } ecg;

    struct
    {
        struct
        {
            int16_t x;
            uint8_t w, h, seed;
        } b[CITY_N];
        struct
        {
            uint8_t x, y, ph;
        } st[STAR_N];
        int16_t total;
        float off;
    } city;

    struct
    {
        float a, d;
    } warp[WARP_N];

    struct
    {
        float x, y, ph, sp;
        uint8_t r;
    } bub[BUB_N];

    struct
    {
        struct
        {
            float x, y, vx, vy;
            uint8_t life, c;
        } p[FW_P_N];
        struct
        {
            float x, y, vy, ty;
        } rk;
        uint8_t n, rk_on, burst;
        int16_t timer;
    } fw;
} st;

/* ============================================================
 *  1  萤火虫：慢速游荡 + 呼吸闪烁
 * ============================================================ */

static void fly_init(void)
{
    for (int i = 0; i < FLY_N; i++)
    {
        st.fly[i].x = rndf(0.0f, (float)W);
        st.fly[i].y = rndf(0.0f, (float)H);
        st.fly[i].ph = rndf(0.0f, 6.2832f);
        st.fly[i].w = rndf(0.03f, 0.07f);
        st.fly[i].c = (i < 6) ? Red : Cyan;
    }
}

static void fly_step(void)
{
    for (int i = 0; i < FLY_N; i++)
    {
        st.fly[i].ph += st.fly[i].w;
        st.fly[i].x += cosf(st.fly[i].ph * 1.7f) * 0.5f;
        st.fly[i].y += sinf(st.fly[i].ph * 2.3f) * 0.18f;

        if (st.fly[i].x < 0.0f)
            st.fly[i].x += (float)W;
        else if (st.fly[i].x >= (float)W)
            st.fly[i].x -= (float)W;

        if (st.fly[i].y < 0.0f)
            st.fly[i].y += (float)H;
        else if (st.fly[i].y >= (float)H)
            st.fly[i].y -= (float)H;
    }
}

static void fly_draw(void)
{
    for (int i = 0; i < FLY_N; i++)
    {
        int g = (int)(3.0f + 4.0f * sinf(st.fly[i].ph * 2.4f) + 0.5f);
        int x = (int)(st.fly[i].x + 0.5f);
        int y = (int)(st.fly[i].y + 0.5f);
        int hg = g * 45 / 100;
        Color c = (Color)st.fly[i].c;

        if (hg < 1)
            hg = 1;

        px(x, y, c, g);
        px(x - 1, y, c, hg);
        px(x + 1, y, c, hg);
        px(x, y - 1, c, hg);
        px(x, y + 1, c, hg);
    }
}

/* ============================================================
 *  2  涟漪：椭圆水波扩散
 * ============================================================ */

static void rip_init(void)
{
    for (int i = 0; i < RIP_N; i++)
    {
        st.rip[i].x = rndf(12.0f, (float)(W - 12));
        st.rip[i].y = rndf(7.0f, 12.0f);
        st.rip[i].r = rndf(0.0f, 32.0f);
    }
}

static void rip_step(void)
{
    for (int i = 0; i < RIP_N; i++)
    {
        st.rip[i].r += 0.55f;

        if (st.rip[i].r > 34.0f)
        {
            st.rip[i].r = 0.0f;
            st.rip[i].x = rndf(12.0f, (float)(W - 12));
            st.rip[i].y = rndf(7.0f, 12.0f);
        }
    }
}

static void rip_draw(void)
{
    for (int i = 0; i < RIP_N; i++)
    {
        float r = st.rip[i].r;
        int g = (int)(7.0f * (1.0f - r / 34.0f) + 0.5f);
        int n;

        if (g <= 0)
            continue;

        n = 14 + (int)(r * 6.0f);

        for (int k = 0; k < n; k++)
        {
            float a = (float)k * 6.2832f / (float)n;

            px((int)(st.rip[i].x + cosf(a) * r + 0.5f),
               (int)(st.rip[i].y + sinf(a) * r * 0.5f + 0.5f), Cyan, g);
        }

        px((int)(st.rip[i].x + 0.5f), (int)(st.rip[i].y + 0.5f), Red, g / 2 + 1);
    }
}

/* ============================================================
 *  3  数字雨：56 列下落光柱
 * ============================================================ */

static void rain_init(void)
{
    for (int i = 0; i < RAIN_N; i++)
    {
        st.rain[i].y = -rndf(0.0f, (float)H * 1.6f);
        st.rain[i].sp = rndf(0.35f, 0.9f);
        st.rain[i].len = (uint8_t)(3 + rand() % 6);
    }
}

static void rain_step(void)
{
    for (int i = 0; i < RAIN_N; i++)
    {
        st.rain[i].y += st.rain[i].sp;

        if (st.rain[i].y - (float)st.rain[i].len > (float)H)
        {
            st.rain[i].y = -rndf(0.0f, 6.0f);
            st.rain[i].sp = rndf(0.35f, 0.9f);
            st.rain[i].len = (uint8_t)(3 + rand() % 6);
        }
    }
}

static void rain_draw(void)
{
    for (int i = 0; i < RAIN_N; i++)
    {
        int x = i * 2;
        int head = (int)st.rain[i].y;

        for (int k = 0; k < st.rain[i].len; k++)
        {
            int y = head - k;

            if (y < 0 || y >= H)
                continue;

            if (k == 0)
                px(x, y, White, 7);
            else
                px(x, y, Cyan, 6 - k);
        }
    }
}

/* ============================================================
 *  4  心电：横向滚动波形
 * ============================================================ */

static int8_t ecg_sample(uint16_t t)
{
    uint8_t p = (uint8_t)(t % 34);

    if (p >= 5 && p < 9)
        return 2;
    if (p == 16)
        return -2;
    if (p == 18)
        return -4;
    if (p == 19)
        return 9;
    if (p == 20)
        return -5;
    if (p == 21)
        return 1;
    if (p >= 26 && p < 31)
        return 3;

    return 0;
}

static void ecg_init(void)
{
    memset(st.ecg.b, 0, W);
    st.ecg.t = 0;
}

static void ecg_step(void)
{
    memmove(st.ecg.b, st.ecg.b + 1, W - 1);
    st.ecg.b[W - 1] = ecg_sample(st.ecg.t++);
}

static void ecg_draw(void)
{
    const int base = 11;

    for (int i = 0; i < W; i++)
    {
        int y1 = base - st.ecg.b[i];
        int y0 = (i > 0) ? base - st.ecg.b[i - 1] : y1;
        int lo = (y0 < y1) ? y0 : y1;
        int hi = (y0 > y1) ? y0 : y1;

        for (int y = lo; y <= hi; y++)
            px(i, y, Cyan, 4);
    }

    for (int i = W - 10; i < W; i++)
        if (st.ecg.b[i] >= 8)
            px(i, base - st.ecg.b[i], Red, 7);

    px(W - 1, base - st.ecg.b[W - 1], White, 7);
}

/* ============================================================
 *  5  天际线：滚动城市
 * ============================================================ */

static uint32_t city_hash(int x, int y, int seed)
{
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ (uint32_t)seed * 83492791u;

    h ^= h >> 13;
    h *= 2654435761u;
    h ^= h >> 16;

    return h;
}

static void city_init(void)
{
    int x = 0;

    for (int i = 0; i < CITY_N; i++)
    {
        st.city.b[i].w = (uint8_t)(3 + rand() % 7);
        st.city.b[i].h = (uint8_t)(2 + rand() % 9);
        st.city.b[i].x = (int16_t)x;
        st.city.b[i].seed = (uint8_t)(rand() % 97);
        x += st.city.b[i].w + 1 + rand() % 3;
    }

    st.city.total = (int16_t)x;
    st.city.off = 0.0f;

    for (int i = 0; i < STAR_N; i++)
    {
        st.city.st[i].x = (uint8_t)(rand() % W);
        st.city.st[i].y = (uint8_t)(rand() % 10);
        st.city.st[i].ph = (uint8_t)(rand() % 64);
    }
}

static void city_step(void)
{
    st.city.off += 0.35f;

    if (st.city.off >= (float)st.city.total)
        st.city.off -= (float)st.city.total;

    for (int i = 0; i < STAR_N; i++)
        st.city.st[i].ph++;
}

static void city_draw(void)
{
    /* 地面 */
    for (int x = 0; x < W; x++)
        px(x, H - 1, White, 1);

    /* 星星 */
    for (int i = 0; i < STAR_N; i++)
        px(st.city.st[i].x, st.city.st[i].y, White,
           2 + ((st.city.st[i].ph >> 4) & 1));

    /* 建筑 */
    for (int i = 0; i < CITY_N; i++)
    {
        for (int rep = 0; rep < 2; rep++)
        {
            int bx = (int)((float)st.city.b[i].x - st.city.off + 0.5f) + rep * st.city.total;
            int top, bw = st.city.b[i].w;

            if (bx > W || bx + bw < 0)
                continue;

            top = H - 1 - st.city.b[i].h;

            for (int dx = 0; dx < bw; dx++)
            {
                int x = bx + dx;

                if (x < 0 || x >= W)
                    continue;

                for (int y = top; y < H - 1; y++)
                    px(x, y, White, (y == top) ? 2 : 1);

                for (int wy = top + 2; wy < H - 2; wy += 3)
                    for (int wx = 1; wx < bw - 1; wx += 2)
                        if (city_hash(wx, wy, st.city.b[i].seed) & 0x20)
                            px(bx + wx, wy, Cyan, 5);
            }

            if (st.city.b[i].h > 7 && (((anim_frame >> 4) + st.city.b[i].seed) & 1))
                px(bx + bw / 2, top - 1, Red, 7);
        }
    }
}

/* ============================================================
 *  6  星空穿越：从中心向外的星轨
 * ============================================================ */

static void warp_init(void)
{
    for (int i = 0; i < WARP_N; i++)
    {
        st.warp[i].a = rndf(0.0f, 6.2832f);
        st.warp[i].d = rndf(0.0f, 1.0f);
    }
}

static void warp_step(void)
{
    for (int i = 0; i < WARP_N; i++)
    {
        st.warp[i].d += 0.012f + st.warp[i].d * 0.045f;

        if (st.warp[i].d > 1.0f)
        {
            st.warp[i].d = 0.03f;
            st.warp[i].a = rndf(0.0f, 6.2832f);
        }
    }
}

static void warp_draw(void)
{
    for (int i = 0; i < WARP_N; i++)
    {
        float r = st.warp[i].d;
        float pr = r - (0.05f + r * 0.16f);
        float ca = cosf(st.warp[i].a);
        float sa = sinf(st.warp[i].a);

        if (pr < 0.02f)
            pr = 0.02f;

        draw_line((int)((float)(W / 2) + ca * pr * 55.0f + 0.5f),
                  (int)((float)(H / 2) + sa * pr * 8.6f + 0.5f),
                  (int)((float)(W / 2) + ca * r * 55.0f + 0.5f),
                  (int)((float)(H / 2) + sa * r * 8.6f + 0.5f),
                  White, (int)(2.0f + r * 5.0f));
    }
}

/* ============================================================
 *  7  气泡：上浮 + 摇摆
 * ============================================================ */

static void bub_init(void)
{
    for (int i = 0; i < BUB_N; i++)
    {
        st.bub[i].x = rndf(0.0f, (float)W);
        st.bub[i].y = rndf(0.0f, (float)H);
        st.bub[i].ph = rndf(0.0f, 6.2832f);
        st.bub[i].sp = rndf(0.2f, 0.55f);
        st.bub[i].r = (uint8_t)(1 + rand() % 2);
    }
}

static void bub_step(void)
{
    for (int i = 0; i < BUB_N; i++)
    {
        st.bub[i].y -= st.bub[i].sp;
        st.bub[i].ph += 0.07f;
        st.bub[i].x += sinf(st.bub[i].ph) * 0.22f;

        if (st.bub[i].y < -2.0f)
        {
            st.bub[i].y = (float)H + 1.0f + rndf(0.0f, 3.0f);
            st.bub[i].x = rndf(0.0f, (float)W);
            st.bub[i].r = (uint8_t)(1 + rand() % 2);
        }
    }
}

static void bub_draw(void)
{
    for (int i = 0; i < BUB_N; i++)
    {
        int x = (int)(st.bub[i].x + 0.5f);
        int y = (int)(st.bub[i].y + 0.5f);

        if (y < -2 || y > H + 1)
            continue;

        if (st.bub[i].r < 2)
        {
            px(x, y, Cyan, 4);
            px(x, y - 1, Cyan, 3);
        }
        else
        {
            px(x - 1, y, Cyan, 4);
            px(x + 1, y, Cyan, 4);
            px(x, y - 1, Cyan, 3);
            px(x, y + 1, Cyan, 3);
            px(x - 1, y - 1, Cyan, 2);
            px(x + 1, y - 1, Cyan, 2);
            px(x - 1, y + 1, Cyan, 2);
            px(x + 1, y + 1, Cyan, 2);
        }

        px(x, y - 1, White, 7);
    }
}

/* ============================================================
 *  8  烟花：升空 -> 爆炸 -> 重力拖尾
 * ============================================================ */

static void fw_init(void)
{
    st.fw.n = 0;
    st.fw.rk_on = 0;
    st.fw.burst = 0;
    st.fw.timer = 0;
}

static void fw_launch(void)
{
    st.fw.rk.x = rndf(14.0f, (float)(W - 14));
    st.fw.rk.y = (float)(H - 1);
    st.fw.rk.vy = -rndf(1.5f, 2.2f);
    st.fw.rk.ty = rndf(2.0f, 6.0f);
    st.fw.rk_on = 1;
}

static void fw_add(float x, float y, float vx, float vy, uint8_t life, uint8_t c)
{
    if (st.fw.n >= FW_P_N)
        return;

    st.fw.p[st.fw.n].x = x;
    st.fw.p[st.fw.n].y = y;
    st.fw.p[st.fw.n].vx = vx;
    st.fw.p[st.fw.n].vy = vy;
    st.fw.p[st.fw.n].life = life;
    st.fw.p[st.fw.n].c = c;
    st.fw.n++;
}

static void fw_step(void)
{
    if (!st.fw.rk_on)
    {
        if (--st.fw.timer <= 0)
            fw_launch();
    }
    else
    {
        st.fw.rk.y += st.fw.rk.vy;
        st.fw.rk.vy += 0.012f;

        if (st.fw.rk.y <= st.fw.rk.ty)
        {
            uint8_t c = (st.fw.burst++ & 1) ? Red : Cyan;

            for (int i = 0; i < 26; i++)
            {
                float a = (float)i * 6.2832f / 26.0f + rndf(0.0f, 0.12f);
                float sp = rndf(0.45f, 1.5f);

                fw_add(st.fw.rk.x, st.fw.rk.y,
                       cosf(a) * sp, sinf(a) * sp * 0.85f,
                       (uint8_t)(16 + rand() % 12), c);
            }

            for (int i = 0; i < 7; i++)
            {
                float a = rndf(0.0f, 6.2832f);
                float sp = rndf(0.2f, 0.8f);

                fw_add(st.fw.rk.x, st.fw.rk.y,
                       cosf(a) * sp, sinf(a) * sp,
                       (uint8_t)(10 + rand() % 8), White);
            }

            st.fw.rk_on = 0;
            st.fw.timer = (int16_t)(14 + rand() % 20);
        }
    }

    for (uint8_t i = 0; i < st.fw.n;)
    {
        st.fw.p[i].vy += 0.055f;
        st.fw.p[i].vx *= 0.992f;
        st.fw.p[i].x += st.fw.p[i].vx;
        st.fw.p[i].y += st.fw.p[i].vy;

        if (st.fw.p[i].life > 0)
            st.fw.p[i].life--;

        if (st.fw.p[i].life == 0 || st.fw.p[i].y > (float)H + 3.0f)
        {
            st.fw.n--;
            st.fw.p[i] = st.fw.p[st.fw.n];
            continue;
        }
        i++;
    }
}

static void fw_draw(void)
{
    if (st.fw.rk_on)
    {
        for (int i = 0; i < 4; i++)
            px((int)(st.fw.rk.x + 0.5f), (int)(st.fw.rk.y + 0.5f) + i,
               i == 0 ? White : Red, 7 - i * 2);
    }

    for (uint8_t i = 0; i < st.fw.n; i++)
    {
        int g = (int)((float)st.fw.p[i].life * 0.45f);

        if (g > 7)
            g = 7;

        px((int)(st.fw.p[i].x + 0.5f), (int)(st.fw.p[i].y + 0.5f),
           (Color)st.fw.p[i].c, g);
        px((int)(st.fw.p[i].x - st.fw.p[i].vx + 0.5f),
           (int)(st.fw.p[i].y - st.fw.p[i].vy + 0.5f),
           (Color)st.fw.p[i].c, g - 3);
    }
}

/* ============================================================
 *  调度
 * ============================================================ */

typedef struct
{
    void (*init)(void);
    void (*step)(void);
    void (*draw)(void);
} Anim_t;

static const Anim_t anims[ANIM_COUNT] = {
    {fly_init, fly_step, fly_draw},   /* 0 Firefly */
    {rip_init, rip_step, rip_draw},   /* 1 Ripple */
    {rain_init, rain_step, rain_draw},/* 2 Matrix */
    {ecg_init, ecg_step, ecg_draw},   /* 3 ECG */
    {city_init, city_step, city_draw},/* 4 Skyline */
    {warp_init, warp_step, warp_draw},/* 5 Warp */
    {bub_init, bub_step, bub_draw},   /* 6 Bubble */
    {fw_init, fw_step, fw_draw},      /* 7 Firework */
};

void Motion_Init(void)
{
    srand(HAL_GetTick());
    anim_cur = 0xFF; /* 下一帧自动初始化 0 号 */
    anim_frame = 0;
}

void Motion_Run(uint8_t id)
{
    if (id >= ANIM_COUNT)
        id = 0;

    if (id != anim_cur)
    {
        anim_cur = id;
        anim_frame = 0;
        anims[id].init();
    }

    anims[id].step();
    anims[id].draw();
    anim_frame++;
}
