import { test, expect, type Page } from "@playwright/test";
async function start(page: Page) {
  await page.goto("/?qa=1");
  await page.click("#expedition");
  for (let i = 0; i < 3; i++)
    await page.locator("[data-draft]").first().click();
  await page.click("#start");
  await expect(page.locator("#overlay")).toBeHidden();
}
async function cell(page: Page, row: number, col: number) {
  const b = await page.locator("#game").boundingBox();
  await page.mouse.click(
    b!.x + ((116 + (col + 0.5) * 99) / 1120) * b!.width,
    b!.y + ((87 + (row + 0.5) * 93) / 620) * b!.height,
  );
}
async function fixture(page: Page) {
  await page.evaluate(() => {
    const q = (window as any).__gardenQA,
      b = q.battle();
    b.schedule = [{ at: 9999, row: 0, kind: 0 }];
    b.sun = 2000;
    b.suns = [];
    b.cooldowns = {};
    b.place("sun", 0, 0);
    b.place("pea", 0, 1);
    b.place("wall", 0, 2);
    b.drainEvents();
    q.renderHUD();
  });
}
test("home navigation and actual progress pages", async ({ page }) => {
  const errors: string[] = [];
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto("/");
  await expect(page.locator("#home h1")).toHaveText("异芽");
  await page.locator('.home-menu [data-page="base"]').click();
  await expect(page.locator("#archive h1")).toContainText("痕迹");
  await page.locator("#archive [data-home]").click();
  await page.locator('.home-menu [data-page="lab"]').click();
  await expect(page.locator(".recipes")).toContainText("60%");
  await expect(page.locator("#archive")).toContainText("四槽显式遗传");
  expect(errors).toEqual([]);
});
for (const [width, height] of [
  [1280, 720],
  [1366, 768],
  [1440, 900],
])
  test(`full board and scaled planting ${width}x${height}`, async ({
    page,
  }) => {
    await page.setViewportSize({ width, height });
    await start(page);
    await page.click('[data-kind="sun"]');
    await cell(page, 4, 0);
    await expect
      .poll(() =>
        page.evaluate(() =>
          (window as any).__gardenQA
            .battle()
            .plants.some((p: any) => p.row === 4 && p.col === 0),
        ),
      )
      .toBe(true);
    const bounds = await page.locator(".game-panel").boundingBox();
    expect(bounds!.y + bounds!.height).toBeLessThanOrEqual(height);
    expect(
      await page.evaluate(() => document.documentElement.scrollWidth),
    ).toBeLessThanOrEqual(width);
    await page.screenshot({ path: `.verification/v03-${width}x${height}.png` });
  });
