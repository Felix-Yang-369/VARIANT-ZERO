import Phaser from "phaser";
import {
  PLANTS,
  ENEMIES,
  type Battle,
  type BattleEvent,
  type Kind,
} from "./model";
import { PHENOTYPES, GENE_IDS } from "./genetics";
import type { Interaction } from "./interaction";
import type { Preferences } from "./settings";
export const X = 116,
  Y = 87,
  CW = 99,
  CH = 93;
export interface SceneHooks {
  battle: () => Battle;
  interaction: Interaction;
  preferences: () => Preferences;
  frame: (delta: number, scene: Garden) => void;
  boardClick: (x: number, y: number) => void;
  cancel: () => void;
  ready: () => void;
  modal: () => boolean;
}
export class Garden extends Phaser.Scene {
  private ghost!: Phaser.GameObjects.Image;
  private label!: Phaser.GameObjects.Text;
  private reactions = new Map<string, { time: number; type: string }>();
  private deaths: { image: Phaser.GameObjects.Image; born: number }[] = [];
  private danger = new Map<number, number>();
  private shakeUntil = 0;
  clearBattle() {
    this.sprites.forEach((s) => s.destroy());
    this.sprites.clear();
    this.deaths.forEach((d) => d.image.destroy());
    this.deaths = [];
    this.reactions.clear();
    this.danger.clear();
    this.shakeUntil = 0;
    this.pointerCell = null;
  }
  consume(events: BattleEvent[]) {
    const b = this.hooks.battle();
    for (const e of events) {
      if (e.type === "shoot")
        this.reactions.set("p" + e.entityId, { time: e.time, type: "shoot" });
      if (e.type === "bite") {
        this.reactions.set("e" + e.entityId, { time: e.time, type: "bite" });
        this.reactions.set("p" + e.targetId, { time: e.time, type: "hit" });
      }
      if (e.type === "danger") this.danger.set(e.row!, e.time + 1.4);
      if (e.type === "boom") this.shakeUntil = e.time + 0.18;
      if ((e.type === "death" || e.type === "shovel") && e.texture) {
        this.deaths.push({
          image: this.add
            .image(X + (e.x! + 0.5) * CW, Y + (e.row! + 0.48) * CH, e.texture)
            .setDisplaySize(84, 84)
            .setDepth(15),
          born: b.time,
        });
      }
    }
  }
  private sprites = new Map<string, Phaser.GameObjects.Image>();
  private fx!: Phaser.GameObjects.Graphics;
  private hover!: Phaser.GameObjects.Graphics;
  private pointerCell: { row: number; col: number } | null = null;
  constructor(private hooks: SceneHooks) {
    super("garden");
  }
  private background!: Phaser.GameObjects.Image;
  preload() {
    for (const k of ["sun", "pea", "wall", ...Object.keys(PHENOTYPES)])
      this.load.image(k + "-source", "/art/" + k + ".png");
    for (let i = 0; i < 4; i++)
      this.load.image("enemy" + i + "-source", "/art/enemy" + i + ".png");
    for (let i = 0; i < 3; i++)
      this.load.image("field" + i, "/art/field" + i + ".png");
  }
  create() {
    // Downsample once for small board sprites: smooth alpha edges and avoid
    // keeping sixteen 1254px character textures resident on the GPU.
    for (const key of [
      "sun",
      "pea",
      "wall",
      ...Object.keys(PHENOTYPES),
      "enemy0",
      "enemy1",
      "enemy2",
      "enemy3",
    ]) {
      const texture = this.textures.createCanvas(key, 256, 256)!;
      texture.context.imageSmoothingEnabled = true;
      texture.context.imageSmoothingQuality = "high";
      texture.context.drawImage(
        this.textures.get(key + "-source").getSourceImage() as HTMLImageElement,
        0,
        0,
        256,
        256,
      );
      texture.refresh();
      this.textures.remove(key + "-source");
    }
    this.background = this.add
      .image(560, 310, "field0")
      .setDisplaySize(1120, 620)
      .setDepth(-2);
    const g = this.add.graphics().setDepth(-1);
    g.fillStyle(0x19382b, 0.32);
    g.fillRoundedRect(X - 12, Y - 12, CW * 9 + 24, CH * 5 + 24, 10);
    for (let r = 0; r < 5; r++)
      for (let c = 0; c < 9; c++) {
        g.fillStyle((r + c) % 2 === 0 ? 0x81a869 : 0xabc286, 0.44);
        g.fillRect(X + c * CW, Y + r * CH, CW, CH);
        g.lineStyle(1, 0xdce8a6, 0.2);
        g.strokeRect(X + c * CW, Y + r * CH, CW, CH);
      }
    this.fx = this.add.graphics().setDepth(20);
    this.hover = this.add.graphics().setDepth(21);
    this.ghost = this.add
      .image(0, 0, "pea")
      .setDisplaySize(86, 86)
      .setAlpha(0.6)
      .setDepth(22)
      .setVisible(false);
    this.label = this.add
      .text(0, 0, "", {
        fontFamily: "sans-serif",
        fontSize: "16px",
        color: "#fff9dd",
        backgroundColor: "#384b2f",
        padding: { x: 9, y: 5 },
      })
      .setDepth(23)
      .setVisible(false);
    this.hooks.ready();
    this.input.mouse?.disableContextMenu();
    this.input.on("pointermove", (p: Phaser.Input.Pointer) => {
      const col = Math.floor((p.x - X) / CW),
        row = Math.floor((p.y - Y) / CH);
      this.pointerCell =
        col >= 0 && col < 9 && row >= 0 && row < 5 ? { row, col } : null;
    });
    this.input.on("pointerout", () => (this.pointerCell = null));
    this.input.on("pointerdown", (p: Phaser.Input.Pointer) => {
      if (p.rightButtonDown()) {
        this.hooks.cancel();
        return;
      }
      this.clickBoard(p.x, p.y);
    });
  }
  clickBoard(x: number, y: number) {
    this.hooks.boardClick(x, y);
  }
  update(_t: number, delta: number) {
    this.hooks.frame(delta, this);
    this.draw();
  }
  draw() {
    const battle = this.hooks.battle(),
      state = this.hooks.interaction.state,
      reduced = this.hooks.preferences().reducedMotion;
    if (this.background.texture.key !== "field" + battle.level)
      this.background.setTexture("field" + battle.level);
    const inspected =
        state.type === "browse"
          ? state.inspected
          : state.type === "gene" || state.type === "experiment"
            ? state.target
            : null,
      material = state.type === "fusion" ? state.first : null,
      second = state.type === "fusion" ? state.second : null;
    const selected = state.type === "plant" ? state.kind : null,
      mode = state.type;
    const shake =
      !reduced && battle.time < this.shakeUntil
        ? Math.sin(battle.time * 150) * 2
        : 0;
    this.cameras.main.setScroll(shake, shake * 0.5);
    for (const [key, reaction] of this.reactions)
      if (battle.time - reaction.time > 0.4) this.reactions.delete(key);
    this.deaths = this.deaths.filter((d) => {
      const p = (battle.time - d.born) / (reduced ? 0.18 : 0.45);
      if (p >= 1) {
        d.image.destroy();
        return false;
      }
      d.image.setAlpha(1 - p);
      if (!reduced)
        d.image
          .setAngle(p * 24)
          .setDisplaySize(84 * (1 - p * 0.35), 84 * (1 - p * 0.35));
      return true;
    });
    const alive = new Set<string>();
    const sprite = (
      id: string,
      key: string,
      x: number,
      y: number,
      w: number,
      h: number,
      depth: number,
    ) => {
      alive.add(id);
      let obj = this.sprites.get(id);
      if (!obj) {
        obj = this.add.image(x, y, key);
        this.sprites.set(id, obj);
      }
      if (obj.texture.key !== key) obj.setTexture(key);
      obj.setPosition(x, y).setDisplaySize(w, h).setDepth(depth);
      return obj;
    };
    for (const p of battle.plants) {
      const sway = reduced ? 0 : Math.sin(battle.time * 2 + p.id) * 2;
      const reaction = this.reactions.get("p" + p.id),
        age = reaction ? battle.time - reaction.time : 99;
      const recoil =
        !reduced && reaction?.type === "shoot" && age < 0.2
          ? -Math.sin((age / 0.2) * Math.PI) * 8
          : 0;
      const plantSprite = sprite(
        "p" + p.id,
        p.phenotype ?? p.kind,
        X + (p.col + 0.5) * CW + recoil,
        Y + (p.row + 0.48) * CH + sway,
        p.mutation === "giant" ? 100 : 86,
        p.mutation === "giant" ? 100 : 86,
        3 + p.row * 2,
      );
      plantSprite.setTint(
        reaction?.type === "hit" && age < 0.14
          ? 0xffb9a0
          : p.mutation === "golden"
            ? 0xffe796
            : p.mutation
              ? 0xead3ff
              : 0xffffff,
      );
    }
    for (const e of battle.enemies) {
      const reaction = this.reactions.get("e" + e.id),
        age = reaction ? battle.time - reaction.time : 99;
      const bite =
        !reduced && age < 0.3 ? Math.sin((age / 0.3) * Math.PI) * 8 : 0;
      const bob = reduced
        ? 0
        : Math.sin(battle.time * (e.kind === 2 ? 12 : 6) + e.id) * 2;
      const o = sprite(
        "e" + e.id,
        "enemy" + e.kind,
        X + (e.x + 0.5) * CW - bite,
        Y + (e.row + 0.43) * CH + bob,
        e.kind === 3 ? 99 : 83,
        e.kind === 3 ? 105 : 91,
        4 + e.row * 2,
      );
      o.setTint(
        e.hit > battle.time
          ? 0xffd6b0
          : e.slowUntil > battle.time
            ? 0xb4eaff
            : 0xffffff,
      );
    }
    for (const [id, o] of this.sprites)
      if (!alive.has(id)) {
        o.destroy();
        this.sprites.delete(id);
      }
    const g = this.fx;
    g.clear();
    for (const p of battle.plants) {
      const px = X + (p.col + 0.5) * CW,
        py = Y + (p.row + 0.84) * CH;
      if (p.genome?.length) {
        for (let i = 0; i < p.genome.length; i++) {
          const gene = p.genome[i];
          const color = [0xa7d471, 0x72cfc2, 0xe9cd72, 0xc096e4][
            GENE_IDS.indexOf(gene) % 4
          ];
          g.fillStyle(color, 0.95);
          g.fillEllipse(px - 16 + i * 11, py, 9, 5);
          g.lineStyle(1, 0xebf4bd, 0.6);
          g.lineBetween(px - 16 + i * 11, py, px - 13 + i * 11, py - 7);
        }
      }
      if (p.mutation) {
        g.lineStyle(2, p.mutation === "golden" ? 0xf5d477 : 0xba86db, 0.65);
        g.strokeEllipse(px, py + 2, 58, 15);
      }
      if (p.evolution) {
        g.fillStyle(0xecebb1);
        g.fillTriangle(px + 25, py - 55, px + 29, py - 61, px + 33, py - 55);
      }
      if (p.hp < battle.getStats(p).hp || p.id === inspected) {
        const x = X + (p.col + 0.5) * CW,
          y = Y + p.row * CH + 9;
        g.fillStyle(0x3d583c, 0.3);
        g.fillRoundedRect(x - 23, y, 46, 4, 2);
        g.fillStyle(0xf6e393);
        g.fillRoundedRect(
          x - 23,
          y,
          46 * Math.max(0, p.hp / battle.getStats(p).hp),
          4,
          2,
        );
      }
    }
    for (const e of battle.enemies)
      if (e.hp < ENEMIES[e.kind].hp) {
        const x = X + (e.x + 0.5) * CW,
          y = Y + e.row * CH + 4;
        g.fillStyle(0x496445, 0.4);
        g.fillRect(x - 20, y, 40, 3);
        g.fillStyle(0xecc087);
        g.fillRect(x - 20, y, (40 * e.hp) / ENEMIES[e.kind].hp, 3);
      }
    for (const b of battle.bullets) {
      const x = X + (b.x + 0.5) * CW,
        y = Y + (b.row + 0.4) * CH;
      g.fillStyle(b.slow ? 0xd9f8ff : 0xe7f1a2, 0.25);
      g.fillEllipse(x - 7, y, 26, 13);
      g.fillStyle(b.slow ? 0xc0f1ff : 0xd5eb7f);
      g.fillCircle(x, y, 7);
      g.fillStyle(0xffffff, 0.6);
      g.fillCircle(x - 2, y - 2, 2);
    }
    battle.cars.forEach((c, r) => {
      if (c.state === "spent") return;
      const x = X + (c.x + 0.5) * CW,
        y = Y + (r + 0.6) * CH;
      if (c.state === "active" && !reduced) {
        for (let i = 1; i < 6; i++) {
          g.fillStyle(0xf1deac, 0.28 - i * 0.035);
          g.fillCircle(x - 24 - i * 12, y + 8, 4 + i * 1.5);
        }
      }
      g.fillStyle(0x3f5946);
      g.fillCircle(x - 17, y + 10, 8);
      g.fillCircle(x + 17, y + 10, 8);
      g.fillStyle(0xbe6550);
      g.fillRoundedRect(x - 25, y - 10, 49, 23, 6);
      g.fillStyle(0xe2a373);
      g.fillRoundedRect(x - 13, y - 17, 25, 14, 4);
      g.lineStyle(4, 0x526443);
      g.lineBetween(x - 20, y - 10, x - 30, y - 29);
      g.lineBetween(x - 30, y - 29, x - 16, y - 29);
    });
    for (const s of battle.suns) {
      const x = X + (s.x + 0.5) * CW,
        y =
          Y +
          (s.row + 0.3) * CH +
          (reduced ? 0 : Math.sin(battle.time * 3 + s.id) * 3);
      g.fillStyle(0xffe798, 0.2);
      g.fillCircle(x, y, 29);
      g.lineStyle(3, 0xffe593);
      for (let a = 0; a < 8; a++) {
        const t = (a * Math.PI) / 4 + battle.time * 0.2;
        g.lineBetween(
          x + Math.cos(t) * 20,
          y + Math.sin(t) * 20,
          x + Math.cos(t) * 24,
          y + Math.sin(t) * 24,
        );
      }
      g.fillStyle(0xfad363);
      g.fillCircle(x, y, 16);
      g.lineStyle(2, 0xffedaa);
      g.strokeCircle(x, y, 13);
      g.fillStyle(0xfff2b2);
      g.fillCircle(x - 4, y - 5, 4);
    }
    for (const e of battle.effects) {
      const alpha = Math.max(
        0,
        (e.until - battle.time) / (e.kind === "boom" ? 0.65 : 0.35),
      );
      const x = X + (e.x + 0.5) * CW,
        y = Y + (e.row + 0.5) * CH;
      if (e.kind === "boom") {
        if (!reduced)
          for (let i = 0; i < 16; i++) {
            const a = (i * Math.PI) / 8,
              r = (1 - alpha) * 130;
            g.fillStyle(i % 2 ? 0xf5af43 : 0xfff1a3, alpha);
            g.fillCircle(
              x + Math.cos(a) * r,
              y + Math.sin(a) * r,
              3 + alpha * 5,
            );
          }
        g.fillStyle(0xffdd78, alpha * 0.7);
        g.fillCircle(x, y, (1 - alpha) * 140 + 20);
        g.lineStyle(5, 0xfff4c1, alpha);
        g.strokeCircle(x, y, (1 - alpha) * 130 + 25);
      } else {
        g.lineStyle(
          e.kind === "hit" ? 2 : 4,
          e.kind === "hit" ? 0xf9f3ba : 0xffe49a,
          alpha,
        );
        g.strokeCircle(x, y, (1 - alpha) * 35 + 7);
      }
    }
    this.hover.clear();
    for (const [row, until] of this.danger) {
      if (battle.time > until) {
        this.danger.delete(row);
        continue;
      }
      this.hover.fillStyle(
        0xef8754,
        reduced ? 0.12 : 0.12 + 0.08 * Math.sin(battle.time * 12),
      );
      this.hover.fillRect(X, Y + row * CH, CW * 9, CH);
    }
    this.ghost.setVisible(false);
    this.label.setVisible(false);
    const outline = (row: number, col: number, color: number) => {
      this.hover.lineStyle(3, color, 0.85);
      this.hover.strokeRoundedRect(
        X + col * CW + 3,
        Y + row * CH + 3,
        CW - 6,
        CH - 6,
        9,
      );
    };
    for (const id of [material, second, inspected]) {
      const p = battle.plants.find((p) => p.id === id);
      if (p) outline(p.row, p.col, 0xffe191);
    }
    if (
      state.type === "fusion" &&
      state.first !== null &&
      state.second === null
    ) {
      for (const p of battle.plants)
        if (battle.recipe(state.first, p.id)) outline(p.row, p.col, 0xbde77c);
    }
    if (
      this.pointerCell &&
      battle.status === "running" &&
      !this.hooks.modal()
    ) {
      const { row, col } = this.pointerCell;
      const occupied = battle.plants.some(
        (p) => p.row === row && p.col === col,
      );
      const error = selected ? battle.placementError(selected, row, col) : "";
      if (selected) {
        this.ghost
          .setTexture(selected)
          .setPosition(X + (col + 0.5) * CW, Y + (row + 0.48) * CH)
          .setTint(error ? 0xef9a8a : 0xffffff)
          .setVisible(true);
        this.label
          .setText(error || "点击种植")
          .setPosition(
            Math.min(890, X + col * CW),
            Math.max(0, Y + row * CH - 28),
          )
          .setVisible(true);
      }
      if (selected || mode !== "browse") {
        this.hover.fillStyle(selected && !!error ? 0xe98c73 : 0xfff4bc, 0.18);
        this.hover.fillRoundedRect(
          X + col * CW + 2,
          Y + row * CH + 2,
          CW - 4,
          CH - 4,
          8,
        );
        outline(row, col, selected && !!error ? 0xe9a18a : 0xf8ebae);
      }
    }
  }
}
