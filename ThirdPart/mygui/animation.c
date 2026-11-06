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

    /* ---- 第二批 ---- */

    struct
    {
        float t;
    } helix;

    struct
    {
        float ax, ay;
    } cube;

#define TUN_N 10
    struct
    {
        float z[TUN_N];
        float t;
    } tun;

#define GEAR_N 8
    struct
    {
        struct
        {
            float x, r, sp;
            uint8_t n;
            int8_t dir;
        } g[GEAR_N];
        float a;
    } gears;

#define RADAR_N 7
    struct
    {
        float a;
        struct
        {
            float a, d, life;
        } bl[RADAR_N];
    } radar;

    struct
    {
        float t;
    } laser;

#define PONG_TR 10
    struct
    {
        float bx, by, vx, vy, p1, p2;
        uint8_t n;
        struct
        {
            int8_t x, y;
        } tr[PONG_TR];
    } pong;

#define SKY_N 20
    struct
    {
        struct
        {
            float x, y, vx, vy, ph, w;
        } s[SKY_N];
    } sky;
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
 *  9  双螺旋：两条正弦链反向缠绕，碱基对跟着向左滚
 * ============================================================ */

static void hel_init(void)
{
    st.helix.t = 0.0f;
}

static void hel_step(void)
{
    st.helix.t += 0.055f;
}

static void hel_draw(void)
{
    const float cy = 9.0f, A = 6.0f, kw = 6.2831853f / 30.0f;
    const float shift = st.helix.t * 3.0f;
    float off;

    /* 两条主链：cos > 0 的那条算"在前面"，亮一档 */
    for (int x = 0; x < W; x++)
    {
        float ph = (float)x * kw - shift;
        float s = sinf(ph), c = cosf(ph);
        int y1 = (int)(cy + A * s + 0.5f);
        int y2 = (int)(cy - A * s + 0.5f);
        int g1 = 2 + (int)(5.0f * (c > 0.0f ? c : 0.0f));
        int g2 = 2 + (int)(5.0f * (-c > 0.0f ? -c : 0.0f));

        px(x, y1, White, g1);
        px(x, y2, White, g2);

        if (c > 0.6f)
            px(x, y1 - 1, White, 2);
        else if (c < -0.6f)
            px(x, y2 + 1, White, 2);
    }

    /* 碱基对：每 5 px 一根，跟着螺旋一起向左滚 */
    off = fmodf(shift / kw, 5.0f);

    for (float xx = -off; xx < (float)W; xx += 5.0f)
    {
        float ph = xx * kw - shift;
        float s = sinf(ph);
        int x = (int)(xx + 0.5f);
        int y1, y2, top, bot, g;

        if (fabsf(s) < 0.45f)
            continue;

        y1 = (int)(cy + A * s + 0.5f);
        y2 = (int)(cy - A * s + 0.5f);
        top = y1 < y2 ? y1 : y2;
        bot = y1 > y2 ? y1 : y2;
        g = fabsf(cosf(ph)) > 0.65f ? 3 : 2;

        for (int y = top + 1; y < bot; y += 2)
            px(x, y, Red, g);
    }
}

/* ============================================================
 *  10  旋转立方体：线框 3D，前后棱亮度不同
 * ============================================================ */

static void cube_init(void)
{
    st.cube.ax = 0.32f;
    st.cube.ay = 0.55f;
}

static void cube_step(void)
{
    st.cube.ax += 0.012f;
    st.cube.ay += 0.021f;
}

static void cube_draw(void)
{
    static const uint8_t E[12][2] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0}, {4, 5}, {5, 7},
        {7, 6}, {6, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    const float s = 3.8f, f = 20.0f, K = 1.6f, cx = 56.0f, cy = 9.0f;
    const float sa = sinf(st.cube.ax), ca = cosf(st.cube.ax);
    const float sb = sinf(st.cube.ay), cb = cosf(st.cube.ay);
    float vx[8], vy[8], vz[8];

    for (int i = 0; i < 8; i++)
    {
        float x = (i & 1) ? s : -s, y = (i & 2) ? s : -s, z = (i & 4) ? s : -s;
        float y1 = y * ca - z * sa, z1 = y * sa + z * ca;
        float x1 = x * cb + z1 * sb, z2 = -x * sb + z1 * cb;
        float p = f / (f + z2 + 8.0f);

        vx[i] = cx + x1 * p * K;
        vy[i] = cy + y1 * p * K;
        vz[i] = z2;
    }

    for (int i = 0; i < 12; i++)
    {
        int a = E[i][0], b = E[i][1];
        float z = (vz[a] + vz[b]) * 0.5f;
        int g = 1 + (int)(6.0f * (1.0f - (z + s) / (2.0f * s)));

        if (g < 1)
            g = 1;
        else if (g > 7)
            g = 7;

        draw_line((int)(vx[a] + 0.5f), (int)(vy[a] + 0.5f),
                  (int)(vx[b] + 0.5f), (int)(vy[b] + 0.5f),
                  z < 0.0f ? White : Cyan, g);
    }

    for (int i = 0; i < 8; i++)
        px((int)(vx[i] + 0.5f), (int)(vy[i] + 0.5f), White, 7);
}

