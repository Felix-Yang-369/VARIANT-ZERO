from pathlib import Path
import shutil, zipfile, hashlib, json
from datetime import datetime

game=Path(r'D:\Projects\零号变种')
ue=game/'UE'
package=(ue/'当前试玩包.txt').read_text(encoding='utf-8-sig').strip()
source=ue/package/'Windows'
stamp=datetime.now().strftime('%Y%m%d-%H%M%S')
name='VariantZero-Preview-'+stamp
out=game/'分享试玩'/name
out.mkdir(parents=True,exist_ok=False)
entries=[]
for file in source.rglob('*'):
    if not file.is_file(): continue
    rel=file.relative_to(source)
    if 'Saved' in rel.parts or file.suffix.lower()=='.pdb' or file.name.startswith('Manifest_'):continue
    target=out/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(file,target)
    entries.append(dict(path=rel.as_posix(),bytes=file.stat().st_size,sha256=hashlib.sha256(file.read_bytes()).hexdigest()))
(out/'开始试玩.bat').write_bytes(('@echo off\r\ncd /d "%~dp0"\r\nstart "" "%~dp0VariantZeroUE.exe" -VZStory -windowed -ResX=1920 -ResY=1080\r\n').encode('utf-8'))
readme='''零号变种：未完成的春天 — 朋友试玩版

这是开发中的 Windows 单机原型，不是完整 1.0。请先解压整个压缩包，再双击「开始试玩.bat」。
不用安装 Unreal Engine、Blender 或登录开发者账号。不要只复制 exe，也不要直接在压缩包内运行。

平台：Windows 10/11 64 位，游戏使用 DirectX 12。其他电脑配置尚未做兼容性普测。
开发机为 RTX 5060 8GB / 32GB 内存，尚未发布经过验证的最低配置或帧率保证。
请预留至少 3GB 空间用于下载、解压和运行日志。Mac、手机不能直接运行这个包。

如启动提示 VCRUNTIME140 / MSVCP140 缺失，请安装微软官方最新 Visual C++ v14 x64 运行库：
https://aka.ms/vc14/vc_redist.x64.exe
说明：https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
不需要下载单独的 DLL 文件。运行库安装需要联网，游戏本体离线运行。

操作：
WASD 移动，鼠标看向，空格跳跃，左 Ctrl 闪避。
E 交互，Q 扫描，左键脉冲，右键护盾，1/2/3 伙伴技能。
G 指定伙伴技能落点，R 召回，Esc 打开存档、设置、地图和伙伴管理。
按左上角任务提示，从温室档案台开始，完成采样、培育和修复，再前往花庭。
温室/花庭主线进展提供培养点；回研究所培养台附近可在伙伴管理中升级。

当前可玩：温室开场、初芽跟随、菌翼蛾研究与传粉、花庭光照机关与两阶段守护者、
修复水路与捷径、初芽短篇故事、伙伴改名/编队/升级、地图与快速移动、存档和声音设置。
已接入探索 BGM、战斗切换、工具音效、主角施法/受击动作及半透明护盾。

仍未完成：主线中文配音、完整三片区域、全部 12 种伙伴、原生 45 物种 20 回合演练、
最终美术与完整结局。Suno 主题曲未下载接入。尚未完成全流程及两小时稳定性验收。

存档：%LOCALAPPDATA%\\VariantZero\\SaveGames\\Story
自动保存 + 三个手动槽，每槽保留 .bak 备份。此试玩包不包含作者的存档或账号信息。
更新到后续共享存档版本时仍使用同一位置；不要删除该目录。

反馈建议：
1. 第一次不看额外解释，能否知道下一步该做什么？
2. 哪个机关、战斗或界面令人困惑？
3. 若遇到卡住、崩溃或掉帧，请附截图、最后三步操作、显卡和内存信息。
4. 游戏日志在 VariantZeroUE/Saved/Logs；发送前检查是否含个人路径。

这是好友内部体验材料，不是完成授权审查、签名及兼容性验收的正式发行版。
'''
(out/'先读我-试玩说明.txt').write_text(readme,encoding='utf-8-sig')
(out/'文件校验.json').write_text(json.dumps(entries,ensure_ascii=False,indent=2),encoding='utf-8')
archive=out.with_suffix('.zip')
with zipfile.ZipFile(archive,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=3) as z:
    for p in out.rglob('*'):
        if p.is_file():z.write(p,Path(name)/p.relative_to(out))
verify=ue/'Saved'/'Automation'/'FriendPackage'/stamp
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    z.extractall(verify)
report=dict(archive=str(archive),bytes=archive.stat().st_size,sha256=hashlib.sha256(archive.read_bytes()).hexdigest(),source=package,verified_extraction=str(verify/name),files=len(entries),excluded=['Saved','*.pdb','Manifest_*'])
(ue/'Docs/friend-package.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
