import { BASE, Battle, type Kind } from "./model";
export type InteractionState =
  | { type: "browse"; inspected: number | null }
  | { type: "plant"; kind: Kind }
  | { type: "gene"; sample: number; target: number | null }
  | {
      type: "experiment";
      target: number;
      action: "evolve" | "mutate" | "inspect";
    }
  | { type: "shovel" }
  | { type: "fusion"; first: number | null; second: number | null };
export class Interaction {
  state: InteractionState = { type: "browse", inspected: null };
  cancel() {
    this.state = { type: "browse", inspected: null };
  }
  choose(kind: Kind) {
    this.state =
      this.state.type === "plant" && this.state.kind === kind
        ? { type: "browse", inspected: null }
        : { type: "plant", kind };
  }
  tool(type: "shovel" | "fusion") {
    if (this.state.type === type) this.cancel();
    else
      this.state =
        type === "shovel" ? { type } : { type, first: null, second: null };
  }
  get canCollect() {
    return this.state.type === "browse";
  }
  click(b: Battle, row: number, col: number): string {
    if (b.status !== "running") return "请先继续游戏";
    const p = b.plants.find((p) => p.row === row && p.col === col);
    const s = this.state;
    if (s.type === "plant") {
      const error = b.place(s.kind, row, col);
      if (!error) this.cancel();
      return error;
    }
    if (s.type === "shovel") {
      if (!p) return "这里没有植物";
      b.remove(p.id);
      return "";
    }
    if (s.type === "gene") {
      if (!p) return "请选择一株植物";
      this.state = { ...s, target: p.id };
      return "";
    }
    if (s.type === "experiment") {
      if (p) this.state = { ...s, target: p.id };
      return "";
    }
    if (s.type === "browse") {
      this.state = { type: "browse", inspected: p?.id ?? null };
      return "";
    }
    if (!p || (!b.genetic && !BASE.includes(p.kind)))
      return "请选择配方中的基础植物";
    if (s.first === null || s.second !== null) {
      this.state = { type: "fusion", first: p.id, second: null };
      return "";
    }
    if (s.first === p.id) return "需要两株不同的植物";
    if (!b.recipe(s.first, p.id)) return "这两株植物不符合配方";
    this.state = { ...s, second: p.id };
    return "";
  }
  reconcile(b: Battle) {
    const s = this.state;
    if (
      s.type === "browse" &&
      s.inspected &&
      !b.plants.some((p) => p.id === s.inspected)
    )
      this.cancel();
    if (
      (s.type === "gene" || s.type === "experiment") &&
      s.target &&
      !b.plants.some((p) => p.id === s.target && p.hp > 0)
    ) {
      if (s.type === "gene") this.state = { ...s, target: null };
      else this.cancel();
    }
    if (s.type === "fusion") {
      if (s.first && !b.plants.some((p) => p.id === s.first && p.hp > 0)) {
        this.state = { type: "fusion", first: null, second: null };
      } else if (
        s.second &&
        (!b.plants.some((p) => p.id === s.second && p.hp > 0) ||
          !b.recipe(s.first!, s.second))
      ) {
        this.state = { ...s, second: null };
      }
    }
  }
  confirm(b: Battle) {
    const s = this.state;
    if (s.type !== "fusion" || s.first === null || s.second === null)
      return "请先选择两株材料";
    const error = b.fuse(s.first, s.second);
    if (!error) this.cancel();
    return error;
  }
}
