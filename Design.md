# MiteEngine架构设计文档

## 目录
1. [MiteEngine架构设计文档](#miteengine架构设计文档)
   1. [目录](#目录)
   2. [整体架构概述](#整体架构概述)
      1. [引擎设计目标](#引擎设计目标)
      2. [引擎设计理念](#引擎设计理念)
      3. [核心架构图](#核心架构图)
   3. [模块职责与技术栈](#模块职责与技术栈)
   4. [Core工具模块](#core工具模块)
      1. [Filesystem文件系统](#filesystem文件系统)
      2. [Logger日志系统](#logger日志系统)
      3. [Thread线程池](#thread线程池)
      4. [Time计时器](#time计时器)
      5. [UUID唯一标识](#uuid唯一标识)
   5. [Event事件模块](#event事件模块)
---

## 整体架构概述
### 引擎设计目标
- 基于OpenGL的场景渲染
- 灵活的扩展性
- 清晰的模块边界

### 引擎设计理念

模块化设计：确保了引擎架构的清晰性和可维护性，为后续的功能扩展奠定基础

- 单一职责原则：每个模块专注于特定功能域，确保清晰的接口边界和职责划分，避免功能重叠和交叉依赖。
- 分层架构设计：Core、Data等作为底层依赖，Light、Material作为中层依赖, Application、UI等作为顶层依赖
- 依赖方向控制：使用CMake的target_link_libraries实现模块之间的依赖方向控制。
- 接口与实现分离：模块通过头文件暴露公共接口，内部实现细节完全封装。

事件驱动架构：事件系统作为引擎最底层依赖，确保模块间松散耦合，提升系统灵活性和可维护性。

- 中心化事件总线：通过EventBus全局单例实现模块间解耦通信
- 统一事件处理：支持同步、异步和延迟事件分发机制


ECS场景管理：ECS架构实现场景对象的灵活组合与高效更新，为复杂场景管理提供清晰的数据组织方式。

- 数据导向设计：Entity作为ID标识，Component存储数据，System处理逻辑
- 组合优于继承：通过组合不同Component构建复杂实体行为
- 缓存友好布局：同类型Component连续存储，提升内存访问效率

渲染管线：采用混合渲染策略，兼顾性能与灵活性，为不同渲染需求提供统一执行框架。

- 多阶段渲染：ShadowMap → G-Buffer → 延迟光照 → 前向渲染
- 数据驱动执行：通过RenderContext传递参数，RenderCommand队列管理绘制命令
- 资源状态隔离：各渲染阶段独立管理帧缓冲和着色器状态

### 核心架构图

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
    
    %% 第八层：依赖最底层和Input
    Window[Window窗口] --> Input
    
    %% 第九层：依赖Renderer和Window
    UI[UI用户界面] --> Renderer
    UI --> Window
    
    %% 最顶层：依赖所有功能模块
    Application[Application应用程序] --> Asset
    Application --> UI

    %% 样式定义
    classDef bottom fill:#0d47a1
    classDef middle fill:#bf360c
    classDef top fill:#9c27b0
    
    class Input,Data,Core,Event bottom
    class Command,Material,Light,Asset,SceneCore,SceneGraph,SceneSerializer,SceneView,Renderer,Window middle
    class UI,Application top
````

## 模块职责与技术栈

| 模块名称 | 核心职责 | 技术栈 | 主要功能 |
|---------|----------|-------------|----------|
| **Application** | 应用生命周期管理 | C++ | 引擎启动/关闭，模块初始化，DEMO场景构建，主循环管理 |
| **Asset** | 资源管理与缓存 | C++, stb_image, assimp, meshoptimizer | 模型/材质/纹理的加载、缓存、生命周期管理、LOD自动生成 |
| **Core** | 基础工具库 | C++, spdlog, std::filesystem, stduuid | 文件系统、日志记录、多线程、计时器、UUID生成 |
| **Data** | 数据封装与着色器管理 | C++, OpenGL, GLM, Shaderc | 几何数据(BV、Mesh)、着色器资源(UBO/SSBO)、实例封装 |
| **Event** | 事件驱动架构核心 | C++, ThreadPool | 事件总线系统，同步/异步/延迟事件分发，模块解耦 |
| **Input** | 输入系统管理 | C++ | 键盘/鼠标事件处理，输入栈管理，输入上下文抽象 |
| **Light** | 光照系统 | C++ | 光源实现(点/方向/聚光)，ShadowMap，光源管理器 |
| **Material** | 材质系统 | C++ | 材质模板抽象，材质工厂，PBR材质实现 |
| **Renderer** | 渲染管线 | C++, OpenGL | 渲染管线(Shadow→GBuffer→延迟→前向)，RenderCommand队列 |
| **SceneCore** | ECS架构核心 | C++ | Entity/Component管理，组件系统，脏组件更新 |
| **SceneGraph** | 场景图与空间加速 | C++ | SceneNode管理，BVH构建，视锥剔除，RayCast |
| **SceneView** | 渲染队列构建 | C++ | RenderableItem构建，RenderQueue填充 |
| **UI** | 用户界面 | C++, Dear ImGui | ViewPort、SceneTree、Properties等界面封装 |
| **Window** | 窗口管理 | C++, GLFW | 窗口创建/管理，窗口事件响应 |

## Core工具模块

### Filesystem文件系统
提供文件系统操作和路径管理功能

功能描述：
- 管理可执行文件路径和资源根目录定位
- 提供资源文件的路径解析和验证
- 实现文件的读写操作和目录创建
- 支持跨平台路径处理

### Logger日志系统
提供分级日志记录系统，支持多输出目标和模块化日志管理

功能描述：
- 多级别日志输出（TRACE / DEBUG / INFO / WARN / ERROR / CRITICAL）
- 控制台彩色输出和文件滚动存储
- 模块专属日志器创建和管理
- 带标签的分类日志记录

### Thread线程池
提供多线程任务调度和并行处理能力，优化计算密集型操作性能

功能描述：
- 线程池管理器统一管理多个专用线程池
- 并行处理工具支持容器元素的批量并行操作
- 基于BS::thread_pool封装，提供类型安全的线程池管理
- 并行工具类简化常见并行模式（ForEach、ForEachIndexed等）
- 自动批处理大小计算和异常安全处理

### Time计时器
提供高精度时间测量和帧率管理，支持帧率无关的动画和物理模拟

功能描述：
- Time类使用静态类模式，管理全局帧时间和增量时间
- Timer类使用实例模式，提供局部计时和性能分析功能
- 支持秒和毫秒两种时间单位输出
- 基于std::chrono高精度时钟，确保时间准确性

### UUID唯一标识
提供全局唯一标识符生成和管理，支持对象标识和序列化

功能描述：
- 生成完全随机UUID用于对象唯一标识
- 支持基于索引和字符串的确定性UUID生成
- 提供UUID与字符串之间的双向转换
- 线程安全的UUID生成机制

模块设计：
- 使用线程局部存储优化随机数生成器性能
- 基于uuids库封装，提供类型安全的UUID操作
- 支持多种生成策略（随机、索引、字符串哈希）
- 轻量级设计，无外部依赖

## Event事件模块