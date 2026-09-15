import { WORLD, SPECIMENS, WITHER } from "./lore";
import Phaser from "phaser";
import "./style.css";
import "./visual-system.css";
import { icon } from "./ui-icons";
import {
  Battle,
  PLANTS,
  LEVELS,
  type Kind,
  type BattleEvent,
  type Plant,
} from "./model";
import {
  GENES,
  GENE_IDS,
  FAMILIES,
  PHENOTYPES,
  EVOLUTIONS,
  MUTATIONS,
  mutationPool,
  applicable,
  stats,
  type Gene,
  type Evolution,
} from "./genetics";
import { RunState, unlock } from "./run";
import { Interaction } from "./interaction";
import { AudioManager } from "./audio";
import { loadSave, saveData } from "./settings";
import { Garden, X, Y, CW, CH } from "./scene";
const $ = <T extends HTMLElement = HTMLElement>(s: string) =>
  document.querySelector<T>(s)!;
const loaded = (() => {
  try {
    return loadSave(
      localStorage,
      matchMedia("(prefers-reduced-motion: reduce)").matches,
    );
  } catch {
    return loadSave({ getItem: () => null, setItem: () => {} }, false);
  }
})();
const save = loaded.data,
  interaction = new Interaction(),
  audio = new AudioManager(save.settings);
let run: RunState | null = null,
  battle = new Battle(0, true),
  game: Phaser.Game | undefined,
  scene: Garden | undefined,
  ready = false,
  acc = 0,
  page = "home",
  lastHUD = "",
  lastOverlay = "",
  bannerUntil = 0;
let modal: null | { resume: boolean; focus: HTMLElement | null } = null,
  toastTimer = 0;
const image = (key: string, cls = "") =>
  `<img class="${cls}" src="/art/${key}.png" alt="" loading="lazy">`;
const plantName = (p: Plant) =>
  p.phenotype ? PHENOTYPES[p.phenotype].name : PLANTS[p.kind].name;
const fmt = (n: number) =>
  `${Math.floor(n / 60)
    .toString()
    .padStart(2, "0")}:${Math.floor(n % 60)
    .toString()
    .padStart(2, "0")}`;
