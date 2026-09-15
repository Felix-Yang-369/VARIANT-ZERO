# 零号变种 · 独立敌方角色资产

两只角色各自使用独立文件夹与 `.blend` 文件，未混入原初生态研究所。

| 素材键 | 已有角色名称 | 主文件 |
| --- | --- | --- |
| enemy0 | 苔行者 | `enemy0_苔行者/苔行者_enemy0_v1.blend` |
| enemy2 | 疾藤潜行者 | `enemy2_疾藤潜行者/疾藤潜行者_enemy2_v1.blend` |

名称核对自 `src/model.ts` 的敌人定义、`src/lore.ts` 的 WITHER 顺序、`src/scene.ts` 的素材载入与 `docs/07-v0.3素材记录.md` 的 enemy0 / enemy2 描述。不另造物种编号。

苔行者包含木质躯体与四肢、紫色眼睛、枝角、带孔破叶兜帽与背披风、苔藓、紫菌、新芽、白花和木种护符。疾藤潜行者包含前倾跑姿、木面甲、紫眼、卷藤四肢与根爪、后掠紫叶冠、背叶和苔藓。

每个文件的角色部件位于“角色名 • 角色资产”根集合。`90` 集合是独立预览地面、灯光与相机，不属于角色；`99` 是隐藏且打包的参考图。可以在其他 Blender 场景中 Link 角色根集合。材质均为程序材质，角色不依赖外部图片贴图。

每个目录有三分之四与背面预览、资产清单和验证结果。使用 Blender 5.2、Cycles、RTX 5060 + OptiX 生成；每个进程都显式设置 GPU 后端，无可用后端会报错。手工打开 Blender 时仍须在用户偏好中启用 OptiX。

这是依据单张图片制作的风格化三维概念首版，背面由建模补全，未达到原图的全部微观细节。角色为静态姿态，尚未绑定骨骼、制作动作、游戏低模、碰撞体、UV 烘焙或引擎接入。

## 重新生成

在当前目录执行以下命令。脚本会重建对应主文件和预览图。

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --factory-startup --python '.\build_enemies.py' -- enemy0
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --factory-startup --python '.\build_enemies.py' -- enemy2
```

生成脚本需要上一级 `configure_gpu.py` 及项目 `public/art/enemy0.png` / `enemy2.png`。完成后的 `.blend` 本身可独立移动与打开。