/* ============================================================
 *  11  透视隧道：方框环由远及近，整体轻微摆动
 * ============================================================ */

static void tun_init(void)
{
    for (int i = 0; i < TUN_N; i++)
        st.tun.z[i] = 0.34f + (float)i * 0.5f;

    st.tun.t = 0.0f;
}

static void tun_step(void)
{
    st.tun.t += 0.05f;

    for (int i = 0; i < TUN_N; i++)
    {
        st.tun.z[i] -= 0.020f;

        if (st.tun.z[i] < 0.32f)
            st.tun.z[i] += (float)TUN_N * 0.5f;
    }
}

static void tun_draw(void)
{
    for (int i = 0; i < TUN_N; i++)
    {
        float z = st.tun.z[i], kk = 1.0f / z;
        float hh = 2.55f * kk, hw = hh * 6.2f;
        float oy;
        int x0, x1, y0, y1, g;
        Color c;

        if (hw > 92.0f)
            continue;

        g = (int)(8.0f - z * 1.35f);

        if (g < 1)
            g = 1;
        else if (g > 7)
            g = 7;

        c = (z < 0.62f) ? White : Cyan;
        oy = 9.0f + sinf(z * 0.75f + st.tun.t * 1.2f) * 2.2f;

        x0 = (int)(56.0f - hw + 0.5f);
        x1 = (int)(56.0f + hw + 0.5f);
        y0 = (int)(oy - hh + 0.5f);
        y1 = (int)(oy + hh + 0.5f);

        draw_line(x0, y0, x1, y0, c, g);
        draw_line(x0, y1, x1, y1, c, g);
        draw_line(x0, y0, x0, y1, c, g);
        draw_line(x1, y0, x1, y1, c, g);
    }

    px(56, 9, White, 3);
}

/* ============================================================
 *  12  齿轮组：一排互相咬合的齿轮，转速和半径成反比
 * ============================================================ */

static const float gear_r[GEAR_N] = {6.0f, 4.6f, 5.8f, 4.4f, 5.8f, 4.6f, 5.9f, 4.7f};
static const uint8_t gear_n[GEAR_N] = {9, 7, 9, 7, 9, 7, 9, 7};
static const float gear_sp[GEAR_N] = {1.00f, 1.30f, 1.03f, 1.36f, 1.03f, 1.30f, 1.02f, 1.28f};

static void ger_init(void)
{
    float x = 20.0f;

    for (int i = 0; i < GEAR_N; i++)
    {
        if (i)
            x += gear_r[i - 1] + gear_r[i];

        st.gears.g[i].x = x;
        st.gears.g[i].r = gear_r[i];
        st.gears.g[i].sp = gear_sp[i];
        st.gears.g[i].n = gear_n[i];
        st.gears.g[i].dir = (i & 1) ? -1 : 1;
    }

    st.gears.a = 0.0f;
}

static void ger_step(void)
{
    st.gears.a += 0.017f;
}

