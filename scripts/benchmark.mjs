import { chromium } from "@playwright/test";
import fs from "node:fs";
fs.mkdirSync(".verification", { recursive: true });
const browser = await chromium.launch({ channel: "chrome", headless: true });
const page = await browser.newPage({ viewport: { width: 1366, height: 768 } });
const errors = [];
page.on("pageerror", (e) => errors.push(e.message));
await page.goto("http://127.0.0.1:5173/?qa=1");
await page.evaluate(() => window.__gardenQA.reset());
await page.locator("#start").click();
await page.waitForFunction(() => window.__gardenQA.audio.debug().unlocked);
const performance = await page.evaluate(async () => {
  const q = window.__gardenQA,
    b = q.battle();
  b.schedule = [{ at: 99999, row: 0, kind: 0 }];
  b.enemies = Array.from({ length: 100 }, (_, i) => ({
    id: 10000 + i,
    kind: i % 4,
    row: i % 5,
    x: 6 + (i % 20) * 0.15,
    hp: 1e9,
    attack: 999,
    slowUntil: 0,
    hit: 0,
  }));
  b.plants = Array.from({ length: 25 }, (_, i) => ({
    id: 30000 + i,
    kind: "pea",
    phenotype: "volley",
    genome: ["fast", "split"],
    primaryFamily: "pea",
    generation: 2,
    xp: 0,
    row: i % 5,
    col: Math.floor(i / 5),
    hp: 100,
    attack: b.time + 0.1,
    production: 0,
    born: b.time,
  }));
  let serial = 40000,
    maxSfx = 0,
    maxMusic = 0;
  const deltas = [],
    counts = [];
  let prev = 0;
  return new Promise((resolve) => {
    function frame(t) {
      while (b.bullets.length < 200)
        b.bullets.push({
          id: serial++,
          row: serial % 5,
          x: (serial % 30) * 0.1,
          damage: 1,
          slow: false,
        });
      const d = q.audio.debug();
      maxSfx = Math.max(maxSfx, d.activeSfx);
      maxMusic = Math.max(maxMusic, d.activeMusic);
      counts.push(b.bullets.length);
      if (prev) deltas.push(t - prev);
      prev = t;
      if (deltas.length < 360) requestAnimationFrame(frame);
      else {
        const sorted = [...deltas].sort((a, b) => a - b);
        resolve({
          fps: (1000 * deltas.length) / deltas.reduce((a, b) => a + b, 0),
          p95FrameMs: sorted[Math.floor(sorted.length * 0.95)],
          maxSfx,
          maxMusic,
          minProjectiles: Math.min(...counts),
          enemies: b.enemies.length,
          plants: b.plants.length,
          timeAdvanced: b.time,
          frames: deltas.length,
        });
      }
    }
    requestAnimationFrame(frame);
  });
});
const synthesis = await page.evaluate(async () => {
  const q = window.__gardenQA,
    { SOUNDS } = await import("/src/audio.ts");
  q.audio.reset();
  for (const name of Object.keys(SOUNDS)) {
    q.audio.play(name);
    await new Promise((r) => setTimeout(r, 100));
  }
  const samples = [...q.audio.cache]
    .filter(([key]) => !key.startsWith("n"))
    .map(([key, b]) => {
      const samples = b.getChannelData(0);
      let sum = 0,
        peak = 0;
      for (const v of samples) {
        sum += v * v;
        peak = Math.max(peak, Math.abs(v));
      }
      return {
        name: key,
        rms: Math.sqrt(sum / samples.length),
        peak,
        length: samples.length,
      };
    });
  q.audio.reset();
  return { samples, afterReset: q.audio.debug() };
});
const result = { performance, synthesis, errors, version: browser.version() };
fs.writeFileSync(
  ".verification/v03-performance.json",
  JSON.stringify(result, null, 2),
);
console.log({
  performance,
  errors,
  sounds: synthesis.samples.length,
  afterReset: synthesis.afterReset,
});
await browser.close();
