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
    { name: "青柱 / 红点", bar: CYAN, dot: RED },
    { name: "红柱 / 青点", bar: RED, dot: CYAN },
    { name: "白柱 / 青点", bar: WHITE, dot: CYAN },
    { name: "红柱 / 白点", bar: RED, dot: WHITE },
    { name: "青白 / 红点", bar: CYAN, dot: WHITE }
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

  // Deterministic pseudo-random, so all ten panels see the same music.
  function noise(n) {
    const x = Math.sin(n * 127.1 + 311.7) * 43758.5453123;
    return x - Math.floor(x);
  }

  function energy(f, t) {
    const beat = (t % 0.5) / 0.5;
    const kick = Math.exp(-beat * 5.6);
    const section = 0.78 + 0.22 * Math.sin(t * 0.21 + 0.8);
    let v = 0;

    v += 0.96 * gauss(f, 68, 38) * (0.52 + 0.48 * kick) * section;
    const bassF = 104 + 58 * Math.sin(t * 0.61);
    v += 0.78 * gauss(f, bassF, 23) * (0.48 + 0.52 * kick);

    const melF = 620 * Math.pow(2, Math.sin(t * 0.39) * 1.12);
    v += 0.58 * gauss(f, melF, 86) * (0.32 + 0.68 * Math.max(0, Math.sin(t * 3.05 + 1.1)));
    v += 0.34 * gauss(f, melF * 2.03, 150) * (0.28 + 0.72 * Math.max(0, Math.sin(t * 2.37 + 0.5)));

    v += 0.70 * gauss(f, 3600, 940) * kick;
    v += 0.20 * gauss(f, 2350, 680) * (0.35 + 0.65 * (0.5 + 0.5 * Math.sin(t * 2.3 + f * 0.013)));
    v += 0.30 * gauss(f, 5200, 520) * Math.max(0, Math.sin(t * 1.73 + 0.4));

    v += 0.035 + 0.045 * (0.5 + 0.5 * Math.sin(t * 3.7 + f * 0.05 + 1.4));
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
      const flicker = 0.94 + 0.06 * Math.sin(t * 9.0 + i * 0.37);
      vals[i] = energy(f, t) * flicker;
    }
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

  function clamp01(v) {
    return v < 0 ? 0 : v > 1 ? 1 : v;
  }

  let palette = PALETTES[0];

  const STYLES = [];

  // 01 — classic thin bars with peak dots.
  STYLES.push({
    id: "bars",
    name: "经典柱状",
    tag: "现有",
    meta: "RAM 0 B · Flash +150 B",
    desc: "112 根细柱逐列上升，柱顶高光，峰值点缓慢下落。最稳的默认风格。",
    init: function () {
      this.v = new Float32Array(W);
      this.pk = new Float32Array(W);
    },
    draw: function (ctx) {
      smoothInto(this.v, vals, 0.55, 0.10);
      for (let i = 0; i < W; i++) {
        const h = Math.min(H - 1, Math.round(this.v[i] * (H - 1)));
        for (let j = 0; j < h; j++) {
          P(ctx, i, H - 1 - j, palette.bar, Math.min(6, 1 + Math.round(((j + 1) / h) * 5)));
        }
        if (h > 0) P(ctx, i, H - h, palette.bar, 7);
        this.pk[i] = peakStep(this.pk[i], h, 0.35);
        if (this.pk[i] > h + 1.2) {
          P(ctx, i, H - 1 - Math.round(this.pk[i]), palette.dot, 6);
        }
      }
    }
  });

  // 02 — folded left/right mirror. Low frequencies at the edges, highs at center.
  STYLES.push({
    id: "mirror",
    name: "左右镜像",
    tag: "现有",
    meta: "RAM 0 B · Flash +80 B",
    desc: "只计算 56 段，左右完全对称；低频在两侧、高频在中央，画面像展开的蝶翼。",
    init: function () {
      this.v = new Float32Array(W / 2);
      this.pk = new Float32Array(W / 2);
      this.target = new Float32Array(W / 2);
    },
    draw: function (ctx) {
      const half = W / 2;
      for (let i = 0; i < half; i++) {
        this.target[i] = Math.max(vals[i * 2], vals[i * 2 + 1]);
      }
      smoothInto(this.v, this.target, 0.55, 0.10);
      for (let i = 0; i < half; i++) {
        const h = Math.min(H - 1, Math.round(this.v[i] * (H - 1)));
        for (let j = 0; j < h; j++) {
          const lv = Math.min(6, 1 + Math.round(((j + 1) / h) * 5));
          P(ctx, i, H - 1 - j, palette.bar, lv);
          P(ctx, W - 1 - i, H - 1 - j, palette.bar, lv);
        }
        if (h > 0) {
          P(ctx, i, H - h, palette.bar, 7);
          P(ctx, W - 1 - i, H - h, palette.bar, 7);
        }
        this.pk[i] = peakStep(this.pk[i], h, 0.35);
        if (this.pk[i] > h + 1.2) {
          const py = H - 1 - Math.round(this.pk[i]);
          P(ctx, i, py, palette.dot, 6);
          P(ctx, W - 1 - i, py, palette.dot, 6);
        }
      }
    }
  });

  // 03 — smooth envelope with a filled body and local peak accents.
  STYLES.push({
    id: "curve",
    name: "包络曲线",
    tag: "现有",
    meta: "RAM 0 B · Flash +220 B",
    desc: "把频谱连成平滑包络，底部用低亮度填充，再在局部峰顶点亮高光。",
    init: function () {
      this.v = new Float32Array(W);
      this.y = new Int16Array(W);
    },
    draw: function (ctx) {
      smoothInto(this.v, vals, 0.62, 0.09);
      for (let i = 0; i < W; i++) {
        this.y[i] = H - 1 - Math.round(this.v[i] * (H - 1));
      }
      for (let i = 0; i < W; i++) {
        const y = this.y[i];
        for (let yy = y + 1; yy < H; yy++) {
          const d = yy - y;
          P(ctx, i, yy, palette.bar, d <= 2 ? 3 : d <= 5 ? 2 : 1);
        }
        P(ctx, i, y, palette.bar, 7);
        if (i + 1 < W) {
          const y2 = this.y[i + 1];
          const lo = Math.min(y, y2);
          const hi = Math.max(y, y2);
          for (let yy = lo; yy <= hi; yy++) P(ctx, i, yy, palette.bar, 6);
        }
        if (i > 0 && i < W - 1 && this.v[i] > 0.35 && this.v[i] >= this.v[i - 1] && this.v[i] > this.v[i + 1]) {
          P(ctx, i, y - 1, palette.dot, 7);
          P(ctx, i, y - 2, palette.dot, 3);
        }
      }
    }
  });

  // 04 — discrete dot columns with a two-pixel rhythm.
  STYLES.push({
    id: "dots",
    name: "点阵柱",
    tag: "现有",
    meta: "RAM 0 B · Flash +40 B",
    desc: "不画连续柱身，只隔行点亮方形点阵；更轻、更接近老式 VFD 的电平条。",
    init: function () {
      this.v = new Float32Array(W);
      this.pk = new Float32Array(W);
    },
    draw: function (ctx) {
      smoothInto(this.v, vals, 0.58, 0.11);
      for (let i = 0; i < W; i++) {
        const h = Math.min(H - 1, Math.round(this.v[i] * (H - 1)));
        for (let j = 0; j < h; j += 2) {
          P(ctx, i, H - 1 - j, palette.bar, Math.min(6, 2 + Math.round(((j + 1) / Math.max(1, h)) * 4)));
        }
        if (h > 0) P(ctx, i, H - h, palette.bar, 7);
        this.pk[i] = peakStep(this.pk[i], h, 0.38);
        if (this.pk[i] > h + 1.2) P(ctx, i, H - 1 - Math.round(this.pk[i]), palette.dot, 7);
      }
    }
  });

  // 05 — scrolling spectrogram history.
  STYLES.push({
    id: "waterfall",
    name: "瀑布声谱",
    tag: "现有",
    meta: "RAM 252 B · Flash +260 B",
    desc: "把每一帧频谱压成 2-bit 历史，向下滚动；能看见节拍、扫频和余音的轨迹。",
    init: function () {
      this.cols = W / 2;
      this.hist = new Uint8Array(this.cols * H);
      // Seed one second of history so the panel is readable immediately.
      for (let r = 0; r < H; r++) {
        sampleSpectrum(0.05 * r, "log");
        const base = r * this.cols;
        for (let c = 0; c < this.cols; c++) {
          const v = Math.max(vals[c * 2], vals[c * 2 + 1]);
          this.hist[base + c] = v < 0.06 ? 0 : v < 0.24 ? 1 : v < 0.52 ? 2 : 3;
        }
      }
    },
    draw: function (ctx) {
      const cols = this.cols;
      const hist = this.hist;
      hist.copyWithin(0, cols);
      const base = cols * (H - 1);
      for (let c = 0; c < cols; c++) {
        const v = Math.max(vals[c * 2], vals[c * 2 + 1]);
        hist[base + c] = v < 0.06 ? 0 : v < 0.24 ? 1 : v < 0.52 ? 2 : 3;
      }
      for (let r = 0; r < H; r++) {
        for (let c = 0; c < cols; c++) {
          const lv = hist[r * cols + c];
          if (!lv) continue;
          const rgb = lv >= 3 ? palette.dot : palette.bar;
          const g = lv === 1 ? 2 : lv === 2 ? 4 : 7;
          P(ctx, c * 2, r, rgb, g);
          P(ctx, c * 2 + 1, r, rgb, g);
        }
      }
    }
  });

  // 06 — bars with a particle fountain at rising peaks.
  STYLES.push({
    id: "spark",
    name: "节拍火花",
    tag: "现有",
    meta: "RAM ≈96 B · Flash +300 B",
    desc: "柱体只做底衬，柱顶在鼓点瞬间喷出带重力的火花；适合节奏强的音乐。",
    init: function () {
      this.v = new Float32Array(W);
      this.pk = new Float32Array(W);
      this.ps = [];
      this.seed = 1;
    },
    draw: function (ctx) {
      smoothInto(this.v, vals, 0.72, 0.13);
      for (let i = 0; i < W; i++) {
        const h = Math.min(H - 1, Math.round(this.v[i] * (H - 1)));
        for (let j = 0; j < h; j++) {
          P(ctx, i, H - 1 - j, palette.bar, Math.min(5, 1 + Math.round(((j + 1) / Math.max(1, h)) * 4)));
        }
        if (h > 0) P(ctx, i, H - h, palette.bar, 6);

        if (h > this.pk[i] + 2 && this.ps.length < 64) {
          const count = 1 + Math.floor(noise(this.seed++) * 3);
          for (let n = 0; n < count; n++) {
            this.ps.push({
              x: i + (noise(this.seed++) - 0.5) * 1.2,
              y: H - 1 - h,
              vx: (noise(this.seed++) - 0.5) * 0.32,
              vy: -0.55 - noise(this.seed++) * 0.75,
              life: 12 + Math.floor(noise(this.seed++) * 10)
            });
          }
        }
        this.pk[i] = peakStep(this.pk[i], h, 0.24);
      }

      for (let n = this.ps.length - 1; n >= 0; n--) {
        const p = this.ps[n];
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.17;
        p.life--;
        if (p.y >= H - 1) {
          p.y = H - 1;
          p.vy *= -0.32;
          p.vx *= 0.65;
          p.life -= 4;
        }
        if (p.life <= 0 || p.x < 0 || p.x >= W) {
          this.ps.splice(n, 1);
          continue;
        }
        const rgb = p.life > 12 && n % 4 === 0 ? WHITE : palette.dot;
        P(ctx, Math.round(p.x), Math.round(p.y), rgb, p.life > 5 ? 7 : 4);
      }
    }
  });

  // 07 — wide elliptical VU gauge.
  STYLES.push({
    id: "vu",
    name: "椭圆 VU",
    tag: "现有",
    meta: "RAM 0 B · Flash +260 B",
    desc: "把整段频谱加权成一个电平，用宽椭圆表盘、指针和峰值保持来表现整体响度。",
    init: function () {
      this.needle = 0;
      this.peak = 0;
      this.hold = 0;
    },
    draw: function (ctx, t) {
      let sum = 0;
      let weight = 0;
      for (let i = 0; i < W; i++) {
        const w = 1.35 - (i / W) * 0.65;
        sum += vals[i] * w;
        weight += w;
      }
      const lv = clamp01((sum / weight) * 1.62);
      this.needle += (lv - this.needle) * (lv > this.needle ? 0.18 : 0.035);
      if (lv >= this.peak) {
        this.peak = lv;
        this.hold = 26;
      } else if (this.hold > 0) {
        this.hold--;
      } else {
        this.peak = Math.max(lv, this.peak - 0.004);
      }

      const cx = W / 2;
      const cy = H - 1;
      const RX = 46;
      const RY = 15;
      const amin = -70 * Math.PI / 180;
      const amax = 70 * Math.PI / 180;

      function pt(a, r) {
        return [cx + Math.sin(a) * RX * r, cy - Math.cos(a) * RY * r];
      }

      for (let s = 0; s <= 20; s++) {
        const a = amin + (s / 20) * (amax - amin);
        const major = s % 5 === 0;
        const p = pt(a, major ? 1.06 : 1.0);
        P(ctx, Math.round(p[0]), Math.round(p[1]), palette.bar, major ? 5 : 2);
      }
      for (let s = 0; s <= 40; s++) {
        const p = pt(amin + (s / 40) * (amax - amin), 1.0);
        P(ctx, Math.round(p[0]), Math.round(p[1]), palette.bar, 2);
      }

      const na = amin + this.needle * (amax - amin);
      for (let s = 2; s <= 26; s++) {
        const p = pt(na, s / 26);
        P(ctx, Math.round(p[0]), Math.round(p[1]), palette.dot, s > 22 ? 7 : 5);
      }
      const tip = pt(na, 0.96);
      P(ctx, Math.round(tip[0]), Math.round(tip[1]), WHITE, 7);

      const pp = pt(amin + this.peak * (amax - amin), 1.02);
      P(ctx, Math.round(pp[0]), Math.round(pp[1]), WHITE, 7);
      P(ctx, Math.round(pp[0]), Math.round(pp[1]) - 1, WHITE, 4);

      P(ctx, cx, cy, WHITE, 7);
      P(ctx, cx - 1, cy, palette.bar, 5);
      P(ctx, cx + 1, cy, palette.bar, 5);

      const n = Math.round(lv * W);
      for (let i = 0; i < n; i++) {
        P(ctx, i, H - 1, palette.dot, i > W * 0.82 ? 7 : 4);
      }
    }
  });

  // 08 — three fixed frequency bands: low cyan, mid red, high white.
  STYLES.push({
    id: "split",
    name: "三色分段",
    tag: "现有",
    meta: "RAM 0 B · Flash +40 B",
    desc: "低频青、中频红、高频白，颜色直接表达频段；柱顶统一白色高光。",
    init: function () {
      this.v = new Float32Array(W);
      this.pk = new Float32Array(W);
    },
    draw: function (ctx) {
      smoothInto(this.v, vals, 0.56, 0.10);
      for (let i = 0; i < W; i++) {
        const c = i < W * 0.28 ? CYAN : i < W * 0.66 ? RED : WHITE;
        const h = Math.min(H - 1, Math.round(this.v[i] * (H - 1)));
        for (let j = 0; j < h; j++) {
          P(ctx, i, H - 1 - j, c, Math.min(6, 1 + Math.round(((j + 1) / Math.max(1, h)) * 5)));
        }
        if (h > 0) P(ctx, i, H - h, WHITE, 7);
        this.pk[i] = peakStep(this.pk[i], h, 0.35);
        if (this.pk[i] > h + 1.2) P(ctx, i, H - 1 - Math.round(this.pk[i]), palette.dot, 6);
      }
    }
  });

  // 09 — chunky LED-wall blocks. Deliberately different rhythm from thin bars.
  STYLES.push({
    id: "blocks",
    name: "大块矩阵",
    tag: "新增",
    meta: "RAM 0 B · Flash +80 B",
    desc: "14 列 × 4 行的大块 LED 墙，能量被量化成整块；顶部有一块缓慢下落的峰值。",
    init: function () {
      this.cols = 14;
      this.v = new Float32Array(this.cols);
      this.pk = new Float32Array(this.cols);
      this.target = new Float32Array(this.cols);
    },
    draw: function (ctx) {
      const cols = this.cols;
      for (let c = 0; c < cols; c++) {
        const idx = Math.round((c / (cols - 1)) * (W - 1));
        let v = 0;
        for (let d = -2; d <= 2; d++) {
          const i = Math.max(0, Math.min(W - 1, idx + d));
          v = Math.max(v, vals[i]);
        }
        this.target[c] = v;
      }
      smoothInto(this.v, this.target, 0.55, 0.13);

      for (let c = 0; c < cols; c++) {
        const level = Math.round(this.v[c] * 4);
        const x = c * 8;
        for (let r = 0; r < level; r++) {
          const y = 14 - r * 4;
          const rgb = r === 0 ? palette.bar : r === 1 ? palette.bar : r === 2 ? palette.dot : WHITE;
          const lv = r === 0 ? 5 : 7;
          for (let yy = 0; yy < 3; yy++) {
            for (let xx = 0; xx < 7; xx++) P(ctx, x + xx, y + yy, rgb, lv);
          }
        }
        this.pk[c] = peakStep(this.pk[c], level, 0.07);
        if (this.pk[c] > level + 0.5) {
          const py = 17 - Math.round(this.pk[c] * 4);
          for (let xx = 0; xx < 7; xx++) P(ctx, x + xx, py, palette.dot, 6);
        }
      }

      for (let x = 0; x < W; x++) P(ctx, x, H - 1, palette.bar, 1);
    }
  });

  // 10 — semicircular radar: range rings, 24 frequency spokes, ping-pong sweep.
  STYLES.push({
    id: "radar",
    name: "半圆雷达",
    tag: "新增",
    meta: "RAM ≈96 B · Flash +500 B",
    desc: "以屏幕底边为圆心的半圆雷达：距离环、24 根频率辐条和往返扫描线，极坐标构图。",
    init: function () {
      this.n = 24;
      this.v = new Float32Array(this.n);
      this.pk = new Float32Array(this.n);
    },
    draw: function (ctx, t) {
      const CX = W / 2;
      const CY = H - 1;
      const RX = 50;
      const RY = 15;
      const n = this.n;

      function pt(a, r) {
        return [CX + Math.sin(a) * RX * r, CY - Math.cos(a) * RY * r];
      }

      // Range rings.
      for (let ring = 1; ring <= 3; ring++) {
        const rr = ring / 3;
        const steps = 96;
        for (let s = 0; s <= steps; s++) {
          const a = -Math.PI / 2 + (s / steps) * Math.PI;
          const p = pt(a, rr);
          P(ctx, Math.round(p[0]), Math.round(p[1]), palette.bar, ring === 3 ? 3 : 2);
        }
      }

      // Radial grid, every 30 degrees.
      for (let s = 0; s <= 6; s++) {
        const a = -Math.PI / 2 + (s / 6) * Math.PI;
        for (let r = 0.15; r <= 1.001; r += 0.08) {
          const p = pt(a, r);
          P(ctx, Math.round(p[0]), Math.round(p[1]), palette.bar, 1);
        }
      }

      // Spectrum spokes. Low frequencies on the left, highs on the right.
      for (let k = 0; k < n; k++) {
        const t0 = k / (n - 1);
        const idx = t0 * (W - 1);
        let target = 0;
        for (let d = -2; d <= 2; d++) {
          const i = Math.max(0, Math.min(W - 1, Math.round(idx) + d));
          target = Math.max(target, vals[i]);
        }
        this.v[k] += (target - this.v[k]) * 0.55;
        const v = this.v[k];
        const a = -Math.PI / 2 + t0 * Math.PI;
        const len = 0.18 + 0.82 * v;
        const rgb = t0 < 0.28 ? CYAN : t0 < 0.66 ? RED : WHITE;

        for (let r = 0.16; r <= len; r += 0.035) {
          const p = pt(a, r);
          P(ctx, Math.round(p[0]), Math.round(p[1]), rgb, r > len - 0.07 ? 7 : 5);
        }
        this.pk[k] = peakStep(this.pk[k], v, 0.012);
        if (this.pk[k] > v + 0.035) {
          const p = pt(a, 0.18 + 0.82 * this.pk[k]);
          P(ctx, Math.round(p[0]), Math.round(p[1]), palette.dot, 6);
        }
      }

      // Sweep: ping-pong across the semicircle, with a short fading trail.
      const phase = (t * 0.62) % 2;
      const sweep = -Math.PI / 2 + (phase < 1 ? phase : 2 - phase) * Math.PI;
      for (let d = 10; d >= 0; d--) {
        const a = sweep - d * 0.023;
        if (a < -Math.PI / 2 || a > Math.PI / 2) continue;
        const lv = d === 0 ? 7 : d < 3 ? 5 : d < 6 ? 2 : 1;
        for (let r = 0.16; r <= 1.001; r += 0.045) {
          const p = pt(a, r);
          P(ctx, Math.round(p[0]), Math.round(p[1]), d < 2 ? WHITE : palette.dot, lv);
        }
      }

      // Echo blips near the sweep.
      for (let k = 0; k < n; k++) {
        if (this.v[k] < 0.52) continue;
        const a = -Math.PI / 2 + (k / (n - 1)) * Math.PI;
        let diff = Math.abs(a - sweep);
        if (diff > Math.PI) diff = Math.PI * 2 - diff;
        if (diff < 0.26) {
          const p = pt(a, 0.18 + 0.82 * this.v[k]);
          const x = Math.round(p[0]);
          const y = Math.round(p[1]);
          P(ctx, x, y, WHITE, 7);
          P(ctx, x + 1, y, palette.dot, 5);
          P(ctx, x, y + 1, palette.dot, 4);
        }
      }

      P(ctx, CX, CY, WHITE, 7);
      for (let x = 0; x < W; x++) P(ctx, x, H - 1, palette.bar, 1);
    }
  });

  const grid = document.getElementById("grid");
  const panels = [];

  STYLES.forEach(function (st, index) {
    const card = document.createElement("article");
    card.className = "card" + (st.tag === "新增" ? " new" : "");
    card.innerHTML =
      '<div class="card-head">' +
        '<span class="idx">' + String(index + 1).padStart(2, "0") + '</span>' +
        '<h2 class="name">' + st.name + '</h2>' +
        '<span class="tag' + (st.tag === "新增" ? " new" : "") + '">' + st.tag + '</span>' +
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

  // Useful for quick console checks while reviewing.
  window.fftDemo = {
    styles: STYLES,
    panels: panels,
    redraw: function () { drawFrame(simT); },
    setTime: function (t) { simT = t; drawFrame(t); updateClock(); },
    reset: function () { simT = 0; panels.forEach(function (p) { p.st.init(); }); drawFrame(0); }
  };



})();