test("collect once, plant failure retains card, tools cross sun to select parents", async ({
  page,
}) => {
  await start(page);
  await fixture(page);
  await page.click('[data-kind="pea"]');
  await cell(page, 0, 0);
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.interaction.state.type,
    ),
  ).toBe("plant");
  await page.keyboard.press("Escape");
  await page.evaluate(() => {
    const b = (window as any).__gardenQA.battle();
    b.suns = [{ id: 99999, row: 0, x: 0, born: b.time }];
  });
  await page.click("#fusion");
  await cell(page, 0, 0);
  await cell(page, 0, 1);
  await expect(page.locator("#confirm-fusion")).toBeVisible();
  expect(
    await page.evaluate(() => (window as any).__gardenQA.battle().suns.length),
  ).toBe(1);
  await page.click("#confirm-fusion");
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.battle().plants.length,
    ),
  ).toBe(2);
  await cell(page, 0, 0);
  expect(
    await page.evaluate(() => (window as any).__gardenQA.battle().suns.length),
  ).toBe(0);
});
test("gene sample preview, install, evolve, mutate, and cancel", async ({
  page,
}) => {
  await start(page);
  await fixture(page);
  await page.evaluate(() => {
    const q = (window as any).__gardenQA;
    q.run().samples = ["health"];
    q.renderHUD();
  });
  await page.click('[data-sample="0"]');
  await cell(page, 0, 1);
  await expect(page.locator("#inspector")).toContainText("100 → 135");
  await page.click("#install");
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.battle().plants[1].genome,
    ),
  ).toEqual(["health"]);
  await page.evaluate(() => {
    const q = (window as any).__gardenQA,
      p = q.battle().plants[1];
    p.xp = 61;
    p.evolutionChoices = ["tough", "agile", "expert"];
    q.renderHUD();
  });
  await page.click("#evolve");
  await page.locator('[data-evolution="tough"]').click();
  await expect(page.locator("#inspector")).toContainText("强韧");
  await page.click("#mutate");
  await expect(page.locator("#inspector")).toContainText("16.67%");
  await page.keyboard.press("Escape");
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.battle().plants[1].mutation,
    ),
  ).toBeUndefined();
  await page.click("#inspect");
  await cell(page, 0, 1);
  await page.click("#mutate");
  await page.click("#confirm-mutate");
  await expect(page.locator("#inspector")).toContainText("突变 ·");
});
test("dead materials revoke selection without charging or advancing randomness", async ({
  page,
}) => {
  await start(page);
  await fixture(page);
  await page.click("#fusion");
  await cell(page, 0, 0);
  await cell(page, 0, 1);
  const before = await page.evaluate(() => {
    const q = (window as any).__gardenQA,
      b = q.battle();
    b.remove(b.plants[0].id);
    q.renderHUD();
    return [b.sun, b.rng.hybrid.state];
  });
  await expect(page.locator("#confirm-fusion")).toHaveCount(0);
  expect(
    await page.evaluate(() => {
      const b = (window as any).__gardenQA.battle();
      return [b.sun, b.rng.hybrid.state];
    }),
  ).toEqual(before);
});
test("restart and exit confirmations freeze, cancel restores state, settings traps focus", async ({
  page,
}) => {
  await start(page);
  await page.click("#restart");
  await expect(
    page.getByRole("dialog", { name: "离开当前出征？" }),
  ).toBeVisible();
  const time = await page.evaluate(
    () => (window as any).__gardenQA.battle().time,
  );
  await page.waitForTimeout(180);
  expect(
    await page.evaluate(() => (window as any).__gardenQA.battle().time),
  ).toBe(time);
  await page.keyboard.press("1");
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.interaction.state.type,
    ),
  ).toBe("browse");
  await page.keyboard.press("Escape");
  expect(
    await page.evaluate(() => (window as any).__gardenQA.battle().status),
  ).toBe("running");
  await page.locator("#battle [data-settings]").click();
  await page.keyboard.press("Shift+Tab");
  await expect(page.locator("[data-close]").last()).toBeFocused();
  await page.keyboard.press("Tab");
  await expect(page.locator("[data-volume]").first()).toBeFocused();
  await page.locator("#muted").check();
  await page.keyboard.press("Escape");
  await page.click("#leave");
  await page.click("#confirm-leave");
  await expect(page.locator("#home")).toBeVisible();
  await page.reload();
  await expect(page.locator("#home")).toBeVisible();
});
test("stage transition retains lineup and cannot duplicate reward; final record once", async ({
  page,
}) => {
  await start(page);
  await fixture(page);
  await page.evaluate(() => {
    const q = (window as any).__gardenQA,
      b = q.battle();
    b.schedule = [];
    b.enemies = [];
    b.tick(1 / 60);
    q.flushEvents();
    q.renderHUD();
  });
  await expect(page.locator("#next")).toBeVisible();
  await page.click("#next");
  for (let i = 0; i < 3; i++)
    await page.locator("[data-draft]").first().click();
  await page.click("#start");
  expect(
    await page.evaluate(
      () => (window as any).__gardenQA.battle().plants.length,
    ),
  ).toBe(3);
  expect(
    await page.evaluate(() => (window as any).__gardenQA.run().stage),
  ).toBe(1);
  await page.evaluate(() => {
    const q = (window as any).__gardenQA,
      r = q.run(),
      b = q.battle();
    r.stage = 2;
    b.schedule = [];
    b.enemies = [];
    b.tick(1 / 60);
    q.flushEvents();
    q.flushEvents();
    q.renderHUD();
  });
  expect(
    await page.evaluate(() => (window as any).__gardenQA.save.records.wins),
  ).toBe(1);
  await page.click("#help");
  await page.keyboard.press("Escape");
  expect(
    await page.evaluate(() => (window as any).__gardenQA.save.records.wins),
  ).toBe(1);
});
test("blur requires active continue and pause/reset has no voices", async ({
  page,
}) => {
  await start(page);
  await page.evaluate(() => window.dispatchEvent(new Event("blur")));
  await expect(page.locator("#resume")).toBeVisible();
  const audio = await page.evaluate(() =>
    (window as any).__gardenQA.audio.debug(),
  );
  expect(audio.activeSfx).toBe(0);
  expect(audio.activeMusic).toBe(0);
  await page.click("#resume");
  await page.click("#restart");
  await page.click("#confirm-leave");
  const after = await page.evaluate(() =>
    (window as any).__gardenQA.audio.debug(),
  );
  expect(after.activeSfx).toBe(0);
  expect(after.activeMusic).toBe(0);
});
