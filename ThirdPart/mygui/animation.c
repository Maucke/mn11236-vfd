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
#define AU_STAR_N 24
#define HB_STAR_N 16
#define HB_BOX_N 5
#define RW_STAR_N 26
#define BR_CAR_N 4

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
        int8_t mt[W];
        struct
        {
            uint8_t x, y, ph;
        } st[AU_STAR_N];
        float t;
    } aurora;

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
        struct
        {
            uint8_t x, y, ph;
        } st[HB_STAR_N];
        struct
        {
            uint8_t x, h, c;
        } box[HB_BOX_N];
        float t;
    } harbor;

#define TUN_N 10
    struct
    {
        float z[TUN_N];
        float t;
    } tun;

    struct
    {
        int8_t hw[H];
        struct
        {
            uint8_t x, y, ph;
        } st[RW_STAR_N];
        float t;
    } runway;

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

    struct
    {
        int8_t cable[W];
        struct
        {
            float x, sp;
            uint8_t c;
        } car[BR_CAR_N];
        float t;
    } bridge;

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
 *  4  极光：两层帘幕在山脊上方摆动
 *
 *  设备只有红/蓝两个通道，没有绿色，所以极光做成"蓝白帘幕 + 红色顶边"。
 *  横向按 sin 分成明暗相间的带子，竖向按 sin 分成射线，向下线性变暗。
 * ============================================================ */

static void aur_init(void)
{
    /* 山脊：四个三角峰取最大值，比正弦叠加更像山 */
    static const int8_t pk[4][3] = {{14, 7, 15}, {40, 5, 13}, {66, 9, 19}, {96, 6, 15}};

    for (int x = 0; x < W; x++)
    {
        int h = 1;

        for (int i = 0; i < 4; i++)
        {
            int d = x - pk[i][0];

            if (d < 0)
                d = -d;

            if (d < pk[i][2])
            {
                int v = (pk[i][1] * (pk[i][2] - d) + pk[i][2] / 2) / pk[i][2];

                if (v > h)
                    h = v;
            }
        }

        st.aurora.mt[x] = (int8_t)h;
    }

    for (int i = 0; i < AU_STAR_N; i++)
    {
        st.aurora.st[i].x = (uint8_t)(rand() % W);
        st.aurora.st[i].y = (uint8_t)(rand() % 8);
        st.aurora.st[i].ph = (uint8_t)(rand() % 64);
    }

    st.aurora.t = 0.0f;
}

static void aur_step(void)
{
    st.aurora.t += 0.0333f;
}

/* 一条极光带：y0 顶边基准，amp 波动幅度，len 高度，g0 亮度，c 颜色，ph 带子相位 */
static void aur_band(float t, float y0, float amp, float len, float g0, Color c, float ph)
{
    for (int x = 0; x < W; x++)
    {
        float patch = sinf((float)x * 0.045f + t * 0.35f + ph) * 0.5f + 0.5f;
        float k, ytop, ray, l, xf = (float)x;

        if (patch < 0.34f)
            continue;

        k = (patch - 0.34f) / 0.66f;
        ytop = y0 + amp * sinf(xf * 0.075f + t * 0.55f)
                  + amp * 0.4f * sinf(xf * 0.19f - t * 0.31f);
        ray = 0.45f + 0.55f * (0.5f + 0.5f * sinf(xf * 0.7f + t * 2.2f));
        l = len * (0.5f + 0.5f * k);

        for (int i = 0; i < (int)l; i++)
        {
            int g = (int)((1.0f - (float)i / l) * g0 * k * ray);

            if (g < 1)
                continue;

            if (i == 0)
                px(x, (int)(ytop + 0.5f), Red, (g * 85) / 100);
            else
                px(x, (int)(ytop + 0.5f) + i, c, g);
        }
    }
}

