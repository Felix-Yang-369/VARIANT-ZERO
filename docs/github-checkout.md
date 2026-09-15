# GitHub 项目恢复与开发

当前基线版本：**0.2.24**。这是现有可玩原型的首次 Git 快照，不代表完整 1.0，也不还原之前 24 次开发的 Git 历史。

仓库保留经典版源码、UE 源码/配置/Content、3D 源文件、音乐、文档和制作脚本。大文件使用 Git LFS；构建产物、缓存、日志、个人存档、Blender 自动备份及本地历史试玩包不上传。

## 获取完整资源

先安装 Git（含 Git LFS），然后：

```powershell
git lfs install
git clone https://github.com/Felix-Yang-369/variant-zero.git D:\Projects\VariantZero
cd D:\Projects\VariantZero
git lfs pull
git lfs fsck
```

仓库为私有，需要被授予访问权限的 GitHub 账号。GitHub 的 Download ZIP 不保证包含 LFS 原文件，请优先使用上述克隆方式。不要购买 LFS 额度或启用额外付费；免费额度不足时停止同步并处理容量。

## 经典版

安装 Node.js 22，运行 `npm ci`、`npm run dev`，打开终端提示的本地地址。

## Unreal 工程

安装 Unreal Engine 5.8 及其支持的 Visual Studio C++/Windows SDK 工具链。工程入口是 `UE/VariantZeroUE.uproject`。优先放在英文路径；本机旧脚本可能引用 `D:\Projects\VariantZeroUE`，可将该路径配置为指向克隆目录中 UE 的目录联接，或按新位置调整本地调用。

仓库不含预编译模块和 Windows 试玩包，首次打开需编译；`开始游戏.bat` 依赖本机生成的 BuildOutput，不是刚克隆即可使用的启动器。朋友仅试玩时应使用独立试玩压缩包。

## 版本与素材授权

每次完成游戏更新后按 AGENTS.md 将小版本号加一，并提交代码与对应资源；验证交付后打 `v0.2.N` 标签。此次首次入库保持 0.2.24。

原有 UE/Docs、资源目录中的来源与授权记录继续适用。第三方素材没有因进入私有仓库而变成可公开再分发的素材；不要直接将仓库改为公开。