static void ger_one(int i)
{
    const float body = st.gears.g[i].r - 1.9f;
    const float ang = st.gears.a * st.gears.g[i].sp * (float)st.gears.g[i].dir;
    const float stepA = 6.2831853f / (float)st.gears.g[i].n;
    const float cx = st.gears.g[i].x, cy = 9.0f;
    int steps = (int)(body * 8.0f);

    if (steps < 16)
        steps = 16;

    /* 轮盘：一圈暗蓝，别抢齿的亮度 */
    for (int k = 0; k < steps; k++)
    {
        float t = (float)k / (float)steps * 6.2831853f;

        px((int)(cx + cosf(t) * body + 0.5f), (int)(cy + sinf(t) * body + 0.5f), Cyan, 2);
    }

    /* 齿：一颗齿两根挨着的短线，撑出约 2 px 宽 */
    for (int k = 0; k < st.gears.g[i].n; k++)
    {
        float t = ang + (float)k * stepA;

        for (int d = 0; d < 2; d++)
        {
            float a = t + (d ? 0.16f : 0.0f) * stepA;

            draw_line((int)(cx + cosf(a) * (body - 0.2f) + 0.5f),
                      (int)(cy + sinf(a) * (body - 0.2f) + 0.5f),
                      (int)(cx + cosf(a) * (st.gears.g[i].r + 0.7f) + 0.5f),
                      (int)(cy + sinf(a) * (st.gears.g[i].r + 0.7f) + 0.5f),
                      White, 5);
        }
    }

    px((int)(cx + 0.5f), (int)(cy + 0.5f), Red, 6);
}

static void ger_draw(void)
{
    for (int i = 0; i < GEAR_N; i++)
        ger_one(i);
}

/* ============================================================
 *  13  雷达：椭圆扫描屏，扫描线带余晖，目标点亮后衰减
 * ============================================================ */

static void rad_init(void)
{
    st.radar.a = 0.0f;

    for (int i = 0; i < RADAR_N; i++)
    {
        st.radar.bl[i].a = rndf(0.0f, 6.2832f);
        st.radar.bl[i].d = rndf(0.25f, 0.95f);
        st.radar.bl[i].life = 0.0f;
    }
}

static void rad_step(void)
{
    st.radar.a += 0.085f;

    if (st.radar.a > 6.2832f)
        st.radar.a -= 6.2832f;

    for (int i = 0; i < RADAR_N; i++)
    {
        float diff = st.radar.a - st.radar.bl[i].a;

        if (diff < 0.0f)
            diff += 6.2832f;

        if (diff < 0.10f)
            st.radar.bl[i].life = 1.0f;
        else
            st.radar.bl[i].life *= 0.988f;

        if (st.radar.bl[i].life < 0.03f && rndf(0.0f, 1.0f) < 0.006f)
        {
            st.radar.bl[i].a = rndf(0.0f, 6.2832f);
            st.radar.bl[i].d = rndf(0.25f, 0.95f);
        }
    }
}

static void rad_draw(void)
{
    const float cx = 56.0f, cy = 9.0f, RX = 27.0f, RY = 7.6f;

    /* 三圈刻度 */
    for (int r = 1; r <= 3; r++)
    {
        float fr = (float)r / 3.0f;
        int n = 44 + r * 16;

        for (int i = 0; i < n; i++)
        {
            float t = (float)i / (float)n * 6.2832f;

            px((int)(cx + cosf(t) * RX * fr + 0.5f),
               (int)(cy + sinf(t) * RY * fr + 0.5f), Cyan, 1);
        }
    }

    /* 十字 */
    draw_line((int)(cx - RX), (int)cy, (int)(cx + RX), (int)cy, Cyan, 1);
    draw_line((int)cx, (int)(cy - RY), (int)cx, (int)(cy + RY), Cyan, 1);

    /* 扫描线 + 余晖 */
    for (int k = 0; k < 16; k++)
    {
        float t = st.radar.a - (float)k * 0.055f;
        int g = 6 - (int)((float)k * 0.42f);

        if (g < 1)
            g = 1;

        draw_line((int)cx, (int)cy,
                  (int)(cx + cosf(t) * RX + 0.5f),
                  (int)(cy + sinf(t) * RY + 0.5f),
                  k == 0 ? White : Cyan, g);
    }

    /* 目标 */
    for (int i = 0; i < RADAR_N; i++)
    {
        float life = st.radar.bl[i].life;
        int x, y, g;

        if (life < 0.06f)
            continue;

        x = (int)(cx + cosf(st.radar.bl[i].a) * RX * st.radar.bl[i].d + 0.5f);
        y = (int)(cy + sinf(st.radar.bl[i].a) * RY * st.radar.bl[i].d + 0.5f);
        g = (int)(7.0f * life);

        px(x, y, White, g);
        px(x + 1, y, Red, g / 2);
    }
}

/* ============================================================
 *  14  利萨茹：示波器曲线，频率比缓慢漂移，一直在变形
 * ============================================================ */

static void las_init(void)
{
    st.laser.t = 0.0f;
}

