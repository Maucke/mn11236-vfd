(function () {
  "use strict";

  const W = 112;
  const H = 18;
  const S = 5;
  const CELL = 4;
  const CW = W * S;
  const CH = H * S;

  const WHITE = [242, 239, 233];
  const CYAN = [82, 226, 244];
  const RED = [255, 88, 86];

  const PALETTES = [
    { bar: CYAN, dot: RED },
    { bar: RED, dot: CYAN },
    { bar: WHITE, dot: CYAN },
    { bar: RED, dot: WHITE },
    { bar: CYAN, dot: WHITE }
  ];

  const shadeCache = new Map();

  function shade(rgb, lv) {
    const level = Math.max(0, Math.min(7, lv | 0));
    const key = rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + level;
    let out = shadeCache.get(key);
    if (out) return out;
    const k = Math.pow(level / 7, 0.78);
    out = "rgb(" + Math.round(rgb[0] * k) + "," + Math.round(rgb[1] * k) + "," + Math.round(rgb[2] * k) + ")";
    shadeCache.set(key, out);
    return out;
  }

  function P(ctx, x, y, rgb, lv) {
    x = x | 0;
    y = y | 0;
    if (x < 0 || y < 0 || x >= W || y >= H || lv <= 0) return;
    ctx.fillStyle = shade(rgb, lv);
    ctx.fillRect(x * S, y * S, CELL, CELL);
  }

  function clear(ctx) {
    ctx.fillStyle = "#080b0b";
    ctx.fillRect(0, 0, CW, CH);
  }

  function gauss(f, center, width) {
    const d = (f - center) / width;
    return Math.exp(-d * d);
  }

  function clamp01(v) {
    return v < 0 ? 0 : v > 1 ? 1 : v;
  }

  function noise(n) {
    const x = Math.sin(n * 127.1 + 311.7) * 43758.5453123;
    return x - Math.floor(x);
  }

  function smoothInto(dst, src, up, down) {
    for (let i = 0; i < src.length; i++) {
      const target = src[i];
      if (target > dst[i]) dst[i] = Math.min(target, dst[i] + up);
      else dst[i] = Math.max(target, dst[i] - down);
    }
  }

  function peakStep(pk, value, fall) {
    if (value >= pk) return value;
    return Math.max(value, pk - fall);
  }

  // Smooth synthetic music. No per-frame stepped random values: the preview
  // should move like music, not flicker like noise.
  function energy(f, t) {
    const beat = (t % 0.5) / 0.5;
    const kick = Math.exp(-beat * 5.2);
    const section = 0.80 + 0.20 * Math.sin(t * 0.19 + 0.7);
    const sway1 = 0.5 + 0.5 * Math.sin(t * 2.1 + f * 0.011);
    const sway2 = 0.5 + 0.5 * Math.sin(t * 4.4 + f * 0.023 + 1.3);
    let v = 0;

    v += 0.96 * gauss(f, 68, 38) * (0.52 + 0.48 * kick) * section;
    const bassF = 104 + 58 * Math.sin(t * 0.61);
    v += 0.78 * gauss(f, bassF, 23) * (0.48 + 0.52 * kick);

    const melF = 620 * Math.pow(2, Math.sin(t * 0.39) * 1.12);
    v += 0.58 * gauss(f, melF, 86) * (0.32 + 0.68 * Math.max(0, Math.sin(t * 3.05 + 1.1)));
    v += 0.34 * gauss(f, melF * 2.03, 150) * (0.28 + 0.72 * Math.max(0, Math.sin(t * 2.37 + 0.5)));

    v += 0.70 * gauss(f, 3600, 940) * kick;
    v += 0.20 * gauss(f, 2350, 680) * (0.35 + 0.65 * sway1);
    v += 0.28 * gauss(f, 5200, 520) * (0.25 + 0.75 * sway2);

    v += 0.035 + 0.035 * sway2;
    return Math.max(0, Math.min(1, v));
  }

  const vals = new Float32Array(W);

  function barFreq(i, mode) {
    if (mode === "linear") return 20 + (4980 * i) / (W - 1);
    return 40 * Math.pow(5000 / 40, i / (W - 1));
  }

  function sampleSpectrum(t, mode) {
    for (let i = 0; i < W; i++) {
      const f = barFreq(i, mode);
      const shimmer = 0.985 + 0.015 * Math.sin(t * 1.7 + i * 0.21);
      vals[i] = energy(f, t) * shimmer;
    }
  }

  let palette = PALETTES[0];

  function bandMax(from, to) {
    let v = 0;
    for (let i = from; i <= to; i++) v = Math.max(v, vals[Math.max(0, Math.min(W - 1, i))]);
    return v;
  }

  function bandAvg(from, to) {
    let sum = 0;
    let n = 0;
    for (let i = from; i <= to; i++) {
      sum += vals[Math.max(0, Math.min(W - 1, i))];
      n++;
    }
    return n ? sum / n : 0;
  }

  const STYLES = [];

  // 01 — Fire: procedural flame tongues, red body and white tips.
  STYLES.push({
    id: "fire",
    name: "火焰",
    tag: "全新",
    meta: "RAM 0 B · Flash +350 B",
    desc: "底部火焰被频谱点燃，低频让火墙升高；火舌左右摆动，边缘留出孔洞，顶部是白色火尖。",
    init: function () {},
    draw: function (ctx, t) {
      for (let x = 0; x < W; x++) {
        const v = vals[x];
        const base = 2 + v * 9;
        const sway = Math.sin(t * 3.4 + x * 0.20) * 1.3 + Math.sin(t * 6.1 + x * 0.47 + 1.2) * 0.7;
        const h = Math.max(1, Math.min(H - 1, Math.round(base + sway * (0.35 + v * 0.8))));
        const top = H - 1 - h;

        for (let y = H - 1; y >= top; y--) {
          const d = H - 1 - y;
          const edge = d / Math.max(1, h);
          const flick = Math.sin(x * 0.63 + y * 1.27 + t * 7.2) + Math.sin(x * 0.21 - y * 0.71 - t * 4.4);
          if (edge > 0.28 && flick < -1.15) continue;
          const lv = Math.max(1, Math.min(7, 7 - Math.round(edge * 4)));
          P(ctx, x, y, edge > 0.68 ? WHITE : RED, lv);
        }

        if (h > 2) {
          P(ctx, x, top, WHITE, 6);
          P(ctx, x, top + 1, WHITE, 3);
        }
      }

      for (let x = 0; x < W; x++) {
        P(ctx, x, H - 1, RED, 4);
        if ((x + Math.floor(t * 8)) % 7 === 0) P(ctx, x, H - 2, WHITE, 3);
      }
    }
  });

  // 02 — Matrix rain: vertical trails driven by spectral columns.
  STYLES.push({
    id: "matrix",
    name: "矩阵雨",
    tag: "全新",
    meta: "RAM ≈224 B · Flash +400 B",
    desc: "28 列点阵雨滴向下坠落；频段越强，雨滴速度、拖尾长度和头部亮度越高。",
    init: function () {
      this.cols = 28;
      this.drops = [];
      for (let c = 0; c < this.cols; c++) {
        this.drops.push({
          y: (c * 5) % H,
          len: 4 + (c % 5),
          seed: c * 1.37
        });
      }
    },
    draw: function (ctx, t) {
      const cols = this.cols;
      const drops = this.drops;

      for (let c = 0; c < cols; c++) {
        const v = Math.max(vals[c * 4], vals[c * 4 + 1], vals[c * 4 + 2], vals[c * 4 + 3]);
        const d = drops[c];
        d.y += 0.09 + v * 0.30;
        if (d.y - d.len > H + 2) {
          d.y = -2 - (c % 4);
          d.len = 3 + Math.round(v * 7);
        }
        const len = Math.max(2, d.len);
        for (let k = 0; k < len; k++) {
          const yy = Math.round(d.y - k);
          if (yy < 0 || yy >= H) continue;
          const lv = k === 0 ? 7 : Math.max(1, 6 - Math.round(k * 0.62));
          const rgb = k === 0 && v > 0.72 ? palette.dot : palette.bar;
          for (let dx = 0; dx < 3; dx++) P(ctx, c * 4 + dx, yy, rgb, lv);
        }
        if (d.y >= 0 && d.y < H) {
          P(ctx, c * 4, Math.round(d.y), WHITE, 7);
          P(ctx, c * 4 + 1, Math.round(d.y), WHITE, 7);
        }
      }

      for (let c = 0; c < cols; c++) {
        for (let y = (c * 3) % 4; y < H; y += 4) {
          if ((c * 7 + y * 5) % 13 === 0) P(ctx, c * 4 + 1, y, palette.bar, 1);
        }
      }
      for (let x = 0; x < W; x++) P(ctx, x, H - 1, palette.bar, 1);
    }
  });

  // 03 — Scope: a moving waveform over a fixed grid.
  STYLES.push({
    id: "scope",
    name: "示波器",
    tag: "全新",
    meta: "RAM 0 B · Flash +250 B",
    desc: "带网格的移动波形；频谱控制振幅，波形横向滑动，像一台老式示波器。",
    init: function () {},
    draw: function (ctx, t) {
      for (let x = 0; x < W; x += 14) {
        for (let y = 0; y < H; y++) P(ctx, x, y, palette.bar, 1);
      }
      for (let y = 2; y < H; y += 4) {
        for (let x = 0; x < W; x++) P(ctx, x, y, palette.bar, 1);
      }

      const phase = t * 5.0;
      let prev = H >> 1;
      for (let x = 0; x < W; x++) {
        const v = vals[x];
        const amp = 1.2 + v * 4.4;
        let y = 9 - Math.sin(phase + x * 0.26) * amp - Math.sin(phase * 1.7 + x * 0.09) * 1.2;
        y = Math.max(1, Math.min(H - 2, Math.round(y)));
        const lo = Math.min(prev, y);
        const hi = Math.max(prev, y);
        for (let yy = lo; yy <= hi; yy++) {
          P(ctx, x, yy, v > 0.66 ? WHITE : palette.bar, yy === y ? 7 : 5);
        }
        if (x % 7 === 0) P(ctx, x, y - 1, palette.dot, 3);
        prev = y;
      }

      const cursor = Math.floor((t * 28) % W);
      for (let y = 1; y < H - 1; y++) P(ctx, cursor, y, palette.dot, 2);
      P(ctx, cursor, 0, WHITE, 5);
      P(ctx, cursor, H - 1, WHITE, 5);
    }
  });

  // 04 — Perspective bars: 3D prisms on a floor grid.
  STYLES.push({
    id: "p-bars",
    name: "透视柱阵",
    tag: "全新",
    meta: "RAM 0 B · Flash +300 B",
    desc: "16 根带顶面和侧面的立体柱，站在透视网格上；高度仍由频段驱动，但构图是 3D 的。",
    init: function () {
      this.v = new Float32Array(16);
      this.pk = new Float32Array(16);
      this.target = new Float32Array(16);
    },
    draw: function (ctx, t) {
      for (let c = 0; c < 16; c++) {
        this.target[c] = bandMax(c * 7, c * 7 + 6);
      }
      smoothInto(this.v, this.target, 0.55, 0.12);

      const vpX = W / 2;
      const vpY = 6;
      for (let x = 0; x <= W; x += 14) {
        for (let k = 1; k <= 10; k++) {
          const u = k / 10;
          P(ctx, Math.round(vpX + (x - vpX) * u), Math.round(vpY + (H - 1 - vpY) * u), palette.bar, 1);
        }
      }
      for (let y = 10; y < H; y += 2) {
        for (let x = 0; x < W; x++) P(ctx, x, y, palette.bar, 1);
      }
      for (let x = 0; x < W; x++) P(ctx, x, vpY, palette.bar, 2);

      for (let c = 0; c < 16; c++) {
        const x = 6 + c * 6;
        const h = Math.min(10, Math.round(this.v[c] * 10));
        if (h > 0) {
          for (let yy = 0; yy < h; yy++) {
            const y = H - 1 - yy;
            for (let xx = 0; xx < 5; xx++) P(ctx, x + xx, y, palette.bar, Math.min(6, 2 + Math.round((yy / h) * 4)));
          }
          P(ctx, x + 4, H - 1 - h, palette.bar, 3);
          for (let xx = 1; xx <= 4; xx++) P(ctx, x + xx, H - 2 - h, WHITE, 6);
          for (let xx = 2; xx <= 4; xx++) P(ctx, x + xx, H - 3 - h, WHITE, 3);
        }
        this.pk[c] = peakStep(this.pk[c], h, 0.28);
        if (this.pk[c] > h + 1.2) {
          P(ctx, x + 2, H - 2 - Math.round(this.pk[c]), palette.dot, 6);
        }
      }
    }
  });

  // 05 — Spiral galaxy: two rotating elliptical arms.
  STYLES.push({
    id: "spiral",
    name: "螺旋星系",
    tag: "全新",
    meta: "RAM 0 B · Flash +350 B",
    desc: "两条椭圆旋臂围绕中心旋转，频率从内到外映射成青 / 红 / 白；背景有稀疏星点。",
    init: function () {},
    draw: function (ctx, t) {
      for (let i = 0; i < 30; i++) {
        const x = (i * 43 + 7) % W;
        const y = (i * 17 + 3) % H;
        const twinkle = 0.5 + 0.5 * Math.sin(t * 1.6 + i * 1.7);
        P(ctx, x, y, i % 5 === 0 ? palette.dot : palette.bar, 1 + Math.round(twinkle * 2));
      }

      const cx = W / 2;
      const cy = H / 2;
      const N = 80;
      for (let arm = 0; arm < 2; arm++) {
        for (let i = 0; i < N; i++) {
          const u = i / N;
          const r = 0.12 + 0.88 * u;
          const ang = u * 5.6 + arm * Math.PI + t * 0.80;
          const v = vals[Math.min(W - 1, Math.round(u * (W - 1)))];
          const rgb = u < 0.3 ? CYAN : u < 0.7 ? RED : WHITE;
          const lv = Math.min(7, 2 + Math.round(v * 5));

          const x = cx + Math.sin(ang) * 50 * r;
          const y = cy - Math.cos(ang) * 8.2 * r;
          P(ctx, Math.round(x), Math.round(y), rgb, lv);

          for (let d = 1; d <= 2; d++) {
            const trail = ang - d * 0.075;
            P(ctx, Math.round(cx + Math.sin(trail) * 50 * r), Math.round(cy - Math.cos(trail) * 8.2 * r), rgb, d === 1 ? 3 : 1);
          }
        }
      }

      for (let dy = -1; dy <= 1; dy++) {
        for (let dx = -1; dx <= 1; dx++) P(ctx, cx + dx, cy + dy, WHITE, 7);
      }
      P(ctx, cx, cy, WHITE, 7);
    }
  });

  // 06 — Bouncing balls: one spring ball per band.
  STYLES.push({
    id: "bounce",
    name: "弹跳球",
    tag: "全新",
    meta: "RAM ≈56 B · Flash +200 B",
    desc: "14 个受重力和弹簧控制的小球，每个球对应一个频段；低频让球弹得更高。",
    init: function () {
      this.balls = [];
      for (let i = 0; i < 14; i++) {
        this.balls.push({ y: H - 1, vy: 0 });
      }
      this.v = new Float32Array(14);
      this.ready = false;
    },
    draw: function (ctx, t) {
      if (!this.ready) {
        for (let i = 0; i < 14; i++) {
          this.v[i] = bandMax(i * 8, i * 8 + 7);
          this.balls[i].y = H - 1 - this.v[i] * 12;
        }
        this.ready = true;
      }
      for (let x = 0; x < W; x++) P(ctx, x, H - 1, palette.bar, 1);

      for (let i = 0; i < 14; i++) {
        const target = bandMax(i * 8, i * 8 + 7);
        this.v[i] += (target - this.v[i]) * 0.30;
        const ball = this.balls[i];
        const targetY = H - 1 - this.v[i] * 12;

        ball.vy += (targetY - ball.y) * 0.17;
        ball.vy *= 0.80;
        ball.y += ball.vy;
        if (ball.y > H - 1) {
          ball.y = H - 1;
          ball.vy *= -0.34;
        }
        if (ball.y < 0) {
          ball.y = 0;
          ball.vy = Math.abs(ball.vy) * 0.30;
        }

        const x = 6 + i * 7;
        const y = Math.round(ball.y);
        const rgb = i < 5 ? CYAN : i < 10 ? RED : WHITE;
        for (let dy = 0; dy < 3; dy++) {
          for (let dx = 0; dx < 3; dx++) P(ctx, x + dx, y + dy, rgb, 7);
        }
        P(ctx, x + 1, y + 1, WHITE, 7);
        P(ctx, x + 1, H - 1, palette.bar, 2);
      }
    }
  });

  // 07 — Big digits: eight 3×5 glyphs, one per band.
  const FONT = [
    ["111", "101", "101", "101", "111"],
    ["010", "110", "010", "010", "111"],
    ["111", "001", "111", "100", "111"],
    ["111", "001", "111", "001", "111"],
    ["101", "101", "111", "001", "001"],
    ["111", "100", "111", "001", "111"],
    ["111", "100", "111", "101", "111"],
    ["111", "001", "001", "001", "001"],
    ["111", "101", "111", "101", "111"],
    ["111", "101", "111", "001", "111"]
  ];

  STYLES.push({
    id: "digits",
    name: "大字频谱",
    tag: "全新",
    meta: "RAM 0 B · Flash +350 B",
    desc: "把 8 个频段直接显示成 8 个超大数字，每个数字 0–9；关闭音乐后也能看到电平回落。",
    init: function () {
      this.v = new Float32Array(8);
      this.d = new Int8Array(8);
    },
    draw: function (ctx, t) {
      for (let b = 0; b < 8; b++) {
        const from = Math.round((b / 8) * (W - 1));
        const to = Math.round(((b + 1) / 8) * (W - 1)) - 1;
        const target = bandAvg(from, Math.max(from, to));
        this.v[b] += (target - this.v[b]) * 0.28;
        const digit = Math.max(0, Math.min(9, Math.floor(this.v[b] * 10)));
        this.d[b] = digit;

        const x0 = 6 + b * 12;
        const y0 = 1;
        const rgb = b < 3 ? CYAN : b < 6 ? RED : WHITE;
        const rows = FONT[digit];

        for (let row = 0; row < 5; row++) {
          for (let col = 0; col < 3; col++) {
            if (rows[row][col] !== "1") continue;
            for (let yy = 0; yy < 3; yy++) {
              for (let xx = 0; xx < 3; xx++) {
                P(ctx, x0 + col * 3 + xx, y0 + row * 3 + yy, rgb, 7);
              }
            }
          }
        }
        for (let xx = 0; xx < 9; xx++) P(ctx, x0 + xx, H - 1, rgb, 2);
      }
    }
  });

  // 08 — Tunnel: expanding elliptical rings with radial grid.
  STYLES.push({
    id: "tunnel",
    name: "椭圆隧道",
    tag: "全新",
    meta: "RAM 0 B · Flash +250 B",
    desc: "多层椭圆环从中心向外扩张，低频让隧道整体脉冲；放射线提供透视骨架。",
    init: function () {},
    draw: function (ctx, t) {
      const cx = W / 2;
      const cy = H / 2;
      const RX = 54;
      const RY = 8.2;
      const bass = bandAvg(0, 24);

      for (let s = 0; s < 12; s++) {
        const a = (s / 12) * Math.PI * 2;
        for (let r = 0.18; r <= 1.001; r += 0.08) {
          P(ctx, Math.round(cx + Math.sin(a) * RX * r), Math.round(cy - Math.cos(a) * RY * r), palette.bar, 1);
        }
      }

      for (let k = 0; k < 8; k++) {
        let z = k / 8 + t * 0.22;
        z -= Math.floor(z);
        const scale = (0.15 + z * 0.85) * (0.94 + bass * 0.12);
        const lv = Math.max(1, Math.round(7 * (1 - z)));
        const rgb = k % 3 === 0 ? palette.dot : palette.bar;
        const steps = 104;
        for (let s = 0; s <= steps; s++) {
          const a = (s / steps) * Math.PI * 2;
          P(ctx, Math.round(cx + Math.sin(a) * RX * scale), Math.round(cy - Math.cos(a) * RY * scale), rgb, lv);
        }
      }

      for (let dy = -1; dy <= 1; dy++) {
        for (let dx = -1; dx <= 1; dx++) P(ctx, cx + dx, cy + dy, WHITE, 7);
      }
    }
  });

  // 09 — Fireworks: rockets launch on beats and explode into radial sparks.
  STYLES.push({
    id: "fireworks",
    name: "节拍烟花",
    tag: "全新",
    meta: "RAM ≈330 B · Flash +500 B",
    desc: "低频触发火箭，升空后炸成径向粒子；没有柱状底图，只有夜空、星点和节拍烟花。",
    init: function () {
      this.parts = [];
      this.rockets = [];
      this.stars = [];
      this.last = -1;
      this.seed = 1;
      for (let i = 0; i < 22; i++) {
        this.stars.push({ x: (i * 37 + 5) % W, y: (i * 11 + 2) % (H - 4), ph: i * 1.37 });
      }
      // Start with one visible burst so the first frame is not an empty sky.
      for (let n = 0; n < 18; n++) {
        const ang = (n / 18) * Math.PI * 2 + noise(this.seed++) * 0.25;
        const sp = 0.08 + noise(this.seed++) * 0.12;
        this.parts.push({
          x: 56,
          y: 7,
          vx: Math.cos(ang) * sp,
          vy: Math.sin(ang) * sp,
          life: 50 + Math.floor(noise(this.seed++) * 28),
          rgb: n % 5 === 0 ? WHITE : palette.bar
        });
      }
    },
    draw: function (ctx, t) {
      for (let i = 0; i < this.stars.length; i++) {
        const s = this.stars[i];
        const tw = 0.5 + 0.5 * Math.sin(t * 1.4 + s.ph);
        P(ctx, s.x, s.y, i % 6 === 0 ? palette.dot : palette.bar, 1 + Math.round(tw));
      }

      for (let x = 0; x < W; x++) P(ctx, x, H - 1, palette.bar, 1);

      const bass = bandMax(0, 22);
      const gap = 0.36 - bass * 0.18;
      if (this.last < 0 || t - this.last > gap) {
        this.last = t;
        if (this.rockets.length < 3) {
          this.rockets.push({
            x: 12 + Math.floor(noise(this.seed++) * (W - 24)),
            y: H - 1,
            vy: -0.32 - noise(this.seed++) * 0.14,
            ty: 3 + Math.floor(noise(this.seed++) * 4)
          });
        }
      }

      for (let i = this.rockets.length - 1; i >= 0; i--) {
        const r = this.rockets[i];
        r.y += r.vy;
        r.vy += 0.003;
        if (r.y <= r.ty) {
          const count = 10 + Math.floor(noise(this.seed++) * 6);
          for (let n = 0; n < count; n++) {
            const ang = (n / count) * Math.PI * 2 + noise(this.seed++) * 0.3;
            const sp = 0.08 + noise(this.seed++) * 0.12;
            this.parts.push({
              x: r.x,
              y: r.y,
              vx: Math.cos(ang) * sp,
              vy: Math.sin(ang) * sp,
              life: 50 + Math.floor(noise(this.seed++) * 28),
              rgb: n % 5 === 0 ? WHITE : palette.bar
            });
          }
          this.rockets.splice(i, 1);
          continue;
        }
        if (r.y > H + 2) {
          this.rockets.splice(i, 1);
          continue;
        }
        const x = Math.round(r.x);
        const y = Math.round(r.y);
        P(ctx, x, y, WHITE, 7);
        P(ctx, x, y + 1, palette.dot, 4);
        P(ctx, x, y + 2, palette.bar, 2);
      }

      for (let i = this.parts.length - 1; i >= 0; i--) {
        const p = this.parts[i];
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.0025;
        p.life--;
        if (p.life <= 0 || p.x < 0 || p.x >= W || p.y < 0 || p.y > H - 1) {
          this.parts.splice(i, 1);
          continue;
        }
        P(ctx, Math.round(p.x), Math.round(p.y), p.life > 40 ? WHITE : p.rgb, p.life > 22 ? 7 : 3);
      }
    }
  });

  // 10 — Laser harp: six vibrating horizontal strings.
  STYLES.push({
    id: "harp",
    name: "激光竖琴",
    tag: "全新",
    meta: "RAM 0 B · Flash +300 B",
    desc: "6 根横向激光弦，每根对应一个频段；振幅越大弦越弯，移动的白色拨弦点扫过琴弦。",
    init: function () {},
    draw: function (ctx, t) {
      for (let s = 0; s < 6; s++) {
        const y = 2 + s * 3;
        const v = bandMax(Math.round((s / 6) * W), Math.round(((s + 1) / 6) * W) - 1);
        const amp = 0.45 + v * 2.55;
        const rgb = s < 2 ? CYAN : s < 4 ? RED : WHITE;
        const lv = 4 + Math.round(v * 3);

        for (let x = 0; x < W; x++) {
          const env = Math.sin((Math.PI * x) / (W - 1));
          const disp = Math.sin(x * 0.40 + t * 5.0 + s * 1.1) * amp * env;
          let yy = y + Math.round(disp);
          yy = Math.max(0, Math.min(H - 1, yy));
          P(ctx, x, yy, rgb, lv);
          if (Math.abs(disp) > 1.2) P(ctx, x, yy - 1, rgb, 2);
        }

        P(ctx, 0, y, rgb, 7);
        P(ctx, W - 1, y, rgb, 7);

        const px = Math.floor((t * 26 + s * 17) % W);
        P(ctx, px, y, WHITE, 7);
        P(ctx, px, y - 1, WHITE, 3);
        P(ctx, px, y + 1, WHITE, 3);
      }
    }
  });

  const grid = document.getElementById("grid");
  const panels = [];

  STYLES.forEach(function (st, index) {
    const card = document.createElement("article");
    card.className = "card" + (st.tag === "全新" ? " new" : "");
    card.innerHTML =
      '<div class="card-head">' +
        '<span class="idx">' + String(index + 1).padStart(2, "0") + '</span>' +
        '<h2 class="name">' + st.name + '</h2>' +
        '<span class="tag' + (st.tag === "全新" ? " new" : "") + '">' + st.tag + '</span>' +
      '</div>' +
      '<p class="desc">' + st.desc + '</p>';

    const screen = document.createElement("div");
    screen.className = "screen";
    const canvas = document.createElement("canvas");
    canvas.width = CW;
    canvas.height = CH;
    canvas.setAttribute("aria-label", st.name + " 频谱预览");
    screen.appendChild(canvas);
    card.appendChild(screen);

    const meta = document.createElement("div");
    meta.className = "meta";
    meta.innerHTML = '<span>' + st.meta + '</span><span class="dotline">112 × 18</span>';
    card.appendChild(meta);

    grid.appendChild(card);

    const ctx = canvas.getContext("2d");
    const off = document.createElement("canvas");
    off.width = CW;
    off.height = CH;
    const octx = off.getContext("2d");

    st.init();
    panels.push({ st: st, ctx: ctx, off: off, octx: octx });
  });

  const playBtn = document.getElementById("play");
  const resetBtn = document.getElementById("reset");
  const speedInput = document.getElementById("speed");
  const speedLabel = document.getElementById("speedLabel");
  const mappingSelect = document.getElementById("mapping");
  const paletteSelect = document.getElementById("palette");
  const clock = document.getElementById("clock");

  let playing = true;
  let simT = 0;
  let speed = 1;
  let mapping = "log";
  let last = performance.now();
  let acc = 0;
  const STEP = 0.05;

  function formatTime(t) {
    const m = Math.floor(t / 60);
    const s = (t % 60).toFixed(1);
    return String(m).padStart(2, "0") + ":" + s.padStart(4, "0");
  }

  // Draw into an offscreen buffer, then submit one complete frame. The visible
  // canvas is never cleared separately, which removes the clear/draw flash.
  function drawFrame(t) {
    sampleSpectrum(t, mapping);
    for (let i = 0; i < panels.length; i++) {
      const p = panels[i];
      clear(p.octx);
      p.st.draw(p.octx, t);
      p.ctx.drawImage(p.off, 0, 0);
    }
  }

  function updateClock() {
    clock.textContent = formatTime(simT);
  }

  function setPlaying(next) {
    playing = next;
    playBtn.textContent = playing ? "暂停" : "播放";
    playBtn.classList.toggle("primary", playing);
    last = performance.now();
    acc = 0;
  }

  playBtn.addEventListener("click", function () {
    setPlaying(!playing);
  });

  resetBtn.addEventListener("click", function () {
    simT = 0;
    for (let i = 0; i < panels.length; i++) panels[i].st.init();
    drawFrame(0);
    updateClock();
    setPlaying(true);
  });

  speedInput.addEventListener("input", function () {
    speed = Number(speedInput.value);
    speedLabel.textContent = speed.toFixed(1) + "×";
  });

  mappingSelect.addEventListener("change", function () {
    mapping = mappingSelect.value;
    for (let i = 0; i < panels.length; i++) panels[i].st.init();
    drawFrame(simT);
  });

  paletteSelect.addEventListener("change", function () {
    palette = PALETTES[Number(paletteSelect.value)];
    drawFrame(simT);
  });

  document.addEventListener("keydown", function (event) {
    if (event.code === "Space" && event.target === document.body) {
      event.preventDefault();
      setPlaying(!playing);
    }
  });

  function frame(now) {
    requestAnimationFrame(frame);
    const dt = Math.min(0.25, (now - last) / 1000);
    last = now;
    if (!playing) return;
    acc += dt;
    if (acc < STEP) return;
    acc = Math.min(acc, STEP * 2);
    simT += STEP * speed;
    drawFrame(simT);
    updateClock();
  }

  drawFrame(0);
  updateClock();
  requestAnimationFrame(frame);

  window.fftDemo = {
    styles: STYLES,
    panels: panels,
    redraw: function () { drawFrame(simT); },
    setTime: function (t) { simT = t; drawFrame(t); updateClock(); },
    reset: function () { simT = 0; panels.forEach(function (p) { p.st.init(); }); drawFrame(0); }
  };



})();
