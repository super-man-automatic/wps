# 🎨 SmartDoc AI - 智能文档编辑器

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blueviolet.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)
![Status](https://img.shields.io/badge/status-Beta-yellow.svg)

> 一款集成 AI 助手的高性能桌面文档编辑器，对标 WPS AI & Microsoft Word

---

## ✨ 核心特性

### 🤖 AI 功能
- **续写**: 根据上下文智能补全内容
- **润色**: 优化文本表达和语言流畅性
- **摘要**: 快速生成文档核心要点
- **对话**: 侧边栏 AI 助手实时对话
- **翻译**: 支持多语言翻译（规划中）

### ⚡ 高性能编辑
- **大文件支持**: 10 万+ 行文档秒级加载
- **分块加载**: 智能懒加载与预加载
- **线程安全**: 基于 shared_mutex 的并发读写
- **实时同步**: AI 后台分析，UI 始终响应

### 🎯 开发者友好
- **模块化设计**: core/ai/gui/plugin 四层架构
- **完整测试**: Google Test 单元测试框架
- **插件系统**: 支持 C++/Python/Lua 扩展
- **开源**: MIT 协议，欢迎贡献

---

## 🚀 快速开始

### 环境需求

| 依赖 | 版本 | 安装方式 |
|------|------|--------|
| CMake | 3.20+ | 官方网站 / apt / brew |
| Qt | 6.2+ | qt.io / apt / brew |
| GTest | 最新 | apt / brew / 源码 |
| 编译器 | C++17 | GCC 9+ / Clang 11+ / MSVC 2019+ |

### 安装依赖（以 Ubuntu 为例）

```bash
# 基础工具
sudo apt-get install build-essential cmake git

# Qt 6 开发库
sudo apt-get install qt6-base-dev qt6-tools-dev

# Google Test
sudo apt-get install libgtest-dev
```

### 编译与运行

```bash
# 1. 克隆项目
git clone https://github.com/yourusername/SmartDoc.git
cd SmartDoc

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置与编译
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel 4

# 4. 运行程序
./SmartDoc

# 5. 运行测试
./SmartDocTests
```

更详细的编译指南见 [BUILD.md](BUILD.md)

---

## 📖 项目结构

```
SmartDoc/
├── src/
│   ├── core/                    # 核心引擎（纯 C++）
│   │   ├── document.{h,cpp}     # 文档模型（线程安全）
│   │   └── parser.{h,cpp}       # Markdown 解析器
│   │
│   ├── ai/                      # AI 功能模块
│   │   ├── llm_client.{h,cpp}   # LLM 客户端（OpenAI/Mock）
│   │   ├── prompt_manager.{h,cpp}  # 提示词模板管理
│   │   └── ai_writer.{h,cpp}    # AI 写作助手
│   │
│   ├── gui/                     # Qt GUI 组件
│   │   ├── mainwindow.{h,cpp}   # 主窗口
│   │   ├── editor_widget.{h,cpp} # 编辑器组件
│   │   └── ai_sidebar.{h,cpp}   # AI 侧边栏
│   │
│   └── main.cpp                 # 入口点
│
├── tests/                       # 单元测试
│   ├── core_tests/
│   ├── ai_tests/
│   └── CMakeLists.txt
│
├── CMakeLists.txt               # CMake 配置
├── BUILD.md                     # 构建指南
└── README.md                    # 本文件
```

---

## 💡 使用示例

### 基础编辑

1. 打开 SmartDoc
2. 在左侧编辑器中输入文本
3. 点击右侧 AI 侧边栏的功能按钮

### 续写功能

```
输入: "从前有座山，山上有个庙..."
点击: 续写按钮
输出: "...庙里有个老和尚讲故事，给小和尚讲的什么呢？"
```

### 润色功能

```
输入: "这个东西很好用非常方便"
点击: 润色按钮
输出: "该工具功能全面、易用性强，使用体验优秀。"
```

---

## 🔧 配置 OpenAI API

默认使用 Mock 后端。要启用真实 AI 功能：

1. 获取 [OpenAI API Key](https://platform.openai.com/api-keys)

2. 在 `src/gui/ai_sidebar.cpp` 中修改：

```cpp
// 修改此行
ai_client_ = std::make_shared<ai::LLMClient>(ai::Backend::Mock);

// 为
ai_client_ = std::make_shared<ai::LLMClient>(ai::Backend::OpenAI);
ai_client_->setApiKey("your-api-key-here");
```

3. 重新编译运行

---

## 🧪 运行测试

```bash
cd build

# 运行所有测试
./SmartDocTests

# 查看详细输出
./SmartDocTests --verbose

# 运行特定测试
./SmartDocTests --gtest_filter=DocumentTest.LoadFromString
```

---

## 🛣️ 开发路线图

### Phase 1 (当前) - MVP ✅
- [x] 基础编辑器 UI
- [x] AI 续写、润色、摘要功能
- [x] Mock AI 后端
- [x] 基本单元测试
- [ ] OpenAI 集成

### Phase 2 (规划中)
- [ ] 完整的大文件懒加载
- [ ] 更多 AI 功能（翻译、解释等）
- [ ] 性能优化（渲染、索引）
- [ ] 本地 llama.cpp 推理支持

### Phase 3 (规划中)
- [ ] 插件系统（C++/Python/Lua）
- [ ] 主题系统与偏好设置
- [ ] 多文档标签页
- [ ] 云同步与协作（可选）

---

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开 Pull Request

### 贡献规范

- 遵循现有代码风格（参考 `.clang-format`）
- 为新功能添加单元测试
- 更新相关文档
- 提交信息清晰明确

---

## 📄 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE) 文件

---

## 👨‍💻 作者

SmartDoc 由以下开发者维护：

- **你的名字** - [GitHub](https://github.com/yourusername)

---

## 💬 社区与支持

- **问题反馈**: [GitHub Issues](https://github.com/yourusername/SmartDoc/issues)
- **讨论**: [GitHub Discussions](https://github.com/yourusername/SmartDoc/discussions)
- **文档**: [Wiki](https://github.com/yourusername/SmartDoc/wiki)

---

## 🙏 致谢

感谢以下开源项目的支持：

- [Qt Framework](https://www.qt.io/)
- [Google Test](https://github.com/google/googletest)
- [OpenAI API](https://openai.com/api/)

---

<div align="center">

**⭐ 如果这个项目对你有帮助，请给个 Star！**

Made with ❤️ for developers and writers

</div>