static void las_step(void)
{
    st.laser.t += 0.011f;
}

static void las_draw(void)
{
    const float cx = 56.0f, cy = 9.0f, A = 53.0f, Bv = 7.8f;
    const float a = 3.0f + 1.15f * sinf(st.laser.t * 0.37f);
    const float b = 4.0f + 1.05f * cosf(st.laser.t * 0.29f);
    const float ph = st.laser.t * 2.3f;
    float lx = 0.0f, ly = 0.0f;
    float hu, hx, hy;

    for (int i = 0; i <= 280; i++)
    {
        float u = (float)i / 280.0f * 6.2832f;
        float x = cx + A * sinf(a * u + ph);
        float y = cy + Bv * sinf(b * u);

        if (i)
            draw_line((int)(lx + 0.5f), (int)(ly + 0.5f),
                      (int)(x + 0.5f), (int)(y + 0.5f), Cyan, 2);

        lx = x;
        ly = y;
    }

    /* 扫描头 */
    hu = fmodf(st.laser.t * 3.1f, 6.2832f);
    hx = cx + A * sinf(a * hu + ph);
    hy = cy + Bv * sinf(b * hu);

    px((int)(hx + 0.5f), (int)(hy + 0.5f), White, 7);
    px((int)(hx + 0.5f), (int)(hy + 0.5f) - 1, White, 3);
}

/* ============================================================
 *  15  乒乓：自己跟自己打的复古球局，带拖尾
 * ============================================================ */

static void pon_init(void)
{
    st.pong.bx = 56.0f;
    st.pong.by = 9.0f;
    st.pong.vx = 1.15f;
    st.pong.vy = 0.42f;
    st.pong.p1 = 7.0f;
    st.pong.p2 = 7.0f;
    st.pong.n = 0;
}

static void pon_step(void)
{
    float t1, d;

    st.pong.bx += st.pong.vx;
    st.pong.by += st.pong.vy;

    if (st.pong.by < 1.4f)
    {
        st.pong.by = 1.4f;
        st.pong.vy = fabsf(st.pong.vy);
    }
    else if (st.pong.by > 16.6f)
    {
        st.pong.by = 16.6f;
        st.pong.vy = -fabsf(st.pong.vy);
    }

    /* 两只拍子都追着球，右边的稍快一点 */
    t1 = st.pong.by - 2.0f;

    d = (t1 - st.pong.p1) * 0.30f;
    st.pong.p1 += (d > 1.0f) ? 1.0f : (d < -1.0f ? -1.0f : d);

    d = (t1 - st.pong.p2) * 0.34f;
    st.pong.p2 += (d > 1.0f) ? 1.0f : (d < -1.0f ? -1.0f : d);

    if (st.pong.bx < 5.0f)
    {
        if (fabsf(st.pong.by - (st.pong.p1 + 2.0f)) < 2.6f)
        {
            st.pong.bx = 5.0f;
            st.pong.vx = fabsf(st.pong.vx) * 1.015f;
            st.pong.vy += (st.pong.by - (st.pong.p1 + 2.0f)) * 0.10f;
        }
        else
        {
            st.pong.bx = 56.0f;
            st.pong.by = rndf(4.0f, 14.0f);
            st.pong.vx = 1.15f;
            st.pong.vy = rndf(-0.5f, 0.5f);
            st.pong.n = 0;
        }
    }
    else if (st.pong.bx > 107.0f)
    {
        if (fabsf(st.pong.by - (st.pong.p2 + 2.0f)) < 2.6f)
        {
            st.pong.bx = 107.0f;
            st.pong.vx = -fabsf(st.pong.vx) * 1.015f;
            st.pong.vy += (st.pong.by - (st.pong.p2 + 2.0f)) * 0.10f;
        }
        else
        {
            st.pong.bx = 56.0f;
            st.pong.by = rndf(4.0f, 14.0f);
            st.pong.vx = -1.15f;
            st.pong.vy = rndf(-0.5f, 0.5f);
            st.pong.n = 0;
        }
    }

    if (st.pong.vy > 1.15f)
        st.pong.vy = 1.15f;
    else if (st.pong.vy < -1.15f)
        st.pong.vy = -1.15f;

    /* 拖尾 */
    if (st.pong.n < PONG_TR)
        st.pong.n++;
    else
    {
        for (int i = 1; i < PONG_TR; i++)
            st.pong.tr[i - 1] = st.pong.tr[i];
    }

    st.pong.tr[st.pong.n - 1].x = (int8_t)(st.pong.bx + 0.5f);
    st.pong.tr[st.pong.n - 1].y = (int8_t)(st.pong.by + 0.5f);
}

