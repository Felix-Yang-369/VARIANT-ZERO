import { Battle } from "./model";
import { GENE_IDS, streams, type Gene } from "./genetics";
import type { SaveData } from "./settings";
export class RunState {
  readonly rng;
  battle: Battle;
  stage = 0;
  samples: Gene[] = [];
  phase: "draft" | "battle" | "won" | "lost" | "ended" = "draft";
  draftRemaining = 3;
  choices: Gene[] = [];
  elapsed = 0;
  kills = 0;
  readonly pool: Gene[];
  discoveries: string[] = [];
  private settled = false;
  constructor(
    public seed: number,
    unlocked: Gene[] = [],
  ) {
    this.rng = streams(seed);
    this.pool = GENE_IDS.filter((g, i) => i < 8 || unlocked.includes(g));
    this.battle = new Battle(0, true, this.rng);
    this.offer();
  }
  private offer() {
    this.choices = this.rng.draft.sample(this.pool, 3);
  }
  pick(gene: Gene) {
    if (this.phase !== "draft" || !this.choices.includes(gene)) return false;
    this.samples.push(gene);
    this.draftRemaining--;
    this.battle.emit("draft", { detail: gene });
    if (this.draftRemaining) this.offer();
    else {
      this.choices = [];
      this.phase = "battle";
    }
    return true;
  }
  install(index: number, id: number) {
    const g = this.samples[index];
    if (!g) return "样本已失效";
    const error = this.battle.installGene(id, g);
    if (!error) this.samples.splice(index, 1);
    return error;
  }
  settle(save: SaveData) {
    if (
      this.settled ||
      this.phase !== "battle" ||
      !["won", "lost"].includes(this.battle.status)
    )
      return false;
    this.settled = true;
    this.elapsed += this.battle.time;
    this.kills += this.battle.kills;
    if (this.battle.status === "lost") this.phase = "lost";
    else if (this.stage === 2) {
      this.phase = "won";
      unlock(save, "symbiosis");
    } else return true;
    save.records.runs++;
    if (this.phase === "won") save.records.wins++;
    save.records.bestStage = Math.max(
      save.records.bestStage,
      this.stage + (this.phase === "won" ? 1 : 0),
    );
    save.records.bestKills = Math.max(save.records.bestKills, this.kills);
    if (this.phase === "won")
      save.records.bestTime = Math.min(
        save.records.bestTime || Infinity,
        this.elapsed,
      );
    return true;
  }
  next() {
    if (!this.settled || this.battle.status !== "won" || this.stage >= 2)
      return false;
    const prior = this.battle;
    this.stage++;
    this.battle = new Battle(this.stage, true, this.rng);
    this.battle.inherit(prior);
    this.settled = false;
    this.phase = "draft";
    this.draftRemaining = 3;
    this.offer();
    return true;
  }
  end() {
    this.phase = "ended";
  }
}
export function unlock(save: SaveData, g: Gene) {
  if (!save.genes.includes(g)) save.genes.push(g);
}
