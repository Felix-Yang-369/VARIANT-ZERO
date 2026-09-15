# 零号变种：未完成的春天

工程已整合到 `D:\Projects\零号变种\UE`，原 `D:\Projects\VariantZeroUE` 为兼容联接。总目录 `开始游戏.bat` 根据 `UE/当前试玩包.txt` 启动最新通过验证的版本。后续开发新增三伙伴独立技能、菌翼蛾群体恢复、队伍状态 HUD 和旧键位迁移，说明见 `Docs/三伙伴独立技能.md`。

最新资产接入：**V-041 菌翼蛾传粉伙伴候选**，运行 `打开传粉伙伴试玩.bat`。完成温室回访后在培养台研究，可组成初芽、新芽、菌翼蛾三只伙伴队伍，再回花床完成传粉。含绑定／动画／LOD、研究事务与传粉中断恢复。详见 `Docs/菌翼蛾接入.md`；完整 1.0 尚未完成。

最新可试内容为 **开场流程原型「温室留言」**：运行 `打开开场流程试玩.bat`。接通留言、扫描、三处采样、确定性培育、协作练习、修复与回访；包含原生 UMG 档案和任务存档。使用独立 `BuildOutput-Prologue`，旧包保留。详见 `Docs/开场流程原型.md`。仍非 P1 完整样章或 1.0；下文为此前 P0 基础记录。

当前为 **P0 生态美术定标候选版**，不是开放世界 1.0 成品。UE 5.8.2，C++ 与蓝图混合工程。初芽和温室已接入基础战斗；主角已接入 Belica 绑定模型与项目原生动画驱动；生态师服装、握持和环境的最终美术验收尚未完成。

生态美术批次加入生态师材质副本、原创育生杖和 4 组 CC0 环境资产，并修复贴地和导航构建问题。Windows 打包、五组功能回归与实际预览通过；详见 `Docs/P0-生态美术定标候选.md` 和 `Docs/ecology-verification.json`。该批次未使用 Hyper3D 额度，未购买素材。后续已使用获授权的现有 Hyper3D 额度生成菌翼蛾候选，并经用户明确授权下载，完成绑定和运行验证；记录见 `Docs/菌翼蛾生成记录.json`。

此前新增：15 项键鼠/11 项手柄按钮重绑定，独立设置持久化、动态按键提示、离线双层音乐与工具提示音，以及重建导航后的外景候选。完整 1.0 尚未完成；详见 `Docs/1.0交付门槛.md`。

此前新增：按住 G / 手柄 LB 指定地面技能位置，松开提交；R / 右摇杆按下召回。初芽实际绕障前往落点，抵达后施放范围技能并归队。详见 `Docs/P0-伙伴指挥版.md`。

此前新增：Esc / 手柄左菜单键打开暂停终端，支持三槽存读档、自动槽读取、难度、总音量与退出确认。菜单可用鼠标、方向键/Enter 或手柄操作，详见 `Docs/P0-暂停存档版.md`。

战斗玩法：T 激活温室失衡节点 → Q 标记 → 按住左键脉冲、右键护盾、1 指挥初芽 → 避开橙色预警 → 平息异常。F1 切换难度。初芽自动攻击、倒地不永久死亡，主角倒下回检查点。详见 `Docs/P0-协作战斗版.md`。

前一批已接入：初芽的 11 骨绑定、待机/移动动画、四张烘焙贴图、三级 LOD，温室拱架/培养岛/日光/导航障碍，以及修复花床的材质状态恢复。详情见 `Docs/P0-资产接入版.md`。

本批角色接入说明见 `Docs/P0-写实角色接入.md`；打包回归记录见 `Docs/hero-verification.json`。完整 1.0 目标尚未完成。

## 运行

双击 `打开P0测试版.bat`，或运行 `BuildOutput-P0-Ecology/Windows/VariantZeroUE.exe`。上一版 `BuildOutput-P0-Hero` 保留。上一版 `BuildOutput-P0-Foundation` 保留。此前 `BuildOutput-P0-Orders`、`BuildOutput-P0-Menu`、`BuildOutput-P0-Combat`、`BuildOutput-P0-Next`、`BuildOutput-P0` 和 `BuildOutput` 均保留。不同打包目录的玩家存档目录独立，本批没有自动迁移或覆盖旧版存档。

源码工程：`VariantZeroUE.uproject`。默认地图 `/Game/VariantZero/Maps/P0_Conservatory_Ecology`；已经打开的旧编辑器需重新打开此工程才能加载更新的原生模块。温室场景已保存于地图，旧灰盒地图保留。

| 操作 | 键位 |
|---|---|
| 移动 / 镜头 | WASD / 鼠标 |
| 跳跃 / 冲刺 / 位移闪避 | 空格 / Shift / Ctrl |
| 生态扫描 / 靠近中央花床修复 | Q / E |
| 设置当前地面为检查点 / 测试复活 | C / K |
| 选择手动槽 / 保存 / 读取 | F6 / F5 / F9 |
| 激活节点 / 标记目标 | T / Q |
| 连续脉冲 / 短时护盾 / 初芽技能 | 按住左键 / 右键 / 1 |
| 切换故事、标准难度 | F1 |

初芽自动跟随，并按移动速度切换动画。修复花床获得 3 份研究样本并改变花瓣材质，重复交互或读档不会再次发放同一笔奖励。当前手动读取回到记录的检查点。HUD 显示槽位、样本数量和保存错误。