static void pon_draw(void)
{
    /* 中线 */
    for (int y = 0; y < H; y += 3)
        px(56, y, Cyan, 1);

    /* 球拍 */
    for (int i = 0; i < 5; i++)
    {
        px(3, (int)(st.pong.p1 + 0.5f) + i, White, 6);
        px(108, (int)(st.pong.p2 + 0.5f) + i, White, 6);
    }

    /* 球的拖尾 */
    for (int i = 0; i < st.pong.n; i++)
    {
        int g = (int)(5.0f * (float)(i + 1) / (float)st.pong.n);

        px(st.pong.tr[i].x, st.pong.tr[i].y,
           (i == st.pong.n - 1) ? White : Red, g);
    }
}

/* ============================================================
 *  16  星座：星星缓慢漂移，近的连线，各自闪烁
 * ============================================================ */

static void sky_init(void)
{
    for (int i = 0; i < SKY_N; i++)
    {
        st.sky.s[i].x = rndf(2.0f, (float)W - 2.0f);
        st.sky.s[i].y = rndf(1.0f, (float)H - 2.0f);
        st.sky.s[i].vx = rndf(-0.10f, 0.10f);
        st.sky.s[i].vy = rndf(-0.05f, 0.05f);
        st.sky.s[i].ph = rndf(0.0f, 6.2832f);
        st.sky.s[i].w = rndf(0.02f, 0.05f);
    }
}

static void sky_step(void)
{
    for (int i = 0; i < SKY_N; i++)
    {
        st.sky.s[i].x += st.sky.s[i].vx;
        st.sky.s[i].y += st.sky.s[i].vy;
        st.sky.s[i].ph += st.sky.s[i].w;

        if (st.sky.s[i].x < 1.0f)
            st.sky.s[i].x += (float)W - 2.0f;
        else if (st.sky.s[i].x > (float)W - 1.0f)
            st.sky.s[i].x -= (float)W - 2.0f;

        if (st.sky.s[i].y < 1.0f)
            st.sky.s[i].y += (float)H - 2.0f;
        else if (st.sky.s[i].y > (float)H - 1.0f)
            st.sky.s[i].y -= (float)H - 2.0f;
    }
}

static void sky_draw(void)
{
    /* 连线：越近越亮 */
    for (int i = 0; i < SKY_N; i++)
    {
        for (int j = i + 1; j < SKY_N; j++)
        {
            float dx = st.sky.s[i].x - st.sky.s[j].x;
            float dy = (st.sky.s[i].y - st.sky.s[j].y) * 2.2f;
            float d = sqrtf(dx * dx + dy * dy);
            int g;

            if (d > 24.0f)
                continue;

            g = (int)(3.0f * (1.0f - d / 24.0f)) + 1;

            draw_line((int)(st.sky.s[i].x + 0.5f), (int)(st.sky.s[i].y + 0.5f),
                      (int)(st.sky.s[j].x + 0.5f), (int)(st.sky.s[j].y + 0.5f),
                      Cyan, g);
        }
    }

    /* 星点 */
    for (int i = 0; i < SKY_N; i++)
    {
        int g = 2 + (int)(5.0f * fabsf(sinf(st.sky.s[i].ph)));
        int x = (int)(st.sky.s[i].x + 0.5f);
        int y = (int)(st.sky.s[i].y + 0.5f);

        px(x, y, White, g);

        if (g >= 6)
        {
            px(x - 1, y, White, 2);
            px(x + 1, y, White, 2);
            px(x, y - 1, White, 2);
        }
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
    {hel_init, hel_step, hel_draw},   /* 8 Helix */
    {cube_init, cube_step, cube_draw},/* 9 Cube */
    {tun_init, tun_step, tun_draw},   /* 10 Tunnel */
    {ger_init, ger_step, ger_draw},   /* 11 Gears */
    {rad_init, rad_step, rad_draw},   /* 12 Radar */
    {las_init, las_step, las_draw},   /* 13 Lissajous */
    {pon_init, pon_step, pon_draw},   /* 14 Pong */
    {sky_init, sky_step, sky_draw},   /* 15 Constellation */
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