const txt = (s: string, v: string) => {
  const e = $(s);
  if (e && e.textContent !== v) e.textContent = v;
};
function persist() {
  try {
    if (saveData(localStorage, save)) return;
  } catch {}
  toast("本机存储不可用，本局仍可继续");
}
function toast(s: string) {
  $(".toast").textContent = s;
  $(".toast").classList.add("show");
  clearTimeout(toastTimer);
  toastTimer = window.setTimeout(
    () => $(".toast").classList.remove("show"),
    3000,
  );
}
function lesson(id: string) {
  if (!save.lessons.includes(id)) {
    save.lessons.push(id);
    persist();
  }
}
$("#app").innerHTML =
  `<main id="home" class="home"><div class="home-top"><span class="edition">BOTANICAL GENETICS / v0.3</span><div class="home-progress"></div></div><section class="home-copy"><div class="brand-mark"><img src="/favicon.svg?v=seedcore-2" alt="异芽种核徽记"></div><h1>异芽</h1><div class="wordmark">MUTABLOOM</div><div class="subtitle">— 杂 交 花 园 —</div><p class="tagline">培育不可能。</p><nav class="home-menu" aria-label="主菜单"><button id="expedition" class="primary">${icon("leaf")}开始出征 <b>${icon("arrow")}</b></button><button data-page="base">${icon("home")}<span>花园基地</span><small>发现与成长</small></button><button data-page="pedia">${icon("book")}<span>植物图鉴</span><small>遗传档案</small></button><button data-page="lab">${icon("flask")}<span>基因实验室</span><small>公开概率</small></button><button data-settings>${icon("settings")}<span>设置</span></button></nav></section><figure class="home-guardian" aria-label="代表杂交植物：辉光蓄能者"><div class="guardian-aura" aria-hidden="true"></div><div class="guardian-shadow" aria-hidden="true"></div><img class="guardian-plant" src="/art/home-lumenheart.png" width="1254" height="1254" alt="金绿躯体、琥珀种核与青色能量纹路的辉光蓄能者" fetchpriority="high" draggable="false"><figcaption><span>向阳 × 种荚 · 杂交标本</span><b>辉光蓄能者</b><small>LUMENHEART</small></figcaption></figure><section class="home-features"><button data-page="lab"><i>${icon("flask")}</i><b>杂交</b><span>让血统交汇</span></button><button data-page="pedia"><i>${icon("dna")}</i><b>进化</b><span>选择生长方向</span></button><button data-page="lab"><i>${icon("spark")}</i><b>突变</b><span>主动拥抱未知</span></button></section><button class="home-note" data-page="world"><b>最后温室，仍在发光。 <span aria-hidden="true">${icon("arrow")}</span></b><span>阅读花园纪事 · 翠绿网络与枯潮</span></button></main>
<main id="archive" class="hidden"><header><button data-home>${icon("back")} 温室</button><span>MUTABLOOM / FIELD NOTES</span><button data-settings>${icon("settings")} 设置</button></header><section id="archive-content"></section></main>
<main id="battle" class="hidden"><header class="battle-header"><button id="leave">← 温室</button><strong>异芽 <small>MUTABLOOM</small></strong><span id="stage-name"></span><div class="controls"><button id="help">? 手册</button><button data-settings>${icon("settings")} 设置</button><button id="sound">♫</button><button id="restart">↻ 重开</button><button id="pause">Ⅱ 暂停</button></div></header><div class="workspace"><section class="game-panel"><div class="seedbank"><div class="sun-meter"><span>☀</span><b id="sun">175</b><small>阳光储备</small><i id="sun-gain"></i></div><div class="cards">${FAMILIES.map((k, i) => `<button class="seed" data-kind="${k}"><kbd>${i + 1}</kbd>${image(k)}<div><b>${PLANTS[k].name}</b><small>☀ ${PLANTS[k].cost}</small><span class="card-status"></span></div><i class="cooldown"></i></button>`).join("")}</div><div class="run-progress"><small>连续出征</small><b id="stage-number"></b><span id="timer">00:00</span></div></div><div class="board-wrap"><div id="game" aria-label="五行九列植物防线"></div><div class="wave-banner" role="status"></div><div id="overlay" class="overlay hidden"></div></div><div class="board-footer"><span id="hint"></span><span id="wave-label"></span></div><div class="samplebar"><span>基因样本</span><div id="samples"></div></div></section><aside class="sidebar"><div class="tools"><button id="inspect">◎ 查看</button><button id="fusion">⚗ 杂交</button><button id="shovel">♧ 铲除</button><button id="cancel">Esc 取消</button></div><section id="inspector"></section><section id="tutorial"></section></aside></div></main><div id="modal-layer" class="modal-layer hidden"></div><div class="toast" role="status"></div>`;
function showPage(name: string) {
  page = name;
  $("#archive").dataset.page = name;
  $("#archive").classList.toggle("is-pedia", name === "pedia");
  $("#home").classList.toggle("hidden", name !== "home");
  $("#archive").classList.toggle(
    "hidden",
    !["base", "pedia", "lab", "world"].includes(name),
  );
  $("#battle").classList.toggle("hidden", name !== "battle");
  if (name === "home")
    $(".home-progress").textContent =
      `${save.discoveries.length} 项发现 · ${save.records.wins} 次完整出征`;
  if (name !== "home" && name !== "battle") renderArchive(name);
  fit();
}
function geneCard(g: Gene) {
  return `<article class="gene-card"><span class="gene-symbol">${icon("dna")}</span><small>GENE ${String(GENE_IDS.indexOf(g) + 1).padStart(2, "0")}</small><h3>${GENES[g].name}</h3><p>${GENES[g].desc}</p><em>${{ any: "所有植物", active: "射击 / 生产植物", attack: "射击植物", produce: "生产植物" }[GENES[g].needs]}</em></article>`;
}
function renderArchive(name: string) {
  let html = "";
  const discovered = Object.keys(PHENOTYPES).filter(k => save.discoveries.includes("plant:" + k)).length;
  if (name === "base")
    html = `<span class="eyebrow">YOUR GARDEN</span><h1>每一株，都留下痕迹。</h1><p>成长来自新的可能。解锁扩展抽取池，不增加永久属性。</p><section class="discovery-hero"><div><span class="eyebrow">LIVING ARCHIVE / 后代发现</span><h2>${discovered ? "新的血统，正在温室扎根。" : "第一份发现，从两株幼苗开始。"}</h2><p>${discovered ? "每次首次杂交发现都会即时收录，结束出征后依然保留。" : "在出征中将不同主系的植物杂交，点亮你的第一页后代档案。"}</p><div class="discovery-track" role="progressbar" aria-label="杂交后代发现" aria-valuemin="0" aria-valuemax="9" aria-valuenow="${discovered}"><i style="width:${discovered / 9 * 100}%"></i></div><span class="discovery-count">${discovered} / 9 种杂交后代 · 三类母体已收录</span></div>${image("nurse", "discovery-plant")}</section><div class="record-grid">${[
      ["发现记录", save.discoveries.length],
      ["完整出征", save.records.wins],
      ["最高击退", save.records.bestKills],
      ["最快胜利", save.records.bestTime ? fmt(save.records.bestTime) : "等待首次胜利"],
      ["结算出征", save.records.runs],
      ["旧版纪念", `已解锁第 ${save.unlocked + 1} 关`],
    ]
      .map(([k, v]) => `<article><b>${v}</b><span>${k}</span></article>`)
      .join(
        "",
      )}</div><h2>解锁里程碑</h2><div class="archive-grid milestones">${(["reserve", "thorns", "shell", "symbiosis"] as Gene[]).map((g, i) => `<article class="milestone ${save.genes.includes(g) ? "complete" : ""}"><small>${save.genes.includes(g) ? "已解锁，下次出征可抽取" : "尚未解锁"}</small><h3>${GENES[g].name}</h3><p>${["完成第一次杂交", "完成第一次进化", "完成第一次突变", "完成三场连续出征"][i]}</p></article>`).join("")}</div>`;
  if (name === "world")
    html = `<section class="world-hero"><span class="eyebrow">THE LAST GREENHOUSE / 花园纪事</span><h1>世界沉寂之后，<br>还有一枚种子醒着。</h1><p>${WORLD.premise}</p><p>${WORLD.mission}</p><span class="world-seal">MUTABLOOM · 温室档案 001</span></section><div class="world-principles"><article><span>01 / 翠绿网络</span><h2>根系记得彼此。</h2><p>它不是一台机器，而是由根、菌丝与光连接的生命网络。网络曾让不同植物交换生长的记忆；当回声消失，许多植物也失去了自然进化的方向。</p></article><article><span>02 / 枯潮</span><h2>失去回应的生命。</h2><p>枯潮不是普通的枯萎。它让生长继续，却夺走生长的秩序。苔木、藤蔓和古树节点开始侵袭温室；它们是敌人，也是这场生态异常的见证。</p></article><article><span>03 / 最后温室</span><h2>你的工作，是让可能性生根。</h2><p>温室保存着花系、荚系、根系三类母体。杂交连接血统，进化选择方向，突变承担风险。每一次发现，都是续写遗传笔记的一页。</p></article></div><h2>一场出征，三段根脉</h2><div class="world-route">${WORLD.stages.map((stage, i) => `<article><img src="/art/field${i}.png" alt="${stage.title.slice(5)}环境" loading="lazy"><div><h3>${stage.title}</h3><p>${stage.text}</p></div></article>`).join("")}</div><h2>枯潮观察记录</h2><p>我们守护花园，也试着理解那些迷失在光外的生命。</p><div class="archive-grid wither-cards">${WITHER.map((e, i) => `<article><img src="/art/enemy${i}.png" alt="${e.name}" loading="lazy"><small>WITHER / 0${i + 1}</small><h3>${e.name}</h3><b>${e.tag}</b><p>${e.note}</p></article>`).join("")}</div><blockquote class="world-quote">“我们还不知道枯潮从何而来。但只要有一株幼苗愿意向光生长，这本笔记就不该合上。”<cite>—— 前任园丁留下的最后一页</cite></blockquote>`;
  if (name === "pedia")
    html = `<span class="eyebrow">SEED VAULT / 温室种子库</span><h1>不可能的植物图鉴</h1><p>收好每一份生长的证据。三类母体，九种后代，十二页等待续写的标本记录。</p><div class="archive-legend"><span>✦ 已收录的母体与发现</span><span>◇ 尚未培育的后代为档案预览</span><span>卡片内可展开培育记录</span></div><div class="archive-grid plants specimen-grid">${[
      ...FAMILIES,
      ...Object.keys(PHENOTYPES),
    ]
      .map((k, i) => {
        const lore = SPECIMENS[k],
          d = PHENOTYPES[k] ?? PLANTS[k as Kind],
          known =
            FAMILIES.includes(k as any) ||
            save.discoveries.includes("plant:" + k);
        return `<article class="specimen-card ${known ? "catalogued" : "undiscovered"}" style="--specimen-accent:${lore.accent}"><div class="specimen-rim" aria-hidden="true"></div><div class="specimen-top"><span>MB / ${String(i + 1).padStart(3, "0")}</span><span class="specimen-status">${known ? "✦ 已收录" : "◇ 待培育"}</span></div><div class="specimen-art">${image(k)}<span class="specimen-orbit" aria-hidden="true"></span></div><div class="specimen-body"><span class="specimen-family">${lore.family}</span><h3>${d.name}</h3><span class="specimen-name">${lore.name}</span><div class="specimen-traits"><span>${lore.role}</span><span>${i < 3 ? "母体" : "杂交后代"}</span></div><div class="specimen-stats"><div><small>耐久</small><b>${d.hp}</b></div><div><small>${d.damage ? "伤害" : "产能"}</small><b>${d.damage ?? (d.produce ? "25 ☀" : "—")}</b></div><div><small>${d.rate ? "射击间隔" : "生产间隔"}</small><b>${d.rate ?? d.produce ?? "—"}${d.rate || d.produce ? "<em>s</em>" : ""}</b></div></div><details class="specimen-notes"><summary>标本记录 <span aria-hidden="true">＋</span></summary><p>${lore.note}</p><small>记录地点 · ${lore.habitat}</small></details></div><div class="specimen-bottom"><span>最后温室 · 遗传档案</span><span aria-hidden="true">▥</span></div></article>`;
      })
      .join(
        "",
      )}</div><h2>四槽基因档案</h2><div class="archive-grid">${GENE_IDS.map((g) => geneCard(g)).join("")}</div>`;
  if (name === "lab")
    html = `<span class="eyebrow">GENETICS LABORATORY</span><h1>每一次实验，风险可见。</h1><p>两株不同主系，100 阳光。后代留在第二株的位置，并继承它的主系。实验仅能在出征中进行。</p><div class="archive-grid recipes">${[
      ["sun", "pea"],
      ["pea", "wall"],
      ["sun", "wall"],
    ]
      .map(
        (pair) =>
          `<article><div class="recipe-parents">${image(pair[0])}<span>×</span>${image(pair[1])}</div><h2>${pair.map((k) => PLANTS[k as Kind].name).join(" × ")}</h2><span class="recipe-cost">100 阳光 · 后代落在第二株</span>${Object.entries(
            PHENOTYPES,
          )
            .filter(([, p]) => pair.every((k) => p.pair.includes(k as any)))
            .map(
              ([k, p], i) =>
                `<div class="recipe-line">${image(k)}<span>${p.name}</span><b>${[60, 30, 10][i]}%</b><span class="probability-track" aria-hidden="true"><i style="width:${[60, 30, 10][i]}%"></i></span></div>`,
            )
            .join("")}</article>`,
      )
      .join(
        "",
      )}</div><div class="research-notes"><article><h2>四槽显式遗传</h2><p>父母基因去重后，等概率无放回继承最多四个。主系决定配对，世代只记录血统，不提升属性。后代可继续杂交；经验、进化和突变不继承。继承后暂不适用的基因保留在槽中，并标为休眠。</p></article><article class="evolution-notes"><h2>三级进化 · 100 ☀</h2><p>每存活 30 秒升一级，三级开放一次进化。四个方向随机展示三个，关闭预览不刷新选项。</p>${Object.values(
      EVOLUTIONS,
    )
      .map((v) => `<p><b>${v.name}</b> · ${v.desc}</p>`)
      .join(
        "",
      )}</article></div><h2>主动突变 · 125 ☀</h2><p>每株只能实验一次。仅从适用结果中等概率抽取：射击植物六种各 16.67%，生产植物五种各 20%，纯防守植物四种各 25%。</p><div class="archive-grid">${Object.values(
      MUTATIONS,
    )
      .map(
        (v) =>
          `<article class="mutation-card"><h3>${v.name}</h3><p>${v.desc}</p></article>`,
      )
      .join("")}</div>`;
  if (name === "base")
    html += `<section class="lore-link"><span class="eyebrow">THE LAST GREENHOUSE</span><h2>温室的灯，为每一株幼苗而亮。</h2><p>${WORLD.mission}</p><button data-world>阅读花园纪事 ${icon("arrow")}</button></section>`;
  if (name === "lab")
    html =
      `<div class="lab-lore"><span>前任园丁的实验笔记</span><p>“杂交，是让两段生长的记忆相遇；进化，是选择下一步；突变，则是明知未知仍然打开培养皿。”</p></div>` +
      html;
  $("#archive-content").innerHTML = html;
  document
    .querySelectorAll<HTMLElement>("[data-world]")
    .forEach((e) => (e.onclick = () => showPage("world")));
}
function startRun(seed = crypto.getRandomValues(new Uint32Array(1))[0]) {
  audio.reset();
  scene?.clearBattle();
  run = new RunState(seed, save.genes);
  battle = run.battle;
  interaction.cancel();
  acc = 0;
  lastHUD = lastOverlay = "";
  bannerUntil = 0;
  document.querySelectorAll(".sun-flight").forEach((e) => e.remove());
  showPage("battle");
  ensureGame();
  renderHUD();
  void audio.unlock();
}
function ensureGame() {
  if (game) return;
  game = new Phaser.Game({
    type: Phaser.AUTO,
    width: 1120,
    height: 620,
    parent: "game",
    backgroundColor: "#273d30",
    scene: new Garden({
      battle: () => battle,
      interaction,
      preferences: () => save.settings,
      frame,
      boardClick,
      cancel,
      ready: () => {
        ready = true;
        lastOverlay = "";
        renderHUD();
        fit();
      },
      modal: () => !!modal || page !== "battle" || run?.phase !== "battle",
    }),
    render: { antialias: true },
    audio: { noAudio: true },
    banner: false,
  });
  if (import.meta.env.DEV && (window as any).__gardenQA)
    (window as any).__gardenQA.game = game;
}
function cancel() {
  interaction.cancel();
  renderHUD();
}
function choose(k: Kind) {
  if (modal || run?.phase !== "battle") return;
  interaction.choose(k);
  audio.play("select");
  lesson("select");
  renderHUD();
}
function handle(error: string) {
  if (error) {
    toast(error);
    audio.play("invalid");
  } else audio.play("select");
  flushEvents();
  renderHUD();
}
function pause() {
  if (modal || run?.phase !== "battle") return;
  battle.pause();
  acc = 0;
  flushEvents();
  renderHUD();
}
function returnHome() {
  audio.reset();
  run?.end();
  run = null;
  interaction.cancel();
  battle = new Battle(0, true);
  scene?.clearBattle();
  showPage("home");
}
function confirmLeave(action: () => void) {
  if (run && ["battle", "draft"].includes(run.phase)) {
    openModal(
      "离开当前出征？",
      `<h2>让这次生长停在这里？</h2><p class="consequence">当前出征无法续玩。已发现的图鉴和解锁会保留。</p><div class="dialog-actions"><button data-close>取消，返回出征</button><button id="confirm-leave" class="primary">确认结束出征</button></div>`,
    );
    $("#confirm-leave").onclick = () => {
      closeModal(false);
      action();
    };
  } else action();
}
function openModal(title: string, body: string) {
  if (modal) return;
  const resume = battle.status === "running";
  if (resume) {
    battle.pause();
    flushEvents();
  }
  modal = { resume, focus: document.activeElement as HTMLElement };
  $("#modal-layer").innerHTML =
    `<section class="dialog" role="dialog" aria-modal="true" aria-label="${title}">${body}</section>`;
  $("#modal-layer").classList.remove("hidden");
  document
    .querySelectorAll<HTMLElement>("#app > main")
    .forEach((e) => (e.inert = true));
  $("#modal-layer")
    .querySelectorAll<HTMLElement>("[data-close]")
    .forEach((e) => (e.onclick = () => closeModal()));
  $("#modal-layer").querySelector<HTMLElement>("button,input")?.focus();
}
function closeModal(resume = true) {
  if (!modal) return;
  const m = modal;
  modal = null;
  $("#modal-layer").classList.add("hidden");
  document
    .querySelectorAll<HTMLElement>("#app > main")
    .forEach((e) => (e.inert = false));
  if (resume && m.resume && battle.status === "paused") {
    battle.pause();
    flushEvents();
  }
  renderHUD();
  m.focus?.focus();
}
function settings() {
  openModal(
    "游戏设置",
    `<span class="eyebrow">MAKE YOURSELF AT HOME</span><h2>让花园更合心意</h2><p>设置即时保存，下一次回到温室仍然生效。</p><fieldset class="settings-group"><legend>声音与氛围</legend>${[
      ["musicVolume", "背景音乐"],
      ["sfxVolume", "战斗音效"],
    ]
      .map(
        ([k, n]) =>
          `<label class="slider-label">${n}<output id="${k}-value">${Math.round(save.settings[k as "musicVolume"] * 100)}%</output><input data-volume="${k}" type="range" min="0" max="100" value="${save.settings[k as "musicVolume"] * 100}"></label>`,
      )
      .join(
        "",
      )}<label class="check-label"><input id="muted" type="checkbox" ${save.settings.muted ? "checked" : ""}>一键静音</label></fieldset><fieldset class="settings-group"><legend>动态与舒适度</legend><label class="check-label"><input id="reduced" type="checkbox" ${save.settings.reducedMotion ? "checked" : ""}>减少动态效果</label><p>关闭震屏，简化粒子。初始设置跟随系统偏好。</p></fieldset><div class="dialog-actions"><button data-close class="primary">保存并返回</button></div>`,
  );
  document.querySelectorAll<HTMLInputElement>("[data-volume]").forEach(
    (e) =>
      (e.oninput = () => {
        const k = e.dataset.volume as "musicVolume" | "sfxVolume";
        save.settings[k] = +e.value / 100;
        txt("#" + k + "-value", e.value + "%");
        audio.apply(save.settings);
        persist();
      }),
  );
  $("#muted").onchange = () => {
    save.settings.muted = $<HTMLInputElement>("#muted").checked;
    audio.apply(save.settings);
    persist();
  };
  $("#reduced").onchange = () => {
    save.settings.reducedMotion = $<HTMLInputElement>("#reduced").checked;
    document.body.classList.toggle(
      "reduced-motion",
      save.settings.reducedMotion,
    );
    persist();
  };
}
function help() {
  openModal(
    "出征手册",
    `<h2>园丁的出征手册</h2><div class="help-content"><p><b>布阵：</b>先建立阳光生产，再为每行布置射手。1–3 选卡，点击空地种植。普通浏览点击阳光收集；工具模式优先选择植物。</p><p><b>基因：</b>点击库存样本，再点击植物，预览并确认安装。每株四槽，同株不能重复安装。</p><p><b>杂交：</b>点击杂交，依次选择不同主系的两株植物，预览概率后确认。结果留在第二株的位置，材料消耗。</p><p><b>进化：</b>植物存活 60 秒达到三级，在查看面板选择一个进化方向。</p><p><b>突变：</b>点击植物，打开主动突变，阅读风险后确认。每株一次，不育植物无法继续杂交。</p><p><b>三场出征：</b>过场继承阵容和防线车，治疗 30% 最大耐久，补给 100 阳光；刷新或退出结束出征。</p><p>空格暂停 · Esc / 右键取消 · 离开页面自动暂停，回来主动继续。</p></div><div class="dialog-actions"><button id="replay">重看操作引导</button><button data-close class="primary">知道了</button></div>`,
  );
  $("#replay").onclick = () => {
    save.lessons = [];
    persist();
    closeModal();
  };
}
function previewGene(p: Plant, g: Gene) {
  const d = stats(
    { ...p, genome: [...(p.genome ?? []), g] },
    PLANTS[p.kind],
    battle.plants,
  );
  return `耐久 ${battle.getStats(p).hp} → ${d.hp}<br>伤害 ${Math.round(battle.getStats(p).damage ?? 0)} → ${Math.round(d.damage ?? 0)} · 射速 ${(1 / (d.rate ?? Infinity)).toFixed(2)} /秒${d.produce ? `<br>每 ${d.produce.toFixed(1)} 秒产 ${d.yield.toFixed(1)} ☀` : ""}`;
}
function statLine(p: Plant) {
  const d = battle.getStats(p);
  return `耐久 ${Math.ceil(p.hp)} / ${d.hp} · 伤害 ${Math.round(d.damage ?? 0)}<br>${d.rate ? (1 / d.rate).toFixed(2) + " 发/秒" : "无射击"}${d.produce ? ` · 每 ${d.produce.toFixed(1)} 秒产 ${d.yield.toFixed(1)} ☀` : ""}`;
}
function costButton(id: string, cost: number, label: string) {
  return `<button id="${id}" class="primary full" ${battle.sun < cost || battle.status !== "running" ? "disabled" : ""}>${battle.sun < cost ? `还缺 ${Math.ceil(cost - battle.sun)} 阳光` : `${label} · ${cost} ☀`}</button>`;
}
function renderInspector() {
  const focused = document.activeElement as HTMLElement;
  const focusId = focused?.closest("#inspector") ? focused.id : "";
  const focusEvolution = focused?.dataset.evolution;
  const s = interaction.state;
  const id =
    s.type === "browse"
      ? s.inspected
      : s.type === "gene" || s.type === "experiment"
        ? s.target
        : null;
  const p = battle.plants.find((p) => p.id === id);
  let html = "";
  const discovered = Object.keys(PHENOTYPES).filter(k => save.discoveries.includes("plant:" + k)).length;
  if (s.type === "fusion") {
    const a = battle.plants.find((p) => p.id === s.first),
      b = battle.plants.find((p) => p.id === s.second),
      r = a && b ? battle.recipe(a.id, b.id) : undefined;
    const pool = r && "pool" in r ? r.pool : [];
    html = `<span class="eyebrow">HYBRIDIZE</span><h2>让血统交汇</h2><div class="steps">${a ? "✓" : "①"} 第一株 → ${b ? "✓" : "②"} 第二株 → 确认</div><p>${a ? `${plantName(a)} · ${PLANTS[a.primaryFamily!].name}主系` : "点击第一株材料；可配对的植物会高亮。"}</p>${b && pool ? `<p>第二株：${plantName(b)}<br>结果位置：${b.row + 1} 行 ${b.col + 1} 列<br>继承耐久比例：${Math.round((a!.hp / battle.getStats(a!).hp + b.hp / battle.getStats(b).hp) * 50)}%<br>世代 F${Math.max(a!.generation ?? 1, b.generation ?? 1) + 1} · 主系 ${PLANTS[b.primaryFamily!].name}</p>${pool.map((k, i) => `<div class="recipe-line">${image(k)}<span>${PHENOTYPES[k].name}</span><b>${[60, 30, 10][i]}%</b></div>`).join("")}<p class="fine">第一株基因：${(a!.genome ?? []).map((g) => GENES[g].name).join("、") || "无"}<br>第二株基因：${(b.genome ?? []).map((g) => GENES[g].name).join("、") || "无"}<br>去重后等概率继承最多四个；消耗两株材料。进化、突变不继承。</p>${costButton("confirm-fusion", 100, "确认杂交")}` : ""}`;
  } else if (s.type === "gene") {
    const g = run?.samples[s.sample];
    html = g
      ? `<span class="eyebrow">GENE GRAFTING</span><h2>${GENES[g].name}</h2><p>${GENES[g].desc}</p>${p ? `<h3>${plantName(p)}</h3><p>${statLine(p)}</p><p>基因槽 ${(p.genome ?? []).length} / 4</p>${applicable(g, battle.getStats(p)) && !p.genome?.includes(g) ? `<p class="stats">安装后：${previewGene(p, g)}</p>` : ""}${applicable(g, battle.getStats(p)) ? `<button id="install" class="primary full" ${p.genome?.includes(g) || (p.genome ?? []).length >= 4 || battle.status !== "running" ? "disabled" : ""}>${p.genome?.includes(g) ? "该基因已安装" : (p.genome ?? []).length >= 4 ? "基因槽已满" : "确认安装 · 消耗 1 样本"}</button>` : "<p>不适用：缺少所需的射击或生产能力。样本会保留。</p>"}` : "<p>点击一株植物预览。样本不消耗阳光，成功后才从库存移除。</p>"}`
      : "<p>样本已失效，按 Esc 返回。</p>";
  } else if (p) {
    html = `<div class="plant-heading">${image(p.phenotype ?? p.kind)}<div><small>F${p.generation ?? 1} · Lv.${1 + Math.floor((p.xp ?? 0) / 30)}</small><h2>${plantName(p)}</h2><span>${PLANTS[p.primaryFamily!]?.name ?? ""}主系</span></div></div><p class="stats">${statLine(p)}</p><div class="genome">${Array.from(
      { length: 4 },
      (_, i) => {
        const g = p.genome?.[i];
        return `<span>${g ? GENES[g].name + (!applicable(g, battle.getStats(p)) ? " · 休眠" : "") : "空基因槽"}</span>`;
      },
    ).join(
      "",
    )}</div>${p.evolution ? `<p>进化 · ${EVOLUTIONS[p.evolution].name}</p>` : ""}${p.mutation ? `<p class="purple">突变 · ${MUTATIONS[p.mutation].name}<small>${MUTATIONS[p.mutation].desc}</small></p>` : ""}`;
    if (s.type === "experiment" && s.action === "mutate") {
      const pool = mutationPool(battle.getStats(p));
      html += `<h3>主动突变 · 不可撤销</h3><p>适用结果各 ${(100 / pool.length).toFixed(2)}%。</p>${pool.map((m) => `<p class="risk"><b>${MUTATIONS[m].name}</b> ${MUTATIONS[m].desc}</p>`).join("")}${costButton("confirm-mutate", 125, "接受风险并实验")}`;
    } else if (s.type === "experiment" && s.action === "evolve") {
      html += `<h3>选择一次生长方向</h3>${(p.evolutionChoices ?? [])
        .map((v) => {
          const d = stats(
            { ...p, evolution: v },
            PLANTS[p.kind],
            battle.plants,
          );
          return `<button class="evolution" data-evolution="${v}" ${battle.sun < 100 || battle.status !== "running" ? "disabled" : ""}><b>${EVOLUTIONS[v].name}</b><small>${EVOLUTIONS[v].desc}<br>耐久 ${battle.getStats(p).hp} → ${d.hp} · 伤害 ${Math.round(battle.getStats(p).damage ?? 0)} → ${Math.round(d.damage ?? 0)}<br>射速 ${(1 / (battle.getStats(p).rate ?? Infinity)).toFixed(2)} → ${(1 / (d.rate ?? Infinity)).toFixed(2)} /秒</small><span>${battle.sun < 100 ? `还缺 ${100 - battle.sun} ☀` : "确认进化 · 100 ☀"}</span></button>`;
        })
        .join("")}`;
    } else
      html += `${!p.evolution ? `<button id="evolve" class="full" ${(p.xp ?? 0) < 60 ? "disabled" : ""}>${(p.xp ?? 0) < 60 ? `再存活 ${Math.ceil(60 - (p.xp ?? 0))} 秒开放进化` : "预览进化方向 · 100 ☀"}</button>` : ""}${!p.mutation ? '<button id="mutate" class="full purple">预览主动突变 · 125 ☀</button>' : ""}`;
  } else if (s.type === "plant")
    html = `${image(s.kind, "inspector-portrait")}<h2>${PLANTS[s.kind].name}</h2><p>${PLANTS[s.kind].desc}</p><p>点击空格种植，成功后退出选卡。<br>Esc / 右键取消。</p>`;
  else
    html = `<span class="eyebrow">FIELD LABORATORY</span><h2>${s.type === "shovel" ? "铲除模式" : "你的花园，由你构筑。"}</h2><p>${s.type === "shovel" ? "点击植物移除，不返还阳光。" : "点击植物查看血统与基因。点击样本安装，或选择杂交开始实验。"}</p><p class="fine">工具优先选植物 · Esc 取消<br>查看工具可穿过阳光选择植物</p>`;
  $("#inspector").innerHTML = html;
  if (focusId) $("#" + focusId)?.focus({ preventScroll: true });
  else if (focusEvolution)
    document
      .querySelector<HTMLElement>('[data-evolution="' + focusEvolution + '"]')
      ?.focus({ preventScroll: true });
  $("#confirm-fusion")?.addEventListener("click", () =>
    handle(interaction.confirm(battle)),
  );
  $("#install")?.addEventListener("click", () => {
    if (s.type === "gene" && p) {
      const e = run!.install(s.sample, p.id);
      if (!e) interaction.state = { type: "browse", inspected: p.id };
      handle(e);
    }
  });
  $("#evolve")?.addEventListener("click", () => {
    interaction.state = { type: "experiment", target: p!.id, action: "evolve" };
    renderHUD();
  });
  $("#mutate")?.addEventListener("click", () => {
    interaction.state = { type: "experiment", target: p!.id, action: "mutate" };
    renderHUD();
  });
  $("#confirm-mutate")?.addEventListener("click", () => {
    const e = battle.mutate(p!.id);
    if (!e) interaction.state = { type: "browse", inspected: p!.id };
    handle(e);
  });
  document.querySelectorAll<HTMLElement>("[data-evolution]").forEach(
    (e) =>
      (e.onclick = () => {
        const error = battle.evolveGenetic(
          p!.id,
          e.dataset.evolution as Evolution,
        );
        if (!error) interaction.state = { type: "browse", inspected: p!.id };
        handle(error);
      }),
  );
}
function renderOverlay() {
  const key = JSON.stringify([
    run?.phase,
    run?.choices,
    run?.draftRemaining,
    battle.status,
    ready,
    run?.stage,
  ]);
  if (key === lastOverlay) return;
  lastOverlay = key;
  const el = $("#overlay");
  el.classList.toggle(
    "hidden",
    run?.phase === "battle" && battle.status === "running",
  );
  if (!run) return;
  if (run.phase === "draft") {
    el.innerHTML = `<section class="draft-dialog"><span class="eyebrow">GENE SELECTION / ${4 - run.draftRemaining} OF 3</span><h2>选择一枚基因，让可能性生根。</h2><p>${run.stage ? "阵容已治疗，获得 100 阳光补给。" : "三次选择，一次全新的构筑。"}样本可留待以后安装。</p><div class="draft-grid">${run.choices.map((g) => `<button data-draft="${g}" aria-label="选择${GENES[g].name}">${geneCard(g)}<span class="pick-label">选取样本 →</span></button>`).join("")}</div></section>`;
    el.querySelectorAll<HTMLElement>("[data-draft]").forEach(
      (e) =>
        (e.onclick = () => {
          run!.pick(e.dataset.draft as Gene);
          flushEvents();
          renderHUD();
        }),
    );
    return;
  }
  let body = "";
  if (battle.status === "ready")
    body = `<span class="eyebrow">EXPEDITION / 0${run.stage + 1}</span><h2>${LEVELS[run.stage].name}</h2><p>${WORLD.stages[run.stage].text}</p><button id="start" class="primary" ${ready ? "" : "disabled"}>${ready ? "开始守护 →" : "准备花园素材…"}</button><small>☀ ${Math.round(battle.sun)} 阳光 · 三种母体 · 四槽基因</small>`;
  if (battle.status === "paused")
    body =
      '<h2>让花园歇一会儿。</h2><p>战斗、音乐和生长计时已暂停。</p><button id="resume" class="primary">继续守护 →</button>';
  if (battle.status === "won" || battle.status === "lost")
    body = `<span class="eyebrow">${battle.status === "won" ? "GROW THE IMPOSSIBLE" : "EVERY SEED IS A NEW BEGINNING"}</span><h2>${run.phase === "won" ? "不可能，也开花了。" : battle.status === "won" ? "这座花园，守住了。" : "带着发现，下次再来。"}</h2><div class="result-stats"><b>${fmt(run.elapsed)}<small>出征用时</small></b><b>${run.kills}<small>击退数量</small></b><b>${run.discoveries.length}<small>本次新发现</small></b></div><p>已解锁：${save.genes.length ? save.genes.map((g) => GENES[g].name).join("、") : "继续实验，解锁更多基因"}</p>${run.stage < 2 && battle.status === "won" ? '<button id="next" class="primary">领取样本，前往下一座花园 →</button>' : '<button id="home-result" class="primary">返回温室 →</button>'}`;
  el.innerHTML = `<section class="dialog">${body}</section>`;
  $("#start")?.addEventListener("click", () => {
    void audio.unlock();
    battle.start();
    flushEvents();
    renderHUD();
  });
  $("#resume")?.addEventListener("click", () => {
    void audio.unlock();
    pause();
  });
  $("#home-result")?.addEventListener("click", returnHome);
  $("#next")?.addEventListener("click", () => {
    if (run!.next()) {
      audio.reset();
      scene?.clearBattle();
      battle = run!.battle;
      interaction.cancel();
      acc = 0;
      lastHUD = lastOverlay = "";
      bannerUntil = 0;
      renderHUD();
    }
  });
}
const lessons: [string, string][] = [
  ["draft", "每次三选一，选取三个基因样本。样本可保留。"],
  ["select", "先按 1 选择阳光花，在左侧空格种下它。"],
  ["plant", "点击空地完成种植，成功后自动取消选卡。"],
  ["collect", "普通浏览点击金色阳光。先建立资源，再补齐每行射手。"],
  ["gene", "点击下方基因样本，再选植物，确认安装。"],
  ["fuse", "打开杂交，选不同主系的两株，查看概率后确认。"],
  ["evolve", "植物存活 60 秒达到三级，点击它预览进化。"],
  ["mutate", "在植物面板预览主动突变，读完风险再确认。"],
];
function renderHUD() {
  if (page !== "battle" || !run) return;
  interaction.reconcile(battle);
  const s = interaction.state;
  const key = JSON.stringify([
    s,
    battle.sun,
    battle.status,
    Math.floor(battle.time),
    battle.plants.map((p) => [
      p.id,
      p.phenotype,
      p.genome,
      p.evolution,
      p.mutation,
      Math.ceil(p.hp),
      (p.xp ?? 0) >= 60,
    ]),
    run.samples,
    save.lessons,
    run.phase,
    ready,
  ]);
  txt("#sun", String(Number(battle.sun.toFixed(1))));
  txt(
    "#timer",
    fmt(
      run.elapsed + (["won", "lost"].includes(battle.status) ? 0 : battle.time),
    ),
  );
  txt("#stage-name", LEVELS[run.stage].name);
  txt("#stage-number", `0${run.stage + 1} / 03`);
  txt("#wave-label", `${battle.spawned} / ${battle.schedule.length} 敌人`);
  txt("#pause", battle.status === "paused" ? "▶ 继续" : "Ⅱ 暂停");
  txt("#sound", save.settings.muted ? "♫ 静音" : "♫");
  txt(
    "#hint",
    `${{ browse: "点击阳光收集 · 点击植物查看", plant: "种植模式", fusion: "杂交：第一株 → 第二株 → 确认", shovel: "铲除模式", gene: "基因：选择目标 → 确认", experiment: s.type === "experiment" && s.action === "inspect" ? "查看工具 · 优先选择植物" : "实验预览 · 战斗继续" }[s.type]} · Esc 取消`,
  );
  for (const el of document.querySelectorAll<HTMLElement>(".seed")) {
    const k = el.dataset.kind as Kind;
    const cd = Math.max(0, (battle.cooldowns[k] ?? 0) - battle.time);
    const status =
      cd > 0
        ? `冷却 ${Math.ceil(cd)}s`
        : battle.sun < PLANTS[k].cost
          ? `缺 ${Math.ceil(PLANTS[k].cost - battle.sun)} ☀`
          : "可种植";
    if (el.querySelector(".card-status")!.textContent !== status)
      el.querySelector(".card-status")!.textContent = status;
    el.classList.toggle("selected", s.type === "plant" && s.kind === k);
    const pct = Math.ceil((cd / PLANTS[k].cd) * 100);
    if (el.dataset.pct !== String(pct)) {
      el.dataset.pct = String(pct);
      el.querySelector<HTMLElement>(".cooldown")!.style.height = pct + "%";
    }
  }
  if (key !== lastHUD) {
    lastHUD = key;
    renderInspector();
    const sampleKey = JSON.stringify([
      run.samples,
      s.type === "gene" ? s.sample : -1,
    ]);
    if ($("#samples").dataset.key !== sampleKey) {
      $("#samples").dataset.key = sampleKey;
      $("#samples").innerHTML = run.samples.length
        ? run.samples
            .map(
              (g, i) =>
                `<button data-sample="${i}" class="${s.type === "gene" && s.sample === i ? "active" : ""}" title="${GENES[g].desc}">${GENES[g].name}</button>`,
            )
            .join("")
        : "<small>暂无样本 · 过场领取新基因</small>";
      document.querySelectorAll<HTMLElement>("[data-sample]").forEach(
        (e) =>
          (e.onclick = () => {
            interaction.state = {
              type: "gene",
              sample: +e.dataset.sample!,
              target: null,
            };
            renderHUD();
          }),
      );
    }
    const next = lessons.find(([id]) => !save.lessons.includes(id));
    $("#tutorial").innerHTML = next
      ? `<small>园丁手记</small><p>${next[1]}</p><button id="skip">跳过全部引导</button>`
      : "";
    $("#skip")?.addEventListener("click", () => {
      save.lessons = lessons.map(([id]) => id);
      persist();
      renderHUD();
    });
  }
  renderOverlay();
}
function flySun(e: BattleEvent) {
  const rect = $("#game").getBoundingClientRect(),
    to = $(".sun-meter").getBoundingClientRect();
  const el = document.createElement("span");
  el.className = "sun-flight";
  el.textContent = "☀";
  el.style.left = rect.x + ((X + (e.x! + 0.5) * CW) / 1120) * rect.width + "px";
  el.style.top =
    rect.y + ((Y + (e.row! + 0.3) * CH) / 620) * rect.height + "px";
  document.body.append(el);
  const dx = to.x + to.width / 2 - parseFloat(el.style.left),
    dy = to.y + 25 - parseFloat(el.style.top);
  el.animate(
    [
      { opacity: 1, transform: "translate(0,0)" },
      {
        opacity: 0,
        transform: save.settings.reducedMotion
          ? "none"
          : `translate(${dx}px,${dy}px) scale(.3)`,
      },
    ],
    { duration: save.settings.reducedMotion ? 120 : 500 },
  )
    .finished.then(() => el.remove())
    .catch(() => el.remove());
  txt("#sun-gain", "+" + (e.value ?? 25));
  $("#sun-gain").animate([{ opacity: 1 }, { opacity: 0 }], { duration: 700 });
}
function flushEvents() {
  const events = battle.drainEvents();
  scene?.consume(events);
  let dirty = false;
  for (const e of events) {
    if (e.type === "pause") {
      audio.setRunning(false);
      document
        .querySelectorAll(".sun-flight")
        .forEach((el) => el.getAnimations().forEach((a) => a.pause()));
    }
    if (e.type === "start" || e.type === "resume") {
      audio.setRunning(true);
      document
        .querySelectorAll(".sun-flight")
        .forEach((el) => el.getAnimations().forEach((a) => a.play()));
    }
    if (e.type === "won" || e.type === "lost") {
      audio.setRunning(false);
      if (run?.settle(save)) dirty = true;
    }
    audio.play(e.type);
    if (e.type === "collect") flySun(e);
    if (lessons.some(([id]) => id === e.type)) lesson(e.type);
    if (e.type === "warning" || e.type === "finalWave") {
      txt(
        ".wave-banner",
        e.type === "warning"
          ? "⚑ 一大波敌人正在接近"
          : "⚑ 最后一波，守住花园！",
      );
      bannerUntil = battle.time + 3;
      $(".wave-banner").classList.add("show");
    }
    if (e.type === "fuse") {
      unlock(save, "reserve");
      dirty = true;
    }
    if (e.type === "evolve") {
      unlock(save, "thorns");
      dirty = true;
    }
    if (e.type === "mutate") {
      unlock(save, "shell");
      toast("突变结果：" + MUTATIONS[e.detail as keyof typeof MUTATIONS].name);
      dirty = true;
    }
    const discovery =
      e.type === "fuse"
        ? "plant:" + e.detail
        : e.type === "gene"
          ? "gene:" + e.detail
          : e.type === "mutate"
            ? "mutation:" + e.detail
            : e.type === "evolve"
              ? "evolution:" + e.detail
              : e.type === "plant"
                ? "plant:" + e.texture
                : null;
    if (discovery && !save.discoveries.includes(discovery)) {
      save.discoveries.push(discovery);
      run?.discoveries.push(discovery);
      dirty = true;
    }
  }
  if (dirty) persist();
}
function boardClick(x: number, y: number) {
  if (
    modal ||
    page !== "battle" ||
    run?.phase !== "battle" ||
    battle.status !== "running"
  )
    return;
  if (interaction.canCollect) {
    const sun = battle.suns.find(
      (s) =>
        Math.hypot(x - (X + (s.x + 0.5) * CW), y - (Y + (s.row + 0.3) * CH)) <
        29,
    );
    if (sun) {
      battle.collect(sun.id);
      flushEvents();
      renderHUD();
      return;
    }
  }
  const row = Math.floor((y - Y) / CH),
    col = Math.floor((x - X) / CW);
  if (row < 0 || row > 4 || col < 0 || col > 8) return;
  handle(interaction.click(battle, row, col));
}
function frame(delta: number, s: Garden) {
  scene = s;
  if (
    page === "battle" &&
    !modal &&
    run?.phase === "battle" &&
    battle.status === "running"
  ) {
    acc += Math.min(delta / 1000, 5 / 60);
    while (acc >= 1 / 60) {
      battle.tick(1 / 60);
      acc -= 1 / 60;
    }
  } else acc = 0;
  flushEvents();
  audio.update(battle.time, battle.time >= battle.endTime);
  if (!bannerUntil || battle.time > bannerUntil)
    $(".wave-banner").classList.remove("show");
  renderHUD();
}
function fit() {
  requestAnimationFrame(() => {
    if (page !== "battle") return;
    const panel = $(".game-panel");
    if (innerWidth >= 900) {
      const top = $(".workspace").getBoundingClientRect().top;
      const width = Math.floor(
        Math.min(innerWidth - 310, ((innerHeight - top - 186) * 1120) / 620),
      );
      $(".workspace").style.gridTemplateColumns =
        `${Math.max(560, width)}px 264px`;
    } else $(".workspace").style.gridTemplateColumns = "1fr";
    game?.scale.refresh();
  });
}
$("#expedition").onclick = () => startRun();
document
  .querySelectorAll<HTMLElement>("[data-page]")
  .forEach((e) => (e.onclick = () => showPage(e.dataset.page!)));
