# SmartDoc AI 编辑器 - 构建指南（WPS 项目）

## 📋 环境要求（已配置）

✅ **编译器**: MSVC 2022 (Visual Studio 2022)  
✅ **C++ 标准**: C++17  
✅ **CMake**: 3.20+  
✅ **Qt 6**: 6.11.0 (msvc2022_64)  
✅ **构建工具**: Ninja  
✅ **Google Test**: 已集成  

---

## 🚀 快速开始（Windows + MSVC）

### 环境检查

确保以下工具已安装：
- ✅ Visual Studio 2022 或 Build Tools for Visual Studio 2022
- ✅ Qt 6.11.0 for MSVC 2022 64-bit (在 `C:/Qt/6.11.0/msvc2022_64`)
- ✅ CMake 3.20+
- ✅ Ninja 生成器

### 步骤1: 进入项目目录

```bash
cd C:\Users\YourUsername\source\repos\wps
# 或你实际的 wps 项目路径
```

### 步骤2: 使用 CMake 预设配置

选择一个预设进行配置：

**调试版本（Debug）:**
```bash
cmake --preset Qt-Debug
```

**发布版本（Release）:**
```bash
cmake --preset Qt-Release
```

> 预设已在 `CMakeUserPresets.json` 中定义，自动使用 Qt 6.11.0 + Ninja

### 步骤3: 编译

```bash
# 从调试预设编译
cmake --build --preset Qt-Debug

# 或从发布预设编译
cmake --build --preset Qt-Release
```

**或手动编译:**
```bash
# 调试版
cmake --build out/build/debug --config Debug --parallel 4

# 发布版
cmake --build out/build/release --config Release --parallel 4
```

### 步骤4: 运行程序

```bash
# Debug 版本
./out/build/debug/SmartDoc.exe

# Release 版本
./out/build/release/SmartDoc.exe
```

### 步骤5: 运行测试

```bash
# Debug 测试
./out/build/debug/SmartDocTests.exe

# Release 测试
./out/build/release/SmartDocTests.exe
```

---

## 🧪 Windows MSVC 故障排除

### ❌ 找不到 Qt 6

**错误信息**: `CMake Error: Could not find Qt6`

**解决方案:**

1. **验证 Qt 路径**
   ```bash
   dir "C:\Qt\6.11.0\msvc2022_64\bin\qmake.exe"
   ```

2. **使用 CMake 预设** (推荐，自动配置)
   ```bash
   cmake --preset Qt-Debug
   ```

3. **手动设置 Qt 路径** (如 Qt 位置不同)
   ```bash
   cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.11.0\msvc2022_64" ..
   ```

### ❌ Ninja 生成器找不到

**错误信息**: `CMake Error: Ninja not found`

**解决方案:**
```bash
# 添加 Ninja 到环境变量，或使用 Visual Studio 生成器
cmake -G "Visual Studio 17 2022" ..
```

### ❌ 链接错误 (LNK2019 / LNK1104)

**原因**: Debug/Release 模式不匹配或库文件缺失

**解决方案:**
```bash
# 清理旧的构建
rmdir /s out

# 重新配置和编译
cmake --preset Qt-Debug
cmake --build --preset Qt-Debug --verbose
```

---

## 📦 发布构建

```bash
# Release 版本构建和安装
cmake --preset Qt-Release
cmake --build --preset Qt-Release
```

---

## ✅ 验证安装

成功编译后，执行程序：

**Windows 命令行:**
```bash
.\out\build\release\SmartDoc.exe
```

**PowerShell:**
```powershell
.\out\build\release\SmartDoc.exe
```

首次运行你会看到：
- ✅ SmartDoc 窗口打开（左侧编辑器，右侧 AI 侧边栏）
- ✅ 3 个快速按钮：续写、润色、摘要
- ✅ 可输入文本，点击按钮获取 Mock AI 响应
- ✅ 菜单栏：新建、打开、保存、关于

---

## 🔗 常见 PowerShell 命令

```powershell
# 进入项目目录
cd C:\Users\YourUsername\source\repos\wps

# 使用预设编译
cmake --preset Qt-Debug
cmake --build --preset Qt-Debug

# 或手动配置
cmake -B out/build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:\Qt\6.11.0\msvc2022_64"
cmake --build out/build/debug --config Debug --parallel 4

# 清理构建
rmdir out -Recurse -Force

# 运行程序
.\out\build\debug\SmartDoc.exe

# 运行测试
.\out\build\debug\SmartDocTests.exe

# 查看编译详情
cmake --build out/build/debug --verbose
```

---

祝你编译顺利！🎉

