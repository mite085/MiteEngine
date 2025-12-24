# MiteEngine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/compiler_support)
[![CMake](https://img.shields.io/badge/CMake-3.10-064f8c.svg)](https://cmake.org/)
[![OpenGL 4.5+](https://img.shields.io/badge/OpenGL-4.6-orange.svg)](https://www.opengl.org/)

[![Build Status](https://github.com/mite085/MiteEngine/actions/workflows/windows_build.yaml/badge.svg)](https://github.com/mite085/MiteEngine/actions)
[![Build Status](https://github.com/mite085/MiteEngine/actions/workflows/ubuntu_build.yaml/badge.svg)](https://github.com/mite085/MiteEngine/actions)
[![Build Status](https://github.com/mite085/MiteEngine/actions/workflows/macos_build.yaml/badge.svg)](https://github.com/mite085/MiteEngine/actions)

[![Format Check](https://github.com/mite085/MiteEngine/actions/workflows/format-check.yaml/badge.svg)](https://github.com/mite085/MiteEngine/actions)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/yourusername/MiteEngine/actions)

**基于OpenGL的轻量级、模块化的C++三维图形引擎**

## 🚀 快速概览
**MiteEngine** 是一个采用现代C++和OpenGL构建的、面向学习和生产的三维图形引擎。它强调清晰的架构设计、高效的模块化组织和高性能的渲染管线，旨在为图形开发者提供一个易于理解、扩展和维护的现代图形引擎实现范例。
### ✨ 核心亮点
- **🎯 现代架构设计**：融合了**ECS数据层**、**事件驱动**和**模块化分层**等现代软件设计理念，确保代码结构清晰、职责明确。
- **🖼️ 高质量渲染管线**：实现了**PBR材质**、**ShadowMap阴影**，并构建了**ShadowMap-GBuffer-DeferredLighting-Forward-Blend混合渲染管线**，兼顾性能与视觉效果。
- **👨‍💻 开发者友好**：代码注释详尽（注释比例高达**27%**），模块边界清晰，依赖关系严格控制，极大降低了学习和二次开发的门槛。
- **🌍 跨平台支持**：已在 **Windows**、**Ubuntu (Linux)** 和 **macOS** 三大主流桌面平台完成自动化构建与运行测试，确保一致的开发体验。
- **📦 标准资产支持**：完整支持行业标准的 **GLTF 2.0** 模型格式加载，便于集成现有三维资产。
- **🛠️ 集成编辑器**：内置基于 **Dear ImGui** 的编辑器界面，包含视口、场景树、属性面板，支持**中文显示**与**样式切换**，开箱即用。
### 📊 项目规模
- **总代码量**：约**50,000**行
- **核心语言**：**C++**, **GLSL**, **CMake**


## 🎨 功能特性

MiteEngine 集成了现代图形引擎的核心功能，从底层架构到上层渲染，提供了完整的三维图形解决方案。

### 🖌️ 渲染与图形 (Rendering & Graphics)
*   **🔦 混合渲染管线 (Hybrid Render Pipeline)**: 结合了延迟渲染与前向渲染的优势，执行流程为：**ShadowMap → G-Buffer → 延迟光照 (Deferred Lighting) → 前向渲染 (Forward) → 透明混合 (Blend)**。
*   **⚙️ 基于物理的渲染 (PBR)**: 完整实现了 **GLTF 2.0** 标准的基于物理的材质模型，支持金属度/粗糙度工作流。
*   **🌑 动态阴影**: 采用 **Shadow Mapping** 技术，为场景提供真实的动态阴影效果。
*   **🪟 透明度与遮罩**: 完整支持 **Alpha Mask**（遮罩）与 **Alpha Blend**（混合）两种透明物体渲染方式。
*   **🐛 调试视图**: 可实时切换并预览渲染管线的中间结果，包括：
    *   **G-Buffer**：世界坐标 (World Position)、法线 (Normal)、基础色 (Base Color)
    *   延迟光照结果 (Deferred Lighting Output)
    *   前向渲染结果 (Forward Output)
    *   最终混合结果 (Final Blend Output)

### 🏗️ 场景与架构 (Scene & Architecture)
*   **📊 ECS (实体-组件-系统) 架构**: 采用数据驱动的 **ECS** 模式管理场景对象，实现高效的缓存访问和灵活的逻辑组合。
*   **🌳 场景图 (Scene Graph)**: 基于 **SceneNode** 的树状场景结构，管理对象间的空间层次与变换继承关系。
*   **⚡ 空间加速结构**: 集成 **BVH (层次包围体)**，大幅加速视锥剔除 (Frustum Culling) 和光线投射 (Ray Cast) 等空间查询操作。
*   **🧩 高度模块化设计**: 引擎严格遵循单一职责原则，分解为 **15+** 个核心模块（如 Core, Event, Asset, Renderer, SceneCore等），模块间依赖关系清晰可控。

### 🛠️ 编辑器与工具 (Editor & Tooling)
*   **💻 集成可视化编辑器**: 基于 **Dear ImGui** 构建了功能完整的编辑器界面，包含四个核心面板：
    *   **视口面板 (Viewport Panel)**: 3D场景实时渲染与交互。
    *   **场景树面板 (Scene Tree Panel)**: 以树形结构展示和管理场景中的所有实体。
    *   **属性面板 (Properties Panel)**: 查看和编辑选中实体的详细组件属性。
    *   **菜单栏 (MenuBar)**: 提供文件、编辑、视图等操作入口。
*   **🌐 国际化与主题**: 支持 **中文显示**，并可灵活切换不同的 **UI 样式 (Style)**。
*   **🎮 交互式视口**: 支持通过鼠标和键盘进行场景漫游、对象选择与变换。

### 📦 资产与跨平台 (Asset & Cross-Platform)
*   **🗃️ 行业标准资产管道**: 内置完整的 **GLTF 2.0** 加载器，支持模型、网格、材质、纹理的一键导入。
*   **✅ 持续集成与跨平台构建**: 通过 **GitHub Actions** 实现自动化CI/CD，确保在 **Windows (MSVC)**、**Ubuntu (GCC/Clang)** 和 **macOS (Clang)** 上的持续构建与测试通过。
*   **🚦 代码质量保障**: 集成自动化 **Clang-Format** 检查，确保代码风格统一。

### 🧪 演示与测试 (Demos & Testing)
*   提供多个内置的**测试场景**，用于展示引擎的各项功能与渲染效果。
*   引擎本身即是一个可运行的**演示程序**，可直接体验编辑器所有功能。

## 🛠️ 技术栈与统计

MiteEngine 基于现代、稳定且广泛使用的技术栈构建，确保了项目的性能、可维护性和可扩展性。

### 📚 核心技术栈

| 类别 | 技术/库 | 版本/说明 | 主要用途 |
| :--- | :--- | :--- | :--- |
| **语言与标准** | C++ | C++17 | 核心开发语言 |
| **图形API** | OpenGL | 4.6 | 底层图形渲染 |
| **窗口与输入** | GLFW | 3.3 | 跨平台窗口创建与管理 |
| **用户界面** | Dear ImGui | docking分支 | 编辑器GUI实现 |
| **数学库** | GLM | 最新 | 图形数学运算（向量、矩阵） |
| **资产加载** | Assimp | 5.0 | 模型文件（GLTF等）导入 |
| | stb_image | 最新 | 纹理图片加载 |
| | meshoptimizer | 最新 | 网格数据处理与优化 |
| **工具库** | spdlog | 最新 | 高性能日志记录 |
| | stduuid | 最新 | UUID生成 |
| **构建系统** | CMake | 3.10 | 跨平台项目构建与依赖管理 |
| **着色器编译** | Shaderc | （集成） | GLSL着色器离线编译与优化 |

### 📊 代码规模与质量
项目总规模约 **31,000+** 行有效代码，体现了完整的引擎实现复杂度。详细的代码统计如下（使用 `cloc` 生成）：
| 语言 | 文件数 | 代码行 | 注释行 | 空白行 | 总计 | 注释比例 |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **C++源文件** | 141 | 19,035 | 3,304 | 3,510 | 25,849 | 14.8% |
| **C++头文件** | 174 | 9,484 | 8,019 | 2,457 | 19,960 | 45.8% |
| **GLSL着色器** | 22 | 1,310 | 709 | 443 | 2,462 | 35.1% |
| **CMake脚本** | 29 | 1,367 | 289 | 230 | 1,886 | 17.4% |
| **总计** | **366** | **31,196** | **12,321** | **6,640** | **50,157** | **28.3%** |

**关键指标解读：**
- **卓越的文档化**：整体注释比例高达 **~28.3%**，其中**头文件注释比例接近 46%**。接口定义、类说明、关键算法都有详尽的文档，极大降低了学习成本和维护难度。
- **完整的图形管线**：**22个GLSL着色器文件**覆盖了从阴影、几何缓冲到光照计算的全套渲染阶段，每个着色器都有详细的注释说明。
- **专业的工程管理**：**29个CMake脚本**文件确保了跨平台构建的可靠性和可重复性。
### 🔗 第三方依赖管理
- 主要第三方库均通过 **Git Submodules** 集成，确保版本可控和构建一致性。
- CMake脚本自动处理依赖的查找、编译与链接，用户只需一条命令即可完成环境准备与项目构建。

## 🏗️ 架构展示

MiteEngine 采用精心设计的模块化分层架构，各模块职责清晰、依赖关系严格，确保了系统的高内聚、低耦合和良好的可扩展性。

### 📐 核心架构图

以下图表清晰地展示了引擎各模块之间的分层依赖关系：

```mermaid
graph TD
    %% 最底层模块
    Core[Core工具]
    Event[Event事件]
    
    %% 第二层：仅依赖最底层
    Input[Input输入] --> Core
    Input --> Event
    Data[Data数据] --> Core
    Data --> Event
    
    %% 第三层：依赖Data和最底层
    Material[Material材质] --> Data
    Light[Light光照] --> Data
    Shader[Shader着色器] --> Data
    
    %% 第四层：依赖Material/Light和SceneCore
    Asset[Asset资产] --> Data
    Asset --> Material
    SceneCore[SceneCore场景核心] --> Material
    SceneCore --> Data
    SceneCore --> Light
    
    %% 第五层：依赖SceneCore
    SceneGraph[SceneGraph场景图] --> SceneCore
    
    %% 第六层：依赖SceneGraph和SceneCore
    SceneView[SceneView场景视图] --> SceneGraph
    
    %% 第七层：依赖SceneView
    Renderer[Renderer渲染] --> SceneView
    Renderer --> Shader
    
    %% 第八层：依赖最底层和Input
    Window[Window窗口] --> Input
    
    %% 第九层：依赖Renderer和Window
    UI[UI用户界面] --> Renderer
    UI --> Window
    
    %% 最顶层：依赖所有功能模块
    Application[Application应用程序] --> Asset
    Application --> UI

    %% 样式定义
    classDef bottom fill: #bbdefb
    classDef middle fill: #ffe0b2
    classDef top fill: #e1bee7
    
    class Input,Data,Core,Event,Material,Light,Shader bottom
    class Asset,SceneCore,SceneGraph,SceneView,Renderer,Window middle
    class UI,Application top
````

架构分层解读：
- 基础层 (🔵蓝色): Core, Event, Data, Input 等提供最基础的运行时支持。
- 核心层 (🟠橙色): Asset, SceneCore, Renderer 等实现引擎的核心数据管理与渲染逻辑。
- 应用层 (🟣紫色): UI, Application 构建最终的用户界面和应用程序。

### 🔄 混合渲染管线流程

引擎的渲染管线是现代混合式架构，结合了延迟渲染和前向渲染的优势：
```mermaid
graph LR
    A[场景数据] --> B(ShadowMap Pass<br/>阴影贴图生成);
    B --> C(GBuffer Pass<br/>几何缓冲填充);
    C --> D(Deferred Lighting Pass<br/>延迟光照计算);
    D --> E(Forward Pass<br/>前向渲染透明/特殊物体);
    E --> F(Blend/Post-Process Pass<br/>混合与后处理);
    F --> G[最终画面];
    
    style B fill:#ffebee
    style C fill:#e8f5e8
    style D fill:#e3f2fd
    style E fill:#fff3e0
    style F fill:#f3e5f5
````

管线阶段说明：
- ShadowMap Pass: 从光源视角渲染深度图，用于后续阴影计算。
- G-Buffer Pass: 将场景的几何信息（位置、法线、材质参数等）渲染到多个渲染目标（MRT）中。
- Deferred Lighting Pass: 利用G-Buffer中的数据，在屏幕空间中进行高效的光照计算。
- Forward Pass: 渲染透明物体、UI等不适合延迟渲染的对象。
- Blend Pass: 混合所有渲染结果（并可在此阶段加入后处理效果）。

### 📦 关键模块职责速览
| 模块 | 核心职责 | 关键设计 |
| :--- | :--- | :--- |
| **`SceneCore`** | ECS架构核心 | Entity/Component注册表，组件系统管理器，脏标记更新 |
| **`SceneGraph`** | 空间结构与加速 | SceneNode树，BVH加速结构，视锥剔除，射线检测 |
| **`Renderer`** | 渲染命令执行 | 多阶段渲染管线，RenderCommand队列，渲染状态管理 |
| **`Asset`** | 资源生命周期 | GLTF加载，纹理/材质缓存，LOD自动生成 |
| **`Event`** | 模块间通信 | 中心化事件总线，支持同步/异步/延迟事件分发 |

### 🎯 设计理念总结
- 模块化与分层: 严格遵循单一职责原则，通过CMake的target_link_libraries控制依赖方向。
- 数据驱动: ECS架构使游戏逻辑与数据分离，提升缓存友好性和系统灵活性。
- 事件驱动: 通过EventBus实现模块间解耦，提高代码的可测试性和可维护性。
- 混合渲染: 结合延迟渲染的效率与前向渲染的灵活性，适应复杂的渲染需求。
- 
此架构为引擎的稳定性、性能优化和未来功能扩展（如Vulkan后端、新的渲染特性）奠定了基础。