document
  .querySelectorAll<HTMLElement>("[data-home]")
  .forEach((e) => (e.onclick = () => showPage("home")));
document
  .querySelectorAll<HTMLElement>("[data-settings]")
  .forEach((e) => (e.onclick = settings));
document
  .querySelectorAll<HTMLElement>("[data-kind]")
  .forEach((e) => (e.onclick = () => choose(e.dataset.kind as Kind)));
$("#leave").onclick = () => confirmLeave(returnHome);
$("#restart").onclick = () => confirmLeave(() => startRun());
$("#pause").onclick = pause;
$("#help").onclick = help;
$("#cancel").onclick = cancel;
$("#shovel").onclick = () => {
  interaction.tool("shovel");
  renderHUD();
};
$("#fusion").onclick = () => {
  interaction.tool("fusion");
  renderHUD();
};
$("#inspect").onclick = () => {
  interaction.state = { type: "experiment", target: 0, action: "inspect" };
  renderHUD();
};
$("#sound").onclick = () => {
  save.settings.muted = !save.settings.muted;
  audio.apply(save.settings);
  persist();
  renderHUD();
};
window.addEventListener("keydown", (e) => {
  if (modal) {
    if (e.key === "Escape") {
      e.preventDefault();
      closeModal();
    }
    if (e.key === "Tab") {
      const a = [
        ...$("#modal-layer").querySelectorAll<HTMLElement>(
          "button:not(:disabled),input",
        ),
      ];
      e.preventDefault();
      const index = a.indexOf(document.activeElement as HTMLElement);
      a[
        index < 0
          ? e.shiftKey
            ? a.length - 1
            : 0
          : (index + (e.shiftKey ? -1 : 1) + a.length) % a.length
      ]?.focus();
    }
    return;
  }
  if (page !== "battle" || e.target instanceof HTMLInputElement) return;
  if (e.key === "Escape") cancel();
  if (e.code === "Space" && !(e.target instanceof HTMLButtonElement)) {
    e.preventDefault();
    pause();
  }
  if (/^[1-3]$/.test(e.key)) choose(FAMILIES[+e.key - 1]);
});
function suspend() {
  if (modal) modal.resume = false;
  if (battle.status === "running") {
    battle.pause();
    flushEvents();
    renderHUD();
  }
  audio.setRunning(false);
}
document.addEventListener("visibilitychange", () => {
  if (document.hidden) suspend();
});
window.addEventListener("blur", suspend);
window.addEventListener("pagehide", () => audio.reset());
window.addEventListener("resize", fit);
showPage("home");
document.body.classList.toggle("reduced-motion", save.settings.reducedMotion);
if (!loaded.ok) toast("存档读取异常，已恢复可用记录");
if (import.meta.env.DEV && new URLSearchParams(location.search).has("qa"))
  (window as any).__gardenQA = {
    battle: () => battle,
    run: () => run,
    reset: (level = 0) => {
      startRun(123);
      while (run!.phase === "draft") run!.pick(run!.choices[0]);
      if (level) {
        run!.stage = level;
        battle = run!.battle = new Battle(level, true, run!.rng);
      }
      renderHUD();
    },
    audio,
    interaction,
    save,
    flushEvents,
    renderHUD,
    startRun,
    boardClick,
  };