static void aur_draw(void)
{
    float t = st.aurora.t;

    /* 星星 */
    for (int i = 0; i < AU_STAR_N; i++)
        px(st.aurora.st[i].x, st.aurora.st[i].y, White,
           1 + ((sinf(t * 1.9f + (float)st.aurora.st[i].ph) > 0.5f) ? 1 : 0));

    /* 远（暗蓝）→ 近（亮白） */
    aur_band(t * 0.85f, 4.5f, 1.2f, 4.0f, 5.5f, Cyan, 0.0f);
    aur_band(t * 1.15f, 3.2f, 1.6f, 5.0f, 10.0f, White, 2.1f);

    /* 山脊（挡住帘幕底部） */
    for (int x = 0; x < W; x++)
    {
        int top = H - 1 - st.aurora.mt[x];

        for (int y = top; y < H; y++)
            px(x, y, White, 2);

        px(x, top, White, (st.aurora.mt[x] >= 5) ? 4 : 3);
    }
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
 *  10  港口吊机：两台龙门吊，小车沿吊臂往返、吊钩上下取箱
 * ============================================================ */

static void hb_crane(float t, int x0, int k)
{
    const int deck = 12, top = 3, half = 13;
    float ph = k ? 2.6f : 0.0f;
    float p;
    int tx, hl, by;

    /* 两条腿 */
    for (int s = -1; s <= 1; s += 2)
        for (int y = top + 2; y <= deck; y++)
            px(x0 + 5 * s, y, White, 2);

    /* X 形斜撑 */
    for (int y = top + 3; y < deck - 1; y += 3)
    {
        draw_line(x0 - 5, y, x0 + 5, y + 2, White, 1);
        draw_line(x0 + 5, y, x0 - 5, y + 2, White, 1);
    }

    /* 吊臂 */
    for (int x = x0 - half; x <= x0 + half; x++)
        px(x, top + 2, White, 3);

    /* 塔尖 + 闪红灯（两台错开） */
    px(x0, top, White, 3);
    px(x0, top + 1, White, 3);

    if ((((int)(t * 1.5f) + k) & 1) == 0)
        px(x0, top - 1, Red, 7);

    /* 小车沿吊臂往返 */
    p = 0.5f + 0.5f * sinf(t * 0.55f + ph);
    tx = x0 - half + 2 + (int)((float)(2 * half - 4) * p + 0.5f);
    px(tx, top + 3, White, 5);

    /* 吊钩上下 */
    hl = 1 + (int)(3.0f * (0.5f + 0.5f * sinf(t * 1.1f + ph)) + 0.5f);

    for (int y = top + 4; y <= top + 3 + hl; y++)
        px(tx, y, White, 1);

    /* 集装箱 */
    by = top + 4 + hl;

    if (by < deck)
    {
        Color c = k ? Cyan : Red;

        px(tx - 1, by, c, 5);
        px(tx, by, c, 5);
        px(tx + 1, by, c, 5);
    }
}

static void hb_init(void)
{
    for (int i = 0; i < HB_STAR_N; i++)
    {
        st.harbor.st[i].x = (uint8_t)(rand() % W);
        st.harbor.st[i].y = (uint8_t)(rand() % 5);
        st.harbor.st[i].ph = (uint8_t)(rand() % 64);
    }

    for (int i = 0; i < HB_BOX_N; i++)
    {
        st.harbor.box[i].x = (uint8_t)(44 + i * 4);
        st.harbor.box[i].h = (uint8_t)(1 + (city_hash(i, 0, 2) & 1));
        st.harbor.box[i].c = (city_hash(i, 1, 2) & 1) ? Cyan : Red;
    }

    st.harbor.t = 0.0f;
}

static void hb_step(void)
{
    st.harbor.t += 0.0333f;
}

static void hb_draw(void)
{
    const int deck = 12;
    float t = st.harbor.t;

    /* 星星 */
    for (int i = 0; i < HB_STAR_N; i++)
        px(st.harbor.st[i].x, st.harbor.st[i].y, White,
           1 + (sinf(t * 2.1f + (float)st.harbor.st[i].ph) > 0.6f ? 1 : 0));

    /* 水面 */
    for (int y = deck + 2; y < H; y++)
    {
        if (y == H - 1)
        {
            for (int x = 0; x < W; x++)
                px(x, y, Cyan, 2);

            continue;
        }

        for (int x = 0; x < W; x++)
            if (((x + (int)(t * 4.0f)) % 11) < 6)
                px(x, y, Cyan, 1);
    }

    /* 码头面 */
    for (int x = 0; x < W; x++)
    {
        px(x, deck, White, 4);
        px(x, deck + 1, White, 1);
    }

    /* 堆场集装箱 */
    for (int i = 0; i < HB_BOX_N; i++)
        for (int k = 0; k < st.harbor.box[i].h; k++)
            for (int x = st.harbor.box[i].x; x < st.harbor.box[i].x + 3; x++)
                px(x, deck - 1 - k, (Color)st.harbor.box[i].c, 3);

    /* 两台龙门吊 */
    hb_crane(t, 24, 0);
    hb_crane(t, 74, 1);
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
 *  12  机场跑道灯：透视跑道，追光从远端一路压过来
 * ============================================================ */

static void rw_init(void)
{
    const int vy = 5;
    const float span = (float)(H - 1 - vy);

    for (int y = 0; y < H; y++)
    {
        if (y <= vy)
            st.runway.hw[y] = 0;
        else
            st.runway.hw[y] = (int8_t)(2.0f + 48.0f * powf((float)(y - vy) / span, 1.55f));
    }

    for (int i = 0; i < RW_STAR_N; i++)
    {
        st.runway.st[i].x = (uint8_t)(rand() % W);
        st.runway.st[i].y = (uint8_t)(rand() % 5);
        st.runway.st[i].ph = (uint8_t)(rand() % 64);
    }

    st.runway.t = 0.0f;
}

static void rw_step(void)
{
    st.runway.t += 0.0333f;
}

static void rw_draw(void)
{
    const int vx = 56, vy = 5;
    const float span = (float)(H - 1 - vy);
    float t = st.runway.t;
    float ph = t * 0.55f;
    int iy;

    ph -= (float)(int)ph; /* 0..1 */

    /* 星星 + 远处城市微光 */
    for (int i = 0; i < RW_STAR_N; i++)
        px(st.runway.st[i].x, st.runway.st[i].y, White,
           1 + (sinf(t * 1.7f + (float)st.runway.st[i].ph) > 0.5f ? 1 : 0));

    for (int x = 0; x < W; x++)
        if ((x % 7) < 3)
            px(x, vy, Cyan, 1);

    /* 远端跑道头一对红灯 */
    px(vx - 3, vy + 1, Red, 4);
    px(vx + 3, vy + 1, Red, 4);

    /* 道面（压得很暗）+ 两条边灯：越近越亮，追光扫过时 +3 */
    for (int y = vy + 1; y < H; y++)
    {
        float u = (float)(y - vy) / span;
        int hw = st.runway.hw[y];
        int g0 = 2 + (int)(4.0f * u);

        if (fabsf(u - ph) < 0.09f)
            g0 += 3;

        for (int x = vx - hw + 1; x <= vx + hw - 1; x++)
            px(x, y, White, 1);

        px(vx - hw, y, White, g0);
        px(vx + hw, y, White, g0);
    }

    /* 中线虚线：往观察者方向流，越近越宽 */
    for (int y = vy + 2; y < H; y++)
    {
        float u = (float)(y - vy) / span;
        int w;

        if (((y + (int)(t * 10.0f)) % 3) != 0)
            continue;

        w = (int)(u * 2.4f + 0.5f);

        if (w < 1)
            w = 1;

        for (int x = vx - w; x <= vx + w; x++)
            px(x, y, Cyan, 3 + (int)(u * 4.0f));
    }

    /* 近端门槛灯：红白相间 */
    for (iy = 6; iy < W - 6; iy += 4)
        px(iy, H - 1, ((iy >> 2) & 1) ? Red : White, 5);
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
 *  15  跨海大桥：主缆下垂，桥面车灯对向行驶，水面反光
 * ============================================================ */

static void br_init(void)
{
    const int deck = 12, tx1 = 30, tx2 = 82, top = 3;

    /* 主缆曲线一次算好，画的时候直接查表 */
    for (int x = 0; x < W; x++)
    {
        float y;

        if (x >= tx1 && x <= tx2)
        {
            float u = (float)(x - tx1) / (float)(tx2 - tx1);

            y = (float)(top + 1) + (float)(deck - top - 2) * 4.0f * u * (1.0f - u);
        }
        else if (x < tx1)
        {
            float u = 1.0f - (float)x / (float)tx1;

            y = (float)(top + 1) + (float)(deck - top - 1) * powf(u, 1.35f);
        }
        else
        {
            float u = (float)(x - tx2) / (float)(W - 1 - tx2);

            y = (float)(top + 1) + (float)(deck - top - 1) * powf(u, 1.35f);
        }

        st.bridge.cable[x] = (int8_t)(y + 0.5f);
    }

    st.bridge.car[0].x = 10.0f;
    st.bridge.car[0].sp = 0.9f;
    st.bridge.car[0].c = Red;
    st.bridge.car[1].x = 48.0f;
    st.bridge.car[1].sp = 0.9f;
    st.bridge.car[1].c = White;
    st.bridge.car[2].x = 70.0f;
    st.bridge.car[2].sp = -0.7f;
    st.bridge.car[2].c = White;
    st.bridge.car[3].x = 95.0f;
    st.bridge.car[3].sp = -0.7f;
    st.bridge.car[3].c = Red;

    st.bridge.t = 0.0f;
}

static void br_step(void)
{
    st.bridge.t += 0.0333f;

    for (int i = 0; i < BR_CAR_N; i++)
    {
        st.bridge.car[i].x += st.bridge.car[i].sp;

        if (st.bridge.car[i].x > (float)(W + 2))
            st.bridge.car[i].x = -2.0f;
        else if (st.bridge.car[i].x < -2.0f)
            st.bridge.car[i].x = (float)(W + 2);
    }
}

static void br_draw(void)
{
    const int deck = 12, tx1 = 30, tx2 = 82, top = 3;
    float t = st.bridge.t;

    /* 水面 */
    for (int y = deck + 2; y < H; y++)
    {
        if (y == H - 1)
        {
            for (int x = 0; x < W; x++)
                px(x, y, Cyan, 2);

            continue;
        }

        for (int x = 0; x < W; x++)
            if (((x + (int)(t * 5.0f)) % 9) < 5)
                px(x, y, Cyan, 1);
    }

    /* 桥面 */
    for (int x = 0; x < W; x++)
    {
        px(x, deck, White, 4);
        px(x, deck + 1, White, 2);
    }

    /* 两座桥塔 + 塔顶红灯 */
    for (int i = 0; i < 2; i++)
    {
        int tx = i ? tx2 : tx1;

        for (int y = top; y <= deck; y++)
        {
            px(tx, y, White, 5);
            px(tx + 1, y, White, 4);
        }

        if (((int)(t * 2.0f) & 1) == 0)
            px(tx, top - 1, Red, 7);

        px(tx, top + 3, White, 3);
        px(tx + 1, top + 3, White, 3);
    }

    /* 主缆 + 吊索 */
    for (int x = 0; x < W; x++)
    {
        int y = st.bridge.cable[x];

        px(x, y, White, 2);

        if (((x - tx1) % 6) == 0 && x > tx1 && x < tx2)
            for (int yy = y + 1; yy < deck; yy++)
                px(x, yy, White, 1);
    }

    /* 车灯 + 水里倒影 */
    for (int i = 0; i < BR_CAR_N; i++)
    {
        int cx = (int)(st.bridge.car[i].x + 0.5f);
        Color c = (Color)st.bridge.car[i].c;

        px(cx, deck - 1, c, 7);
        px(cx - ((st.bridge.car[i].sp > 0.0f) ? 1 : -1), deck - 1, c, 3);
        px(cx, deck + 3, c, 2);
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
    {aur_init, aur_step, aur_draw},   /* 3 Aurora */
    {city_init, city_step, city_draw},/* 4 Skyline */
    {warp_init, warp_step, warp_draw},/* 5 Warp */
    {bub_init, bub_step, bub_draw},   /* 6 Bubble */
    {fw_init, fw_step, fw_draw},      /* 7 Firework */
    {hel_init, hel_step, hel_draw},   /* 8 Helix */
    {hb_init, hb_step, hb_draw},      /* 9 Harbor */
    {tun_init, tun_step, tun_draw},   /* 10 Tunnel */
    {rw_init, rw_step, rw_draw},      /* 11 Runway */
    {rad_init, rad_step, rad_draw},   /* 12 Radar */
    {las_init, las_step, las_draw},   /* 13 Lissajous */
    {br_init, br_step, br_draw},      /* 14 Bridge */
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