基础手柄映射：左摇杆移动、右摇杆镜头，底部键跳跃、右键闪避、左键交互、顶部键扫描、左摇杆按下冲刺。暂停与存读档菜单已支持手柄输入；重绑定页面已完成；最终 UMG 界面和实体手柄人工验收仍未完成。

## 本次已验证

- 基础整合版打包成功，4 个原生测试套件通过、0 失败；伙伴指挥、导航/存档、战斗、暂停/重绑定/音频状态四组打包运行检查全部 PASS，退出码均为 0。见 `Docs/foundation-verification.json`。
- 预览：`Previews/零号变种-P0-基础整合.png`。

以下为此前伙伴指挥版记录：

- 伙伴指挥版打包成功；`VZ_ORDERS`、`VZ_SMOKE`、`VZ_COMBAT`、`VZ_MENU` 均 PASS，四次打包运行退出码均为 0。原生测试 3 个通过、0 失败。详见 `Docs/orders-verification.json`。
- 实际预览：`Previews/零号变种-P0-伙伴指挥.png`。

以下为此前暂停存档版记录：

- 暂停存档版打包成功；720p / 1080p 菜单测试、原有战斗与存档冒烟检查全部通过，退出码均为 0。原生测试 3 个通过、0 失败。详情见 `Docs/menu-verification.json`。
- 实际菜单预览：`Previews/零号变种-P0-暂停存档.png`。

以下为此前协作战斗版记录：

- 协作战斗版构建、打包成功；3 个原生测试套件通过、0 失败。打包运行中的战斗输入、冷却、遮挡、自动攻击、节点平息、倒地恢复与原有存档/导航/骨架检查全部 PASS。证据见 `Docs/combat-verification.json`。
- 实际打包预览：`Previews/零号变种-P0-协作战斗.png`（1920×1080）。

以下为此前资产接入版记录：

- 资产接入版打包成功，`p0-next-packaging.log`；打包运行退出码 0，`p0-next-runtime.log` 中 `VZ_RIG`、`VZ_MATERIAL`、`VZ_NAV`、`VZ_SMOKE` 全部 PASS。
- 导航路线以 5 个路径点绕过真实阻挡物；重新查看打包截图确认初芽已使用绿色/象牙色材质，并非灰色默认材质。
- Blender 文件保存重开后验证实际动画变形；UE 重新加载后网格与两段动画均保有正确骨架引用。

以下为基础原型的持续验证记录：

- 原生 Editor / Win64 Development 编译、Cook、Stage、Archive 成功；`p0-packaging.log`。
- UE 自动化报告 **当时 2 个测试套件通过，0 失败**：12/45 物种边界、存档校验、损坏备份恢复、未来版本拒绝覆盖、非法状态拒绝、修复奖励持久去重；`TestResults/Native/index.json`。
- 打包程序 D3D12 启动；自动验证移动、开阔平面伙伴跟随、检查点、3 手动槽保存、读取、奖励去重与复活，`VZ_SMOKE: PASS`，退出码 0；`p0-runtime.log`。
- 打包画面已截图查看：`BuildOutput-P0/Windows/VariantZeroUE/Saved/Screenshots/Windows/VZ_P0_smoke.png`。图中 Quinn、球体、方块均为代理资产。
- 冻结旧 v0.4 的 12 个源/配置文件与 SHA-256 清单，并导出物种、随机、商店及战斗对照数据。没有修改旧网页代码或存档。

这不是完整按键人工验收、复杂寻路验收、正式美术验收、全流程通关或性能测试。战斗已有工具试作，正式主线、培育、中文配音、演练原生移植尚未完成。

## 文件与开发

- `Source/VariantZeroUE/VZState.*`：版本化状态、存档文件、P0 修复事务。
- `Source/VariantZeroUE/VZPrototype.*`：原生角色、伙伴代理、测试地图逻辑与 HUD。
- `Source/VariantZeroUE/Tests/`：使用独立 GUID 临时目录的测试，不读写玩家存档。
- `Reference/v0.4/`：冻结网页基线与对照样例，不是完成的 C++ 移植。
- `Docs/1.0实施追踪.md`：批准的产品边界、已实现内容与剩余阶段。
- `Docs/资产登记.csv`：资产来源与发行核对状态。
- `Scripts/build_p0.ps1`、`Scripts/test_native.ps1`：复现构建和测试。

编译后运行 `Scripts/test_native.ps1`。对照数据生成使用 `Tools/LegacyOracle` 中独立 Windows 工具链；先 `npm ci --prefix Tools/LegacyOracle`，再用其中的 tsx 执行 `Scripts/export_legacy_fixtures.mts`。

主存档在运行程序对应的 `Saved/SaveGames/VariantZero`，包括 `slot_0.vzsave` 自动槽与 `slot_1..3.vzsave` 手动槽。`.bak` 为上一版。损坏且无有效备份或版本不支持时不覆盖；不支持未来版本不会降级读取旧备份。

开发包加 `-VZSmokeTest` 会自动运行并退出；它使用 `Saved/Automation/Runtime/<随机ID>`，与玩家存档完全隔离。

下一道门槛是写实主角、初芽候选版的美术与动作修整、温室品质定标及 P0 剩余交互基础，再进入 30–45 分钟样章。样章通过前不批量制作后续区域与全部配音。

