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
      1. [模块概述](#模块概述)
      2. [设计理念](#设计理念)
      3. [事件处理模式](#事件处理模式)
      4. [事件处理流程](#事件处理流程)
      5. [核心组件](#核心组件)
   6. [Data数据模块](#data数据模块)
      1. [Transform变换](#transform变换)
      2. [Camera相机](#camera相机)
      3. [Model模型/Mesh网格体](#model模型mesh网格体)
      4. [Bounding Volume包围盒](#bounding-volume包围盒)
      5. [Frustum视锥体](#frustum视锥体)
      6. [Ray射线](#ray射线)
      7. [Runtime Texture运行时纹理对象](#runtime-texture运行时纹理对象)
   7. [Shader着色器模块](#shader着色器模块)
      1. [Framebuffer 帧缓冲 \& G-Buffer 几何缓冲](#framebuffer-帧缓冲--g-buffer-几何缓冲)
      2. [Shader 着色器 \& Shader Cache 着色器缓存机制](#shader-着色器--shader-cache-着色器缓存机制)
      3. [Unifrom Buffer 统一缓冲区](#unifrom-buffer-统一缓冲区)
      4. [Shader Storage Buffer 存储缓冲区](#shader-storage-buffer-存储缓冲区)
      5. [Binding Point Manager 绑定点管理](#binding-point-manager-绑定点管理)
   8. [Asset资产模块](#asset资产模块)
      1. [Texture Loader纹理加载器](#texture-loader纹理加载器)
      2. [Material Loader材质加载器](#material-loader材质加载器)
      3. [Model Loader模型加载器](#model-loader模型加载器)
      4. [Asset Cache资产缓存](#asset-cache资产缓存)
      5. [Asset Manager资产管理器](#asset-manager资产管理器)
   9. [Material材质模块](#material材质模块)
      1. [Material Param Variant材质可变参数](#material-param-variant材质可变参数)
      2. [Material Instance材质实例](#material-instance材质实例)
      3. [Material Template材质模板](#material-template材质模板)
      4. [GLTF PBR Material基于物理的材质](#gltf-pbr-material基于物理的材质)
      5. [Material Factory材质工厂](#material-factory材质工厂)
   10. [Light光照模块](#light光照模块)
       1. [Light光源抽象](#light光源抽象)
       2. [Shadow Map阴影贴图](#shadow-map阴影贴图)
       3. [Point Light点光源](#point-light点光源)
       4. [Directional Light方向光](#directional-light方向光)
       5. [Spot Light聚光灯](#spot-light聚光灯)
       6. [Light Manager光源管理器](#light-manager光源管理器)
   11. [Input 输入模块](#input-输入模块)
       1. [Input Manager输入管理器](#input-manager输入管理器)
       2. [Input Context/Stack 输入上下文/上下文栈](#input-contextstack-输入上下文上下文栈)
       3. [Input State Tracker输入状态跟踪器](#input-state-tracker输入状态跟踪器)
   12. [Scene Core场景核心模块](#scene-core场景核心模块)
       1. [Component组件基类](#component组件基类)
       2. [Entity 场景实体](#entity-场景实体)
       3. [Scene Registry场景注册表](#scene-registry场景注册表)
       4. [Event Publisher组件事件发布器](#event-publisher组件事件发布器)
       5. [Snapshot组件状态快照](#snapshot组件状态快照)
       6. [Component System组件系统](#component-system组件系统)
       7. [Component System Manager组件系统管理器](#component-system-manager组件系统管理器)
       8. [Scene Core Components组件具体实现](#scene-core-components组件具体实现)
       9. [Scene Core对外接口](#scene-core对外接口)
   13. [Scene Graph场景图模块](#scene-graph场景图模块)
       1. [Scene Node场景节点/Manager场景节点管理器](#scene-node场景节点manager场景节点管理器)
       2. [Spatiral Partition空间划分](#spatiral-partition空间划分)
       3. [Bounding volume hierarchy层次包围体](#bounding-volume-hierarchy层次包围体)
       4. [Scene Graph场景图接口设计](#scene-graph场景图接口设计)
   14. [Scene View场景视图模块](#scene-view场景视图模块)
       1. [Renderable Item可渲染项/Builder可渲染项构建器](#renderable-item可渲染项builder可渲染项构建器)
       2. [Render Queue渲染队列](#render-queue渲染队列)
       3. [Scene View场景视图接口设计](#scene-view场景视图接口设计)
   15. [Render 渲染模块](#render-渲染模块)
       1. [Render Command渲染命令](#render-command渲染命令)
       2. [Render Device渲染设备](#render-device渲染设备)
       3. [Render Context渲染上下文](#render-context渲染上下文)
       4. [Render Pipeline渲染管线架构](#render-pipeline渲染管线架构)
       5. [ShadowMap Stage阴影贴图阶段](#shadowmap-stage阴影贴图阶段)
       6. [G-Buffer Stage几何缓冲阶段](#g-buffer-stage几何缓冲阶段)
       7. [Deferred Lighting Stage延迟光照阶段](#deferred-lighting-stage延迟光照阶段)
       8. [Forward Stage前向渲染阶段](#forward-stage前向渲染阶段)
---

## 整体架构概述
### 引擎设计目标
- 基于OpenGL的场景渲染
- 灵活的扩展性
- 清晰的模块边界

### 引擎设计理念

**模块化设计**：确保了引擎架构的清晰性和可维护性，为后续的功能扩展奠定基础

- 单一职责原则：每个模块专注于特定功能域，确保清晰的接口边界和职责划分，避免功能重叠和交叉依赖。
- 分层架构设计：Core、Data等作为底层依赖，Light、Material作为中层依赖, Application、UI等作为顶层依赖
- 依赖方向控制：使用CMake的target_link_libraries实现模块之间的依赖方向控制。
- 接口与实现分离：模块通过头文件暴露公共接口，内部实现细节完全封装。

**事件驱动架构**：事件系统作为引擎最底层依赖，确保模块间松散耦合，提升系统灵活性和可维护性。

- 中心化事件总线：通过EventBus全局单例实现模块间解耦通信
- 统一事件处理：支持同步、异步和延迟事件分发机制


**ECS场景管理**：ECS架构实现场景对象的灵活组合与高效更新，为复杂场景管理提供清晰的数据组织方式。

- 数据导向设计：Entity作为ID标识，Component存储数据，System处理逻辑
- 组合优于继承：通过组合不同Component构建复杂实体行为
- 缓存友好布局：同类型Component连续存储，提升内存访问效率

**渲染管线**：采用混合渲染策略，兼顾性能与灵活性，为不同渲染需求提供统一执行框架。

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
    class Asset,SceneCore,SceneGraph,SceneSerializer,SceneView,Renderer,Window middle
    class UI,Application top
````

## 模块职责与技术栈

| 模块名称 | 核心职责 | 技术栈 | 主要功能 |
|---------|----------|-------------|----------|
| **Application** | 应用生命周期管理 | C++ | 引擎启动/关闭，模块初始化，DEMO场景构建，主循环管理 |
| **Asset** | 资源管理与缓存 | C++, stb_image, assimp, meshoptimizer | 模型/材质/纹理的加载、缓存、生命周期管理、LOD自动生成 |
| **Core** | 基础工具库 | C++, spdlog, std::filesystem, stduuid | 文件系统、日志记录、多线程、计时器、UUID生成 |
| **Data** | 数据封装 | C++, GLM | 几何数据(BV、Mesh)、实例封装 |
| **Shader** | 着色器管理 | C++, OpenGL, Shaderc | 着色器资源(FBO/UBO/SSBO)|
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

**功能描述**：
- 管理可执行文件路径和资源根目录定位
- 提供资源文件的路径解析和验证
- 实现文件的读写操作和目录创建
- 支持跨平台路径处理

### Logger日志系统
提供分级日志记录系统，支持多输出目标和模块化日志管理

**功能描述**：
- 多级别日志输出（TRACE / DEBUG / INFO / WARN / ERROR / CRITICAL）
- 控制台彩色输出和文件滚动存储
- 模块专属日志器创建和管理
- 带标签的分类日志记录

### Thread线程池
提供多线程任务调度和并行处理能力，优化计算密集型操作性能

**功能描述**：
- 线程池管理器统一管理多个专用线程池
- 并行处理工具支持容器元素的批量并行操作
- 基于BS::thread_pool封装，提供类型安全的线程池管理
- 并行工具类简化常见并行模式（ForEach、ForEachIndexed等）
- 自动批处理大小计算和异常安全处理

### Time计时器
提供高精度时间测量和帧率管理，支持帧率无关的动画和物理模拟

**功能描述**：
- Time类使用静态类模式，管理全局帧时间和增量时间
- Timer类使用实例模式，提供局部计时和性能分析功能
- 支持秒和毫秒两种时间单位输出
- 基于std::chrono高精度时钟，确保时间准确性

### UUID唯一标识
提供全局唯一标识符生成和管理，支持对象标识和序列化

**功能描述**：
- 生成完全随机UUID用于对象唯一标识
- 支持基于索引和字符串的确定性UUID生成
- 提供UUID与字符串之间的双向转换
- 线程安全的UUID生成机制
- 支持多种生成策略（随机、索引、字符串哈希）

## Event事件模块

### 模块概述
Event模块是MiteEngine的核心通信枢纽，采用**事件总线模式**实现模块间的松耦合通信。该模块提供了灵活的事件订阅-生产-分发机制，支持同步、异步和延迟事件处理，确保系统各组件间的高效、安全通信。

### 设计理念
- 中心化事件总线：通过EventBus全局单例统一管理所有事件通信
- 类型安全分发：利用C++模板和RTTI确保事件类型安全
- 多模式处理：支持同步、异步、延迟三种事件处理模式
- 优先级控制：提供五级事件处理优先级机制（仅在延迟事件处理阶段生效）
- RAII管理：通过SubscriptionGroup自动管理订阅生命周期

### 事件处理模式

| 处理模式 | 执行时机 | 适用场景 |
|---------|----------|----------|
| **同步** | 立即执行 | 实时响应、UI更新 |
| **异步** | 线程池执行 | 计算密集型任务 |
| **延迟** | 下一帧处理 | 帧末处理、状态同步 |

### 事件处理流程
```mermaid
flowchart TD
    A[事件发布 Post] --> B[复制订阅者列表<br/>带锁操作]
    B --> C[分类订阅者<br/>同步/异步/延迟]
    
    C --> D[同步处理]
    C --> E[异步处理]
    C --> F[延迟同步处理]
    C --> G[延迟异步处理]
    
    D --> H[立即在主线程执行]
    E --> I[提交到线程池执行]
    F --> J[加入延迟队列]
    G --> J
    
    J --> K[ProcessQueue调用]
    K --> L{延迟事件类型?}
    L -->|同步| M[主线程执行]
    L -->|异步| N[线程池执行]
    
    M --> O[事件处理完成]
    N --> O
    H --> O
    I --> O
````

### 核心组件
**事件基类** (Event)：作为所有事件的基类，定义了统一的事件接口和传播控制机制。

**关键特性**：
- 事件传播控制：通过EventResult枚举控制事件传播行为
- 类别系统：支持事件类别掩码，便于批量订阅
- 克隆能力：支持事件对象深拷贝，用于异步处理

**事件总线** (EventBus)：系统的核心通信枢纽，管理所有事件的订阅和分发。

```mermaid
sequenceDiagram
    participant P as 发布者
    participant EB as EventBus
    participant S as 订阅者
    participant TP as 线程池
    
    P->>EB: Post(事件)
    EB->>EB: 分类订阅者
    
    alt 同步处理（立即执行--实时响应、UI更新）
        EB->>S: 立即调用
    else 异步处理（线程池执行--计算密集型任务）  
        EB->>TP: 提交任务
        TP->>S: 子线程处理
    else 延迟处理（下一帧处理--帧末处理、状态同步）
        EB->>EB: 存储到队列
        EB->>S: ProcessQueue时调用
    end
````

**订阅组** (SubscriptionGroup)：提供RAII风格的事件订阅管理，简化订阅生命周期管理。

**基本事件订阅模式**：

```cpp
// 定义事件
class WindowResizeEvent : public Event {
    EVENT_CLASS_CATEGORY(EventCategory::EVENT_CATEGORY_WINDOW)
};
// 订阅事件
m_Subscriptions.SubscribeImmediate<WindowResizeEvent>(
    BIND_DISPATCH_FN(OnWindowResized)
);
// 发布事件
WindowResizeEvent event(1920, 1080);
EventBus::Publish(event);
````

**异步/延迟/类别事件订阅模式**：

```cpp
// 异步处理计算密集型事件
m_Subscriptions.SubscribeAsync<MeshProcessingEvent>(
    BIND_DISPATCH_FN(ProcessMeshAsync),
    EventPriority::High
);
// 延迟处理帧末任务  
m_Subscriptions.SubscribeDeferred<FrameEndEvent>(
    BIND_DISPATCH_FN(CleanupFrameResources)
);
// 类别订阅处理所有类别为“输入”的事件
m_Subscriptions.SubscribeByCategoryImmediate(
    EventCategory::EVENT_CATEGORY_INPUT,
    [this](Event& e) { ProcessInputEvent(e); }
);
````

Event模块作为引擎最底层的基础设施，为整个系统提供了高效、灵活、安全的通信机制，是实现模块化架构和松耦合设计的关键支撑。

## Data数据模块

Data数据模块是MiteEngine的数据核心层，基于GLM数学库开发，负责管理所有渲染相关的数据类型、GPU资源设施。作为引擎的第二层基础模块，它为上层渲染系统提供统一的数据抽象和资源管理。

### Transform变换
Transform系统是MiteEngine的数学基础组件，提供统一的3D空间变换管理。作为纯数学工具类，为所有需要空间变换的组件提供底层支持。

**设计理念**
- 右手坐标系：遵循OpenGL标准，Y轴向上，Z轴向前
- 双旋转表示：内部四元数存储，外部欧拉角接口
- 惰性计算：矩阵缓存与脏标记优化
- 相机友好：提供专用的视图矩阵和相机控制接口

**符合OpenGL默认的坐标系规范**
```cpp
// OpenGL标准右手坐标系定义
static const glm::vec3 s_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);      // +Y 向上
static const glm::vec3 s_WorldForward = glm::vec3(0.0f, 0.0f, -1.0f); // -Z 向前
````

**变换属性管理**

| 属性 | 内部存储 | 外部接口 | 默认值 |
|------|----------|----------|--------|
| **位置** | `glm::vec3` | 直接访问 | `(0,0,0)` |
| **旋转** | 四元数 + 欧拉角 | 度制欧拉角 | 单位四元数 |
| **缩放** | `glm::vec3` | 直接访问 | `(1,1,1)` |
| **旋转顺序** | `EulerOrder` | 枚举选择 | `YXZ` |

**矩阵缓存的惰性计算机制**
```mermaid
flowchart TD
    subgraph TransformSet [设置操作 SET]
        A1[Set Position/Rotation/Scale] --> A2[更新对应属性值]
        A2 --> A3[标记脏标志<br/>m_MatrixDirty = true]
        
        B1[Set Rotation] --> B2[更新欧拉角]
        B2 --> B3[立即更新四元数<br/>UpdateRotationFromEuler]
        B3 --> A3
        
        C1[SetRotationQuat] --> C2[更新四元数]
        C2 --> C3[标记旋转脏标志<br/>m_RotationDirty = true]
        C3 --> A3
    end
    
````
```mermaid
flowchart TD
    subgraph TransformGet [获取操作 GET]
        D1[Get Matrix/Position/Scale] --> D2{检查矩阵脏标志<br/>m_MatrixDirty?}
        D2 -->|是| D3[更新局部矩阵<br/>UpdateLocalMatrix]
        D3 --> D4[清除矩阵脏标志]
        D4 --> D5[返回缓存值]
        D2 -->|否| D5
        
        E1[Get Rotation] --> E2{检查旋转脏标志<br/>m_RotationDirty?}
        E2 -->|是| E3[更新欧拉角<br/>UpdateEulerFromRotation]
        E3 --> E4[清除旋转脏标志]
        E4 --> E5[返回欧拉角]
        E2 -->|否| E5
    end
````

**双旋转表示**
```cpp
// 内部：四元数存储（计算高效，无万向节锁）
mutable glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
// 外部：欧拉角接口（用户友好）
mutable glm::vec3 m_RotationEuler = glm::vec3(0.0f);
````

**相机控制系统（防翻滚旋转）**：确保相机的Up朝向始终和WorldUp保持一致

| 向量类型 | 计算方式 | 用途 |
|----------|----------|------|
| `GetConstrainedUp()` | `s_WorldUp(0,1,0)` | 朝上方向不变 |
| `GetForward()` | 四元数旋转 `s_WorldForward(0,0,-1)` | 基础前向 |
| `GetConstrainedRight()` | `cross(forward, constrained up)` | 防翻滚右向 |
| `GetConstrainedForward()` | `cross(worldUp, constrained right)` | 约束前向 |


### Camera相机

Camera系统是MiteEngine的视图管理核心，基于GLM数学库开发，负责投影矩阵计算和相机参数管理。与Transform系统协同工作，共同完成的3D视图解算功能。

**设计理念**
- 投影与视图分离：Camera专注投影矩阵，Transform负责视图矩阵
- 双投影支持：透视与正交投影模式
- 惰性计算：投影矩阵缓存与脏标记优化

**投影矩阵的惰性计算机制**(Transform惰性计算机制的简化版)
```mermaid
flowchart TD
   subgraph CameraSet [设置操作 SET]
        A[设置相机参数] --> B[标记投影脏标志]
        B --> C[参数验证与钳制]
    end
    
````
```mermaid
flowchart TD
    subgraph CameraGet [获取操作 GET]
        D[获取投影矩阵] --> E[检查脏标志]
        E -->|干净| F[返回缓存矩阵]
        E -->|脏| G{投影类型判断}
        
        G -->|透视| H[计算透视矩阵]
        G -->|正交| I[计算正交矩阵]
        
        H --> J[短边FOV自适应]
        I --> K[基于正交尺寸计算]
        
        J --> L[清除脏标志]
        K --> L
        L --> F
    end
````
**相机控制接口**：统一的缩放控制接口Zoom()，自动适配投影类型：若为透视相机，则改变视场角；若为正交相机，则调整视口大小。

**短边FOV自适应算法**：仿照Blender设计，确保视野范围始终充足的同时，适应各种屏幕比例

当宽高比 $Aspect\geq1.0$ 时（宽屏），直接使用垂直的FOV进行计算（缓存的FOV为垂直FOV）：

$$
ProjectionMatrix = \text{perspective}\left( \text{radians}(FOV),\ Aspect,\ Near,\ Far \right)
$$

当宽高比 $Aspect<1.0$ 时（竖屏），先根据缓存的垂直FOV计算水平方向的视场角，再将其作为输入参数交给 $\text{perspective}$ 函数：

$$
horizontalFOV = 2\cdot\arctan\left(\frac{\tan\left(\frac{\text{radians}(FOV)}{2}\right)}{Aspect}\right)
$$

$$
ProjectionMatrix=\text{perspective}\left(horizontalFOV,Aspect,Near,Far\right)
$$

其中GLM提供的 $\text{perspective}$ 函数使用垂直方向的视场角 $FOV$（以弧度为单位）。

### Model模型/Mesh网格体
Mesh和Model系统是MiteEngine的几何数据管理层，负责组织和管理3D模型的网格数据和材质关联。采用分层设计，支持多子网格和多LOD级别。

**设计理念**
- 资源引用模式：Mesh引用父Model的GPU资源
- LOD链式管理：统一的基础LOD + 可选细节级别
- 材质关联：网格与材质实例的自动绑定
- 包围盒层次：模型级和网格级包围盒支持

**层次结构关系**
```mermaid
graph TD
    A[Model模型] --> B[Mesh子网格1]
    A --> C[Mesh子网格2]
    
    B --> E[LOD链管理]
    C --> F[LOD链管理]
    
    E --> H[基础LOD]
    E --> I[LOD级别123…]
    
    A --> K[材质实例数组]
    B --> L[材质索引引用]
    C --> J[材质索引引用]
````

**安全访问机制**
- 边界检查：所有索引访问都进行有效性验证
- 异常处理：无效网格构造抛出标准异常
- 无效LOD请求：返回基础级别数据

### Bounding Volume包围盒
包围盒系统是MiteEngine的空间计算核心，提供统一的几何体表示和高效的相交测试算法。该系统支持多种包围体类型，为空间查询、碰撞检测和视锥剔除提供数学基础。

**设计理念**
- 统一接口：多种包围体类型的统一抽象
- 类型转换：支持包围体间的智能转换
- 精确测试：基于分离轴定理的精确相交检测
- 性能优化：近似算法与精确算法的平衡

**包围体类型体系**
```mermaid
classDiagram
    class BoundingVolume {
        +GetType() BoundingVolumeType
        +Transform(matrix) BoundingVolume
        +Intersects(other) IntersectionType
        +GetAABBApproximation() BoundingVolumeAABB
        +ConvertTo(targetType) bool
    }
    
    class BoundingVolumeAABB {
        +min: vec3
        +max: vec3
        +GetCenter() vec3
        +GetSize() vec3
    }
    
    class BoundingVolumeSphere {
        +center: vec3
        +radius: float
    }
    
    class BoundingVolumeOBB {
        +center: vec3
        +extents: vec3
        +orientation: mat3
    }
    
    class BoundingVolumePlane {
        +normal: vec3
        +distance: float
    }
    
    BoundingVolume --> BoundingVolumeAABB
    BoundingVolume --> BoundingVolumeSphere
    BoundingVolume --> BoundingVolumeOBB
    BoundingVolume --> BoundingVolumePlane
````

**OBB的分离轴定理** (SAT) ：OBB-OBB相交测试采用完整的分离轴定理，测试15个可能的分离轴：

分离轴集合：6个面法线轴（每个OBB的3个轴向）9个边叉积轴（3×3组合）

对于每个分离轴 $\vec{a}$，计算投影半径：
$$r_A = \sum_{i=0}^{2} |\vec{a} \cdot \vec{u}A^i| \cdot e_A^i$$
$$r_B = \sum{i=0}^{2} |\vec{a} \cdot \vec{u}_B^i| \cdot e_B^i$$

其中 $\vec{u}_A^i$ 和 $\vec{u}_B^i$ 是OBB的轴向向量， $e_A^i$ 和 $e_B^i$ 是半长。

时间复杂度： $O(1)$ 常数时间，但需要测试最多15个轴

**Sphere的Welzl最小包围球算法**：采用随机化线性时间算法计算点集的最小包围球：
1. 随机打乱点集顺序
2. 递归构建支撑集（最多4个点）
3. 基于支撑集大小计算最小球：1点：零半径球; 2点：直径球; 3点：外接圆球; 4点：四面体外接球

时间复杂度： 期望 $O(n)$

**四态相交结果**：支持所有的相交检测情况描述

```cpp
enum class IntersectionType {
    Outside,    // 完全不相交
    Inside,     // 测试对象在主体内部
    Intersect,  // 部分相交
    Covered     // 测试对象完全包含主体
};
````

**测试策略矩阵**

| 主体类型 | AABB | Sphere | OBB | Plane |
|---------|------|--------|-----|-------|
| **AABB** | 轴对齐测试 | 距离平方比较 | SAT简化 | 投影范围测试 |
| **Sphere** | 对称转换 | 中心距离比较 | 局部空间测试 | 符号距离测试 |
| **OBB** | 对称转换 | 对称转换 | 完整SAT | 投影范围测试 |
| **Plane** | 对称转换 | 对称转换 | 对称转换 | 无意义 |

对称转换规则：
- AABB转换：所有类型均可通过外接AABB近似
- Sphere转换：通过外接球保持保守性
- OBB转换：保持方向信息，精度最高
- Plane转换：有限支持，主要用于特殊场景

### Frustum视锥体

视锥体系统采用平面集合表示法，通过6个裁剪平面定义可视空间，为所有几何体类型提供统一的可见性判断接口：
```cpp
BoundingVolumeIntersection::IntersectionType TestBoundingVolume(const BoundingVolume& volume);
````

**平面提取算法**

从视图投影矩阵中提取6个裁剪平面，采用行向量组合法：

对于列主序矩阵 $M$，平面系数为：
- 左平面: $\text{row}_3 + \text{row}_0$
- 右平面: $\text{row}_3 - \text{row}_0$
- 下平面: $\text{row}_3 + \text{row}_1$
- 上平面: $\text{row}_3 - \text{row}_1$
- 近平面: $\text{row}_3 + \text{row}_2$
- 远平面: $\text{row}_3 - \text{row}_2$

标准化处理确保数值稳定性：

$$\vec{n} = \frac{\vec{coefficients}_{xyz}}{|\vec{coefficients}_{xyz}|}, \quad d = \frac{coefficient_w}{|\vec{coefficients}_{xyz}|}$$

**球体测试算法**

基于距离比较的快速判断，时间复杂度 $O(1)$ ：

```cpp
for (each plane P) {
    float distance = P.DistanceToPoint(sphere.center);
    if (distance < -sphere.radius) return Outside;  // 早期退出
    if (distance < sphere.radius) intersects = true;
}
return intersects ? Intersect : Inside;
````

**AABB测试算法**

采用极值点优化，避免测试所有8个顶点：

对于平面法线 $\vec{n}$：
- 正顶点：在 $\vec{n}$ 正方向上的AABB角点
- 负顶点：在 $\vec{n}$ 负方向上的AABB角点

测试逻辑：
- 测试负顶点：若 $d_{\text{negative}} < 0$，可能完全在外
- 验证完全在外：必要时测试8个顶点确认
- 测试正顶点：若 $d_{\text{positive}} < 0$，标记为相交

```mermaid
graph LR
    A[几何体输入] --> B{类型判断}
    B -->|球体| C[快速距离测试]
    B -->|AABB| D[极值点优化]
    B -->|OBB| E[AABB近似]
    
    C --> G[早期退出]
    D --> G
    E --> G
    
    G --> H[结果输出]
````

**优化技术**
- 早期退出：发现完全在外立即返回
- 极值点计算：将AABB测试从 $O(48)$ 优化到 $O(12)$
- 缓存友好：平面数据连续存储，提高缓存命中率
- 分支预测：减少测试中的条件分支

视锥体系统通过算法设计和多层优化策略，为实时渲染提供了高效的可见性判断基础，在复杂场景中可实现显著的性能提升。

### Ray射线

射线系统是MiteEngine的空间查询工具，提供从屏幕到3D空间的射线投射和几何体相交检测功能。主要用于鼠标拾取、碰撞检测(暂未实现)等交互场景。

**射线定义** $\vec{p} = \vec{o} + t\vec{d}$ 
```cpp
struct Ray {
    glm::vec3 origin;     // 射线起点
    glm::vec3 direction;  // 射线方向（单位向量）
    float tMin;           // 有效范围最小值
    float tMax;           // 有效范围最大值
};
````

**屏幕射线生成**

```mermaid
graph LR
    A[屏幕UV坐标] --> B[NDC坐标转换]
    B --> C[视图空间变换]
    C --> D[世界空间变换]
    D --> E[射线方向计算]
    E --> F[标准化射线]
````

- 屏幕UV(0,1) → NDC(1,1): $(2u-1, 1-2v, -1)$
- 逆投影变换: $\text{viewPos} = P^{-1} \times \text{ndcPos}$
- 逆视图变换: $\text{worldPos} = V^{-1} \times \text{viewPos}$

**相交检测统一接口设计**

通过提供统一的相交测试接口，支持所有包围体类型，通过类型分发自动选择最优算法，简化使用复杂度。
```cpp
bool Ray::Intersects(const BoundingVolume& volume, float& t) const;
````

**AABB相交检测**（SLAB方法）：基于分离轴定理的高效算法
1. 对每个坐标轴计算相交区间
2. 取所有区间的交集
3. 检查交集是否在有效范围内

对于每个轴 $i$：
$$t_1 = \frac{\text{aabb.min}[i] - \text{origin}[i]}{\text{direction}[i]}$$
$$t_2 = \frac{\text{aabb.max}[i] - \text{origin}[i]}{\text{direction}[i]}$$

取 $t_{\text{min}} = \max(t_1, t_2)$ 和 $t_{\text{max}} = \min(t_1, t_2)$ 的区间交集。

**球体相交检测**：基于二次方程求解

球方程： 
$$|\vec{p} - \vec{c}|^2 = r^2$$

射线方程： 
$$\vec{p} = \vec{o} + t\vec{d}$$

代入得二次方程： 
$$at^2 + bt + c = 0$$

其中： 
$$a = \vec{d} \cdot \vec{d}$$
$$b = 2\vec{d} \cdot (\vec{o} - \vec{c})$$
$$c = (\vec{o} - \vec{c}) \cdot (\vec{o} - \vec{c}) - r^2$$

**三角形相交检测**（Möller-Trumbore算法）：高效的单次相交测试

算法核心： 

$$\begin{bmatrix}t \\ u \\ v\end{bmatrix} = \frac{1}{\vec{d} \cdot (\vec{e_1} \times \vec{e_2})}\begin{bmatrix}\vec{q} \cdot \vec{e_2} \\\vec{p} \cdot \vec{t} \\\vec{q} \cdot \vec{d}\end{bmatrix}$$

其中： 
$$\vec{e_1} = \vec{v_1} - \vec{v_0}$$
$$\vec{e_2} = \vec{v_2} - \vec{v_0}$$
$$\vec{p} = \vec{d} \times \vec{e_2}$$
$$\vec{q} = \vec{t} \times \vec{e_1}$$
$$\vec{t} = \vec{o} - \vec{v_0}$$

其中： $\vec{v_0},\vec{v_1},\vec{v_2}$ 分别为三角形第一、二、三个顶点坐标 (Vertex 0,1,2)， $u,v$ 分别为相对于边 $\vec{v_0}\rightarrow\vec{v_1}$ 和 $\vec{v_0}\rightarrow\vec{v_2}$ 的重心坐标分量。 $\vec{o}, t,\vec{d}$ 为射线 $\vec{p} = \vec{o} + t\vec{d}$ 的三个分量

若$u,v$其中任意一个值为负，则射线和三角形不会相交。否则判定为相交，返回射线

### Runtime Texture运行时纹理对象

运行时纹理系统负责动态创建和管理渲染目标，包括G-Buffer、ShadowMap等非资产纹理。通过事件驱动架构与GPU资源管理解耦，支持实时调整尺寸和类型特定的参数配置。

设计特点
- 事件驱动创建：通过RuntimeTextureCreateEvent委托GPU设备创建纹理
- 类型化配置：根据纹理用途（阴影贴图、G-Buffer、光照结果）自动设置过滤模式和环绕方式
- 动态尺寸调整：支持运行时重设纹理尺寸，适应窗口变化
- 资源生命周期：RAII模式管理，自动清理GPU资源

## Shader着色器模块

Shader着色器模块是MiteEngine的渲染基础层，基于OpenGL图形API开发，负责管理所有着色器资源和GPU缓冲区。（注意，在文件结构上与Data在同一文件夹下，后续应当考虑拆分）

### Framebuffer 帧缓冲 & G-Buffer 几何缓冲
帧缓冲系统采用分层封装模式，提供从基础帧缓冲到专用几何缓冲的统一管理：

```mermaid
classDiagram
    class FrameBuffer {
        +Bind() void
        +Unbind() void
        +Resize() bool
        +GetColorAttachment() RuntimeTexturePtr
        +GetDepthAttachment() RuntimeTexturePtr
    }
    
    class GBuffer {
        +bind() void
        +unbind() void
        +resize() bool
        +GetTexture() RuntimeTexturePtr
    }
    
    class RuntimeTexture {
        +initialize() bool
        +resize() bool
        +cleanup() void
    }
    
    GBuffer --> FrameBuffer
    FrameBuffer --> RuntimeTexture
````

**规格驱动配置模式**： 帧缓冲通过FrameBufferSpec提供的附件列表统一配置所有参数

| 附件类型 | OpenGL绑定点 | 纹理参数 | 用途 |
|---------|-------------|----------|------|
| 颜色附件 | `GL_COLOR_ATTACHMENT0+N` | 线性过滤 | G-Buffer数据存储 |
| 深度附件 | `GL_DEPTH_ATTACHMENT` | 最近邻过滤 | 深度测试 |
| 模板附件 | `GL_STENCIL_ATTACHMENT` | 最近邻过滤 | 模板测试 |

**G-Buffer专用封装**：几何缓冲区布局设计

| 纹理索引 | 数据类型 | 存储格式 | 用途描述 |
|---------|----------|----------|----------|
| 0 | WorldPosDepth | RGBA32F | 世界坐标 + 深度 |
| 1 | BaseColorMatType | RGBA16F | 基础颜色 + 材质类型 |
| 2 | MetallicRoughnessAO | RGBA16F | PBR参数 |
| 3 | NormalScale | RGBA16F | 法线 + 缩放 |
| 4 | EmissionAlpha | RGBA16F | 自发光 + 透明度 |
| 5 | NPRParam | RGBA16F | 非真实渲染参数 |
| 6 | NPRColor | RGBA16F | 非真实渲染颜色 |

**智能资源生命周期管理**：采用RAII模式确保资源安全释放

```mermaid
flowchart LR
    A[FrameBuffer构造]
    A --> C[创建FBO对象]
    C --> D[创建RuntimeTexture附件]
    D --> E[配置纹理参数]
    E --> F[绑定到FBO, 执行完整性验证]
    
    H[FrameBuffer析构] --> I[Release清理]
    I --> J[清理颜色附件]
    J --> K[清理深度模板附件]
    K --> L[删除FBO对象]
````

**动态尺寸调整算法**：支持运行时重设尺寸，保持所有附件一致性

```mermaid
flowchart LR
    A[Resize调用] --> B{尺寸验证}
    B -->|无效| C[记录警告并返回]
    B -->|有效| D{尺寸是否变化}
    D -->|否| E[直接返回成功]
    D -->|是| F[更新规格尺寸, 重新初始化帧缓冲]
    F --> H[完整性验证]
    H -->|成功| I[返回成功]
    H -->|失败| J[记录错误并返回]
````

**性能优化特性**
1. 纹理参数优化：根据附件类型自动选择最优参数
- G-Buffer纹理：禁用mipmap，最近邻过滤（避免插值误差）
- 阴影贴图：最近邻过滤，边界钳制（减少阴影锯齿）
- 光照结果：线性过滤，边缘拉伸（平滑光照过渡）
  
2. 验证缓存机制：避免重复的完整性检查，通过脏标记管理验证状态。

### Shader 着色器 & Shader Cache 着色器缓存机制

着色器系统采用SPIR-V编译管线和智能缓存机制，提供统一的着色器资源管理：

```mermaid
classDiagram
    class OpenGLShader {
        +LoadFromFile() void
        +LoadFromSource() void
        +Bind() void
        +Unbind() void
    }
    
    class ShaderCache {
        +GetOpenGLShader() shared_ptr~OpenGLShader~
        +Clear() void
    }
    
    class ShaderIncluder {
        +GetInclude() shaderc_include_result*
        +ReleaseInclude() void
    }
    
    OpenGLShader --> ShaderIncluder
    ShaderCache --> OpenGLShader
````

**核心设计模式**：基于Shaderc的SPIR-V编译管线，采用两阶段编译策略确保跨平台兼容性

```mermaid
flowchart TB
    A[GLSL源码] --> B[Shaderc编译]
    B --> C[SPIR-V字节码]
    C --> D[OpenGL特殊化]
    D --> E[可执行着色器]
    
    F[文件包含] --> G[ShaderIncluder]
    G --> H[递归解析]
    H --> B
    
    I[预定义宏] --> J[条件编译]
    J --> B
````

**编译配置表**：为后续兼容Vulkan提供一定的基础
| 配置项 | 默认值 | 作用 |
|--------|--------|------|
| 目标环境 | Vulkan 1.2 | 严格SPIR-V验证 |
| 优化级别 | 性能优化 | 生成高效代码 |
| 调试信息 | 启用 | 便于错误追踪 |
| 包含解析 | 自定义 | 支持相对路径 |

**核心算法**：文件包含解析算法

```mermaid
flowchart LR
    A[GetInclude调用] --> B{包含类型判断}
    B -->|相对路径| C[基于请求文件解析]
    B -->|系统路径| D[直接使用路径]
    
    C --> E[深度检查≤16, 文件存在性验证]
    D --> E

    E --> H[创建包含结果]
````

**性能优化特性**：编译缓存策略与包含文件优化
- 路径规范化：消除路径差异导致的重复编译
- 弱引用管理：自动清理无引用着色器
- 深度限制：防止无限递归包含（最大16层）
- 路径缓存：避免重复的文件系统操作
- 错误恢复：提供详细的包含错误信息

**错误处理机制**：分层错误报告
```mermaid
flowchart LR
    A[编译错误] --> B[Shaderc错误信息]
    B --> C[增强错误提示]
    C --> D[文件路径上下文]
    D --> E[抛出标准异常]
    
    F[链接错误] --> G[OpenGL信息日志]
    G --> H[程序链接状态]
    H --> E
````

### Unifrom Buffer 统一缓冲区
Uniform Buffer系统采用预分配绑定点模式，提供类型安全的GPU常量数据管理。后续创建的实例负责管理UBO，包括创建、更新和绑定操作

```mermaid
classDiagram
    class ShaderUBO {
        +Initialize() void
        +UpdateData() bool
        +Bind() void
        +Unbind() void
    }
    
    class XXXInstance {
        +CreateUBO() ShaderUBOPtr
        +UpdateUBO() ShaderUBOPtr
        +BindUBO() ShaderUBOPtr
    }
    
    ShaderUBO --> XXXInstance : 使用
````

**生命周期状态机**：确保资源管理的严格顺序

```mermaid
stateDiagram-v2
    [*] --> Uninitialized
    Uninitialized --> Initialized : Initialize()
    Initialized --> Updating : UpdateData()
    Updating --> Bound/Unbound : Bind()/Unbind()
    Bound/Unbound --> [*] : Destroy()
    
    note right of Uninitialized
        验证绑定点有效性
        检查大小合理性
    end note
    
    note right of Updating
        数据范围验证
        OpenGL错误检查
    end note
````

**缓冲区创建流程**：采用标准的OpenGL缓冲区创建模式

```mermaid
flowchart LR
    A[glGenBuffers] --> B[glBindBuffer]
    B --> C[glBufferData分配内存]
    C --> E[OpenGL错误检查]
    E --> F[创建成功]
    E --> G[创建失败清理]
````

**内存管理策略--大小对齐要求**：遵循OpenGL UBO的对齐规则
1. 基础对齐：std140布局标准
2. 数组对齐：按vec4对齐
3. 结构体对齐：最大成员对齐

**内存管理策略--数据更新**：支持部分更新和全量更新（目前仅使用了全量更新策略）
```cpp
// 全量更新
UpdateData(data, totalSize, 0);
// 部分更新  
UpdateData(partialData, partialSize, offset);
````

### Shader Storage Buffer 存储缓冲区
Shader Storage Buffer系统在UBO基础上扩展可读写映射和GPU计算能力，支持大规模数据交互
```mermaid
classDiagram
    class ShaderSSBO {
        +MapBuffer() void*
        +UnmapBuffer() bool
        +ReadData() bool
        +ClearData() bool
    }
    
    class ShaderUBO {
        // 只读常量数据管理
    }
    
    ShaderSSBO --|> ShaderUBO : 扩展功能
````

**核心设计模式--内存映射状态机**：管理CPU-GPU内存映射的复杂状态转换

```mermaid
stateDiagram-v2
    [*] --> Unmapped
    Unmapped --> Mapped : MapBuffer()
    Mapped --> Reading : ReadData()
    Mapped --> Writing : UpdateData()
    Mapped --> Clearing : ClearData()
    Reading --> Mapped
    Writing --> Mapped
    Clearing --> Mapped
    Mapped --> Unmapped : UnmapBuffer()
    
    note right of Mapped
        禁止绑定操作
        禁止直接数据更新
        必须通过映射指针访问
    end note
````

**扩展使用模式支持**：相比UBO增加更多使用场景
| 使用模式 | OpenGL枚举 | 适用场景 |
|---------|------------|----------|
| 动态复制 | `GL_DYNAMIC_COPY` | CPU→GPU数据传输 |
| 动态读取 | `GL_DYNAMIC_READ` | GPU→CPU数据读取 |
| 流式绘制 | `GL_STREAM_DRAW` | 频繁数据更新 |

**核心算法实现--内存映射管理算法**：提供零拷贝数据访问能力

```cpp
void* MapBuffer(GLenum access) {
    if (m_IsMapped) return nullptr;  // 防止重复映射
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
    void* mappedPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
    m_IsMapped = (mappedPtr != nullptr);
    return mappedPtr;
}
````

**核心算法实现--数据清除优化算法**：利用映射机制实现高效数据初始化

```mermaid
flowchart LR
    A[ClearData调用] --> B{映射状态检查}
    B -->|已映射| C[返回错误]
    B -->|未映射| D[映射为写入模式]
    D --> E[指针有效性验证]
    E --> F[批量填充清除值]
    F --> G[解映射缓冲区]
````

**性能优化特性--访问模式优化**：根据数据流向选择最优访问策略：

| 数据流向 | 推荐方法 | 性能开销 |
|---------|----------|----------|
| CPU→GPU批量更新 | `UpdateData()` | 中等，适合大块数据 |
| GPU→CPU数据读取 | `ReadData()` | 较低，同步操作 |
| 频繁小量更新 | `MapBuffer()` | 最佳，零拷贝 |
| 数据初始化 | `ClearData()` | 高效，批量操作 |

**计算着色器集成**：与GPU计算管线深度整合

```cpp
// 绑定SSBO到计算着色器
ssbo->Bind();
glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ);
glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
````

### Binding Point Manager 绑定点管理
绑定点管理系统是MiteEngine的GPU资源协调核心，采用**命名空间隔离**和**预分配策略**统一管理UBO、SSBO和纹理单元的绑定点分配。该系统确保在复杂渲染场景中避免资源绑定冲突，提供线程安全的动态分配机制。

**多命名空间隔离**：系统将GPU绑定点划分为三个独立的命名空间，从根本上避免资源类型冲突

```mermaid
graph LR
    A[绑定点管理系统] --> B[UBO命名空间]
    A --> C[SSBO命名空间] 
    A --> D[纹理命名空间]
    
    B --> F[预分配: Camera/Material/Model]
    B --> G[动态分配: 按类型分组]
    
    C --> I[预分配: LightSSBO]
    C --> J[动态分配: 按类型分组]
    
    D --> L[预分配: G-Buffer/ ShadowMap]
    D --> M[动态分配: 按类别分组]

````

**混合分配策略**：结合预分配和动态分配的优点

预分配资源（引擎初始化时固定）：
- 核心UBO：Camera、Material、Model、Shadow相关
- 核心SSBO：Light数据
- 运行时纹理：G-Buffer各通道、ShadowMap、光照结果
- 外部纹理：BaseColor、Normal、PBR贴图等

动态分配资源（运行时按需分配）：
- 临时UBO/SSBO缓冲区
- 动态生成的纹理资源
- 用户自定义资源

**类型化分配管理**：为每种资源类型维护独立的分配状态

| 管理维度 | 实现机制 | 优势 |
|---------|----------|------|
| **类型分组** | `std::array<std::atomic<uint32_t>, TypeCount>` | 避免类型间竞争 |
| **位集状态** | `std::bitset<1024>` | O(1)查询，固定内存 |
| **原子操作** | `std::atomic<uint32_t>` | 线程安全无锁 |

**使用流程**

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant BPM as BindingPointManager
    participant GPU as GPU资源
    
    App->>BPM: 初始化引擎
    BPM->>BPM: PreallocateCommonResources()
    BPM->>GPU: 预分配核心资源绑定点
    
    App->>BPM: 请求动态UBO绑定点
    BPM->>BPM: AllocateUBOBinding(type, name)
    BPM->>App: 返回绑定点索引
    
    App->>GPU: 使用分配的点创建UBO
    App->>BPM: 资源销毁时释放绑定点
    BPM->>BPM: ReleaseUBOBinding(point)
````

Shader模块作为引擎的关键渲染基础设施，它为上层渲染管线提供统一的着色器编译、资源绑定和缓冲区管理能力，支撑渲染技术的实现。

## Asset资产模块
Asset模块是MiteEngine的资源管理核心，负责统一加载、缓存和生命周期管理各类游戏资产。基于事件驱动架构，该模块通过异步加载机制（暂未实现，待后续使用Async事件订阅机制完善）和智能缓存策略，为渲染系统提供高效的资源访问接口，支持模型、材质、纹理等资产的按需加载和内存优化。

### Texture Loader纹理加载器
纹理加载器采用事件驱动架构和统一加载接口设计，负责将外部图像文件和嵌入式纹理数据解析为标准的纹理资产。通过STB图像库和Assimp接口实现多格式支持，与GPU资源创建完全解耦。

**设计模式--事件驱动资源创建，双路径加载策略**：纹理加载器仅负责CPU端数据解析，通过TextureLoadEvent委托Renderer创建GPU资源，实现加载逻辑与渲染API的彻底分离；支持外部文件加载和Assimp嵌入式纹理两种数据源的路径格式，通过统一的内部接口处理像素数据转换。
```mermaid
graph LR
    A[加载请求] --> B{路径类型判断}
    B -->|外部文件| C[外部纹理--STB文件加载]
    B -->|嵌入式标识| D[嵌入式纹理--Assimp数据提取]
    
    C --> G[像素数据解析+元数据构建]
    D --> G
    G --> I[发布加载事件,Renderer创建GPU资源]
````

**缓存集成机制--路径哈希标识**：使用UUID生成器基于纹理路径创建唯一标识，确保相同路径的纹理仅加载一次。缓存查询流程如下：
1. 路径→UUID转换生成搜索ID
2. 缓存查找并验证路径匹配
3. 返回已缓存资产或触发加载

### Material Loader材质加载器

材质加载器采用GLTF PBR标准解析和纹理依赖管理设计，专门处理基于物理渲染的材质导入。通过Assimp接口提取PBR参数和纹理引用，构建完整的材质资产依赖图。
```mermaid
flowchart LR
    A[GLTF场景] --> B[遍历材质]
    B --> C[提取PBR参数]
    B --> D[解析纹理引用]
    C --> E[构建材质资产]
    D --> E
    E --> F[发布加载事件]
````

**PBR参数提取**：采用分层回退策略，从GLTF专用参数回退到传统材质参数

| 参数 | 优先级 | 默认值 |
|------|--------|--------|
| 基础颜色 | GLTF PBR → 漫反射 → 灰色 | (0.8,0.8,0.8,1.0) |
| 金属度 | GLTF因子 → 0.0 | 0.0 |
| 粗糙度 | GLTF因子 → 1.0 | 1.0 |

**纹理依赖管理**：支持5种核心纹理类型

| 纹理类型 | 用途 |
|----------|------|
| 基础颜色 | 反照率 |
| 金属粗糙度 | PBR参数 |
| 法线 | 表面细节 |
| 自发光 | 发光效果 |
| 环境光遮蔽 | 阴影细节 |

### Model Loader模型加载器

模型加载器采用多格式适配和LOD链式管理设计，通过Assimp库解析3D模型并构建完整的资产依赖图，支持自动网格优化和层次细节生成。
```mermaid
flowchart TD
    A[模型文件] --> B{格式判断}
    B -->|GLTF| C[GLTF特化配置]
    B -->|OBJ| D[OBJ特化配置]
    B -->|其他| E[通用配置]
    
    C --> F[Assimp解析]
    D --> F
    E --> F
    
    F --> G[材质加载]
    F --> H[网格处理]
    G --> I[资产构建]
    H --> I
    I --> J[事件发布]
````

**格式特化配置**（主要使用GLTF作为引擎标准格式）
| 格式 | 特化配置 | 优化标志 |
|------|----------|----------|
| GLTF | 缓存优化 | `aiProcess_ImproveCacheLocality` |
| OBJ | 网格合并 | `aiProcess_OptimizeMeshes` |
| FBX | 骨骼限制 | `aiProcess_LimitBoneWeights` |

**通用处理管线**
- 三角化：`aiProcess_Triangulate`
- 法线生成：`aiProcess_GenNormals`
- 切线计算：`aiProcess_CalcTangentSpace`
- 顶点合并：`aiProcess_JoinIdenticalVertices`

**LOD链式管理**：支持多级细节自动生成

```mermaid
flowchart LR
    A[原始网格] --> B[顶点缓存优化]
    B --> C[网格简化]
    C --> D[顶点重映射]
    D --> E[LOD级别n]
    
    A --> F[基础LOD级别0]
    E --> G[LOD链构建]
    F --> G
````

**网格简化算法**：基于meshoptimizer的高效简化
1. 顶点缓存优化：`meshopt_optimizeVertexCache`
2. 粗略简化：`meshopt_simplifySloppy`
3. 顶点重映射：`meshopt_optimizeVertexFetchRemap`
4. 数据重组：`meshopt_remapVertexBuffer` + `meshopt_remapIndexBuffer`

注意： 常规方法是使用meshopt_simplify进行粗略简化，利用target_error限制误差。但实际操作过程中，发现无论如何调整target_error，均无法确保粗略简化可以精简模型顶点数，所以使用simplifySloppy以确保顶点数量减少

**数据合并算法**：将多子网格合并为单一渲染批次

```cpp
// 索引偏移修正公式
adjustedIndex = originalIndex + vertexOffset
// 顶点数据合并
mergedData = Merge(subMesh[i].vertexData)
````
合并优势：
- 减少Draw Call数量
- 提升缓存一致性
- 简化资源管理

**包围盒计算**：基于所有子网格的极值点计算模型级包围盒：

$$BBox_{min} = \min(subMesh[i].bbox_{min})$$
$$BBox_{max} = \max(subMesh[i].bbox_{max})$$

**事件驱动架构**：模型加载事件流

1. 材质依赖解析完成
2. 网格数据处理完毕
3. LOD链构建完成
4. 生成ModelSourceData
5. 发布ModelLoadEvent
6. Renderer接收并创建GPU资源

模型加载器通过格式特化配置和智能的LOD管理，为引擎提供高性能的3D模型导入能力。

### Asset Cache资产缓存

资产缓存采用引用计数和LRU淘汰策略的双重管理机制，为引擎提供线程安全的资源生命周期管理，支持模型、材质、纹理等各类资产的统一缓存。

**设计架构**：LRU淘汰策略
```mermaid
flowchart TD
    A[资产请求] --> B{缓存查找}
    B -->|命中| C[更新LRU位置]
    B -->|未命中| D[加载新资产]
    
    C --> E[返回资产指针]
    D --> F[存储到缓存]
    F --> E
    
    G[引用管理] --> H[引用计数增减]
    H --> I{计数归零}
    I -->|是| J[LRU标记待清理]
    I -->|否| K[保持活跃]
````

**引用计数管理**：基于RAII模式的智能引用跟踪
| 操作 | 引用计数变化 | 触发条件 |
|------|-------------|----------|
| `Store` | 初始为0 | 新资产入库 |
| `Get` | 无变化 | 只读访问 |
| `AddRefCount` | +1 | 显式增加引用 |
| `Release` | -1 | 显式释放引用 |

**LRU淘汰策略**：基于访问频率的智能缓存管理
- 最近访问：移动到链表头部
- 淘汰候选：链表尾部元素
- 淘汰条件：引用计数为0 + 超过缓存限制

**性能特性**
- 查找复杂度：O(1) - 哈希表
- LRU更新：O(1) - 链表操作
- 线程安全：细粒度锁保护

**调用模式**
1. 加载器调用Store()存入新资产
2. 使用者调用Get()获取资产指针
3. 场景引用调用AddRefCount()增加引用
4. 场景卸载调用Release()减少引用
5. 系统定期调用PurgeUnused()清理

### Asset Manager资产管理器
资产管理器作为Asset模块的统一对外接口，采用单例模式和统一生命周期管理设计，为引擎提供简洁高效的资源加载和引用管理服务。
```mermaid
flowchart LR
    B[资产管理器]
    B --> C{资源类型}
    C -->|纹理| D[纹理加载器]
    C -->|模型| E[模型加载器]
    C -->|材质| F[材质加载器]
    
    D --> G[纹理缓存]
    E --> H[模型缓存]
    F --> I[材质缓存]
    
    G --> J[返回资产ID]
    H --> J
    I --> J
````

**调用模式**
```cpp
// 1. 加载资源
auto textureID = AssetManager::Get().LoadTexture("texture.png");
auto modelID = AssetManager::Get().LoadGLTFModel("scene.gltf");
// 2. 使用资源  
auto texture = AssetManager::Get().GetTexture(textureID);
auto model = AssetManager::Get().GetModel(modelID);
// 3. 释放资源
AssetManager::Get().ReleaseTexture(textureID);
AssetManager::Get().ReleaseModel(modelID);
````

Asset模块通过统一的资源管理架构和智能缓存机制，为引擎提供了高效可靠的资产加载管线。其事件驱动的异步加载设计（暂未实现，待后续使用Async事件订阅机制完善）和引用计数生命周期管理，确保了大规模资产场景下的性能和内存效率。

## Material材质模块

Material模块是MiteEngine的材质管理系统，采用模板-实例分离架构统一管理材质创建和参数配置。该模块支持GLTF PBR标准材质，通过事件驱动机制与Asset模块协同工作，为渲染管线提供灵活的材质表达能力。

Material模块采用三层架构设计，通过材质模板和参数列表创建材质实例：
```mermaid
classDiagram
    class MaterialTemplate {
        <<abstract>>
        +CreateInstance() MaterialInstance
        +GetMaterialType() MaterialType
    }
    
    class GLTFPBRMaterialTemplate {
        +SetDefaultBaseColor()
        +SetDefaultMetallic()
        +SetDefaultRoughness()
    }
    
    class MaterialInstance {
        +InitializeUBO()
        +UpdateUBO()
        +BindTexturesOnly()
    }
    
    class MaterialParamVariant {
        +GetType() Type
        +Get() T
        +Is() bool
    }
    
    MaterialTemplate <|-- GLTFPBRMaterialTemplate
    MaterialTemplate --> MaterialInstance : 创建
    MaterialParamVariant --> MaterialInstance : 参数存储
````

MaterialFactory材质工厂作为中心协调者，管理模板注册和实例创建

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant MF as MaterialFactory
    participant GT as GLTFPBRMaterialTemplate
    participant MI as MaterialInstance
    participant MPV as MaterialParamVariant
    
    App->>MF: Initialize()
    MF->>GT: 注册PBR模板
    
    App->>MF: CreateInstance(PBR)
    MF->>GT: CreateInstance()
    GT->>MI: 创建实例
    GT->>MPV: 设置默认参数
    MI->>App: 返回材质实例
````

**运行时数据流**：材质实例在运行时管理参数和渲染状态

```mermaid
graph LR
    A[MaterialInstance] --> B[MaterialParamVariant]
    A --> C[UBO数据]
    A --> D[纹理绑定]
    
    B --> E[参数类型安全]
    C --> F[Shader Uniform]
    D --> G[GPU纹理单元]
    
    E --> H[运行时修改]
    F --> I[渲染管线]
    G --> I
````

### Material Param Variant材质可变参数

[Material Param Variant](./src/engine/data/basic_type/material_param_variant.h)核心设计--**变体模式** (Variant Pattern)：MaterialParamVariant采用std::variant实现类型安全的联合体，为材质系统提供统一的参数存储接口

核心算法--**类型映射算法**：通过编译期索引映射实现高效的运行时类型查询

```mermaid
flowchart LR
    A[GetType调用] --> B[获取variant索引]
    B --> C[预计算类型映射表]
    C --> D[返回对应Type枚举]
    
    subgraph C [编译期类型映射]
        F[索引0: Bool]
        G[索引1: Int]
        H[其他类型...]
    end
````

**Shader字符串编译期多态转换算法**：采用std::visit+lambda的访问者模式，利用C++17的constexpr if实现类型特化（为程序化生成Shader的设计目标提前规划，目前暂未启用）
```mermaid
flowchart LR
    B[std::visit访问]
    B --> C{类型判断}
    
    C -->|标量类型| D[直接转换]
    C -->|向量类型| E[分量拼接]
    C -->|矩阵/数组类型| F[暂不支持]
    
    D --> H[格式化输出String]
    E --> H
    
    subgraph D [标量处理]
        D1[bool: true/false]
        D2[数值: 保留3位小数]
        D3[去除尾随零]
    end
    
    subgraph E [向量处理]
        E1[提取各分量]
        E2[vecN构造函数格式]
        E3[逗号分隔]
    end
````

**访问复杂度**

| 操作 | 时间复杂度 | 备注 |
|------|------------|------|
| 类型查询 | O(1) | 索引映射 |
| 值获取 | O(1) | 直接访问 |
| 类型检查 | O(1) | 编译期优化 |
| 字符串转换 | O(n) | 数据依赖 |

### Material Instance材质实例

[MaterialInstance](./src/engine/data/basic_instance/material_instance.h)采用UBO统一管理所有材质参数，实现高效的GPU数据传输

```mermaid
classDiagram
    class MaterialInstance {
        +InitializeUBO()
        +UpdateUBO()
        +Apply() 
        +BindTexturesOnly()
    }
    
    class MaterialUniformBuffer {
        +baseColor: vec4
        +metallicRoughnessAO: vec3
        +emission: vec4
        +textureCNMROFlags: vec4
    }
    
    class ShaderUBO {
        +Initialize()
        +UpdateData()
        +Bind()
    }
    
    MaterialInstance --> MaterialUniformBuffer : 包含
    MaterialInstance --> ShaderUBO : 管理
````

材质实例也负责了UBO的生命周期管理：
```mermaid
stateDiagram-v2
    [*] --> Uninitialized
    Uninitialized --> Initialized : InitializeUBO()
    Initialized --> Updating : UpdateUBO()
    Updating --> Bound : BindBuffersOnly()
    Bound --> Updating : 参数修改
    Bound --> [*] : 析构
    
    note right of Initialized
        预分配GPU内存
        设置动态更新标志
    end note
````
以及确定性纹理绑定策略（使用[Binding Point Manager 绑定点管理](#binding-point-manager-绑定点管理)预先分配的绑定点，确保与[Uniform.glsl](./assets/shaders/common/uniforms.glsl)中的绑定一致）:
| 纹理类型 | 绑定点 | 默认处理 |
|----------|--------|----------|
| BaseColor | `binding = 18` | 绑定默认纹理 |
| Normal | `binding = 19` | 绑定默认纹理 |
| MetallicRoughness | `binding = 20` | 绑定默认纹理 |
| Emissive | `binding = 21` | 绑定默认纹理 |
| Occlusion | `binding = 22` | 绑定默认纹理 |

在实际渲染流程中，材质实例负责MaterialUBO参数的绑定、以及纹理的绑定操作：
```mermaid
sequenceDiagram
    participant R as Renderer
    participant MI as MaterialInstance
    participant UBO as ShaderUBO
    participant GPU as GPU
    
    R->>MI: Apply(textureBindFunc)
    MI->>UBO: Bind()
    UBO->>GPU: 绑定UBO到固定点
    MI->>MI: BindTexturesOnly()
    loop 纹理
        MI->>GPU: 绑定纹理
    end
````

用于绑定的MaterialUniformBuffer采用紧凑的向量打包策略优化内存使用：
| 字段 | 类型 | 分量 | 用途 |
|------|------|------|------|
| `baseColor` | `vec4` | RGBA | 基础颜色 |
| `metallicRoughnessAO` | `vec3` | M/R/AO | PBR参数 |
| `emission` | `vec4` | RGB+Intensity | 自发光 |
| `textureCNMROFlags` | `vec4` | 4×bool | 纹理启用标志 |
| `nprParameters` | `vec4` | 4×float | NPR参数 |
| `nprColors` | `vec4` | RGB+Power | NPR颜色 |
| 纹理参数 | 5×`vec4` | Scale+Offset | 纹理变换 |

### Material Template材质模板
MaterialTemplate作为抽象基类，定义了材质创建的通用方法：

1. 获取材质源数据/使用默认值生成源数据
2. 初始化UBO
3. 发布创建事件
4. 填充材质数据
5. 设置纹理绑定
6. 返回材质实例

扩展接口设计

MaterialTemplate提供多个虚函数供具体材质类型定制
| 虚函数 | 用途 | 默认实现 |
|--------|------|----------|
| `GetMaterialType()` | 类型标识 | 纯虚函数 |
| `GetMaterialTypeName()` | 类型名称 | 纯虚函数 |
| `GetDefaultBaseColor()` | 默认颜色 | 灰色 |
| `GetDefaultMetallic()` | 默认金属度 | 0.0 |
| `GetDefaultRoughness()` | 默认粗糙度 | 1.0 |

并提供静态工具方法简化派生类开发
| 工具方法 | 功能 | 复杂度 |
|----------|------|--------|
| `GetParameter()` | 安全参数提取 | O(1) |
| `GetTextureSlot()` | 纹理槽位访问 | O(1) |
| `HasParameter()` | 参数存在检查 | O(1) |
| `HasTextureSlot()` | 纹理存在检查 | O(1) |

### GLTF PBR Material基于物理的材质

[GLTFPBRMaterial](./assets/shaders/brdf/pbr.brdf.glsl)是MaterialTemplate的具体实现，专门处理GLTF 2.0标准的基于物理渲染材质。该模板通过重写默认参数方法提供符合GLTF规范的PBR材质配置。

**PBR渲染方程实现**

基于Cook-Torrance微表面模型，渲染方程如下：

$$L_o = L_e + \sum_{k=1}^{n} (L_{\text{diffuse}} + L_{\text{specular}}) + L_{\text{ambient}}$$

其中： 
$L_o$：出射光亮度， 
$L_e$：自发光项， 
$L_{\text{diffuse}}$：漫反射项， 
$L_{\text{specular}}$：镜面反射项， 
$L_{\text{ambient}}$：环境光项 

**基础反射率计算 (F0)**

采用GLTF标准的电介质-金属混合模型：

```mermaid
flowchart LR
    A[基础颜色] --> B{金属度判断}
    B -->|金属| C[F0 = 基础颜色]
    B -->|电介质| D[F0 = 0.04]
    C --> E[混合结果]
    D --> E
````

计算基础反射率的公式： 

$$F_0 = \text{mix}(0.04, \text{baseColor}, \text{metallic})$$

**法线分布函数 (D) - GGX/Trowbridge-Reitz**

描述微表面法线分布的统计模型： 

$$D(h) = \frac{\alpha^2}{\pi((n \cdot h)^2(\alpha^2 - 1) + 1)^2}$$

其中： 
$\alpha = \text{roughness}^2$， 
$h$：半角向量， 
$n$：表面法线 

**几何遮蔽函数 (G) - Smith方法**

考虑微表面相互遮蔽的几何效应：

$$G = G_1(l) \cdot G_1(v)$$ 
$$G_1(x) = \frac{n \cdot x}{(n \cdot x)(1 - k) + k}$$ 
$$k = \frac{(\text{roughness} + 1)^2}{8}$$ 

**菲涅尔方程 (F) - Schlick近似**

描述光线在界面反射比例的物理现象： 

$$F(v, h) = F_0 + (1 - F_0)(1 - (v \cdot h))^5$$

**能量守恒规则**

- 镜面反射比例 $k_S = F$
- 漫反射比例 $k_D = (1 - F) \times (1 - \text{metallic})$
- 金属材质没有漫反射分量

**环境光照 (IBL) 实现**

漫反射环境光 

$$L_{\text{diffuse}} = k_D \times \text{baseColor} \times \text{irradiance} \times \text{occlusion}$$

镜面反射环境光 

$$L_{\text{specular}} = \text{prefilteredColor} \times (F \times \text{brdfLUT}.x + \text{brdfLUT}.y)$$

**输入参数验证**

| 参数 | 有效范围 | 默认处理 |
|------|----------|----------|
| baseColor | [0, ∞] | 返回黑色 |
| metallic | [0, 1] | 钳制到边界 |
| roughness | [0, 1] | 钳制到边界 |
| normal | length > 0 | 返回默认值 |

GLTF PBR Material实现遵循GLTF 2.0标准，通过物理正确的BRDF计算和能量守恒设计，为引擎提供了基于物理的渲染能力。

### Material Factory材质工厂

MaterialFactory作为材质系统的中心协调者，采用工厂模式统一管理所有材质模板和实例创建

**工厂初始化流程**：引擎启动时完成材质模板的注册和事件订阅
```mermaid
graph LR
    A[模板创建] --> B[类型提取]
    B --> C{类型冲突检查}
    C -->|存在| D[记录错误]
    C -->|不存在| E[注册模板]
    E --> F[存储到映射表]
````

**运行时实例创建流程**：
```mermaid
sequenceDiagram
    participant AM as Asset模块
    participant EB as EventBus
    participant MF as MaterialFactory
    participant MT as MaterialTemplate
    participant MI as MaterialInstance
    
    AM->>EB: MaterialLoadedEvent
    EB->>MF: OnMaterialLoaded
    MF->>MF: CreateInstanceFromMaterialSourceData
    MF->>MT: CreateInstance(sourceData)
    MT->>MI: 创建实例
    MI->>AM: 返回实例引用
````

**性能特性**：

内存管理
| 资源类型 | 管理策略 | 生命周期 |
|----------|----------|----------|
| 材质模板 | 唯一指针 | 引擎运行期 |
| 材质实例 | 共享指针 | 引用计数 |
| 事件订阅 | RAII模式 | 工厂生命周期 |

访问性能
| 操作 | 复杂度 | 优化策略 |
|------|--------|----------|
| 模板查找 | O(1) | 哈希映射 |
| 实例创建 | O(1) | 直接委托 |
| 事件处理 | O(1) | 早期阻断 |

Material模块通过工厂-模板-实例的三层架构，实现了类型安全、高性能的材质管理系统。该设计遵循GLTF PBR标准，通过物理正确的BRDF计算，为引擎提供了基于物理的渲染能力，同时保持了良好的扩展性。

## Light光照模块

Light模块是MiteEngine的光照管理系统，负责统一管理各类光源类型、阴影计算和光照数据组织。该模块支持多种光源类型，通过SSBO高效传递光照数据，为渲染管线提供完整的光照解决方案。
```mermaid
graph LR
    B[LightManager]
    B --> C[SSBO数据更新]
    C --> D[GPU光照数据]
    D --> E[着色器采样]
    
    F[阴影系统] --> G[ShadowMap生成]
    G --> H[阴影数据]
    H --> E
````

### Light光源抽象

Light基类采用策略模式，为不同类型光源提供统一的接口规范
```mermaid
classDiagram
    class Light {
        <<abstract>>
        +PrepareGPULightData() GPULightData
        +CalculateInfluenceRadius() float
        +CreateDefaultShadowMap() void
        +Validate() bool
    }
    
    class DirectionalLight {
        +PrepareGPULightData() GPULightData
        +CalculateInfluenceRadius() float
    }
    
    class PointLight {
        +PrepareGPULightData() GPULightData
        +CalculateInfluenceRadius() float
    }
    
    Light <|-- DirectionalLight
    Light <|-- PointLight
````

核心接口设计：所有光源类型必须实现统一的GPU数据准备接口
```mermaid
flowchart TD
    A[PrepareGPULightData]
    A --> C[填充基础属性]
    C --> D{光源类型判断}
    D -->|点光源| E[填充点光源参数]
    D -->|聚光灯| F[填充聚光灯参数]
    D -->|方向光| G[填充方向光参数]
    E --> I[返回GPULightData]
    F --> I
    G --> I
````

GPULightData内存布局：类型特定属性使用union联合体，减少内存占用
| 字段组 | 大小 | 对齐 | 用途 |
|--------|------|------|------|
| 基础属性 | 48字节 | 16字节 | 颜色、强度、位置、类型 |
| 类型特定属性Union | 16字节 | 16字节 | 光源类型参数 |
| 总大小 | 64字节 | 16字节 | 完整光源数据 |

影响半径公式：

- 点光源：$r = \text{radius} \times \sqrt{\text{intensity}}$
- 聚光灯：$r = \text{range} \times \text{intensity}$
- 方向光：$r = \infty$（无限大）

阴影系统集成（具体信息参考[阴影贴图章节](# Shadow Map阴影贴图)）

```mermaid
sequenceDiagram
    participant L as Light
    participant SM as ShadowMap
    participant SD as ShadowData
    
    L->>SM: PrepareShadowData
    SM->>SM: 计算阴影矩阵
    SM->>SM: 配置级联参数
    SM->>SD: 生成阴影数据
    SD->>L: 返回结果
````

### Shadow Map阴影贴图

ShadowMap系统采用组合模式，通过统一的基类接口管理不同类型的阴影贴图
```mermaid
classDiagram
    class ShadowMap {
        <<abstract>>
        +PrepareShadowData() ShadowMapData
        +GetShadowMatrixCount() size_t
        +GetShadowMatrix() mat4
        +NeedsUpdate() bool
    }
    
    class DirectionalShadowMap {
        +PrepareShadowData() ShadowMapData
        +GetShadowMatrixCount() size_t
    }
    
    class PointShadowMap {
        +PrepareShadowData() ShadowMapData
        +GetShadowMatrixCount() size_t
    }
    
    ShadowMap <|-- DirectionalShadowMap
    ShadowMap <|-- PointShadowMap
````

阴影数据统一接口：所有阴影类型必须实现统一的阴影数据准备接口ShadowMapData
```mermaid
flowchart TD
    A[PrepareShadowData] --> B[光源变换提取]
    B --> C{光源类型判断}
    C -->|方向光| D[计算级联分割]
    C -->|点光源| E[计算立方体贴图矩阵]
    C -->|聚光灯| F[计算投影矩阵]
    D --> G[生成级联VP矩阵]
    E --> H[生成6个面矩阵]
    F --> I[生成单个VP矩阵]
    G --> J[填充ShadowMapData]
    H --> J
    I --> J
````

该阴影系统通过统一的接口设计和类型特定的优化算法，为不同光源类型提供了高质量的阴影解决方案。

### Point Light点光源
PointLight继承自Light基类，通过重写模板方法实现点光源特定行为。

GPU数据结构
| 字段组 | 内容 | 大小 |
|--------|------|------|
| 基础属性 | color, intensity, position, type | 48字节 |
| 点光源参数 | range, falloff, 填充 | 16字节 |

核心算法：

点光源影响半径算法：点光源采用基于物理的衰减模型计算影响范围 

$$r_{\text{influence}} = r_{\text{base}} \times \sqrt{I}$$

其中： $r_{\text{base}}$：基础影响半径， $I$：光源强度， $\sqrt{I}$：强度因子，模拟物理衰减

点光源衰减算法：采用物理正确的平方反比衰减模型，结合平滑边缘处理 

$$A = \frac{f}{d^2} \times \left(1 - \text{smoothstep}(0.8r, r, d)\right)$$

其中： $d$：光源到表面的距离， $r$：光源影响范围， $f$：衰减系数， $\text{smoothstep}$：平滑过渡函数

立方体贴图阴影算法：PointShadowMap采用立方体贴图技术生成全向阴影
| 面索引 | 目标方向 | 上方向 | 用途 |
|--------|----------|--------|------|
| 0 | +X | -Y | 右面 |
| 1 | -X | -Y | 左面 |
| 2 | +Y | +Z | 上面 |
| 3 | -Y | -Z | 下面 |
| 4 | +Z | -Y | 前面 |
| 5 | -Z | -Y | 后面 |

投影矩阵公式： 

$$P = \text{perspective}(90^\circ, 1.0, \text{near}, \text{far})$$

视图矩阵公式： 

$$V_i = \text{lookAt}(\text{position}, \text{position} + \text{target}_i, \text{up}_i)$$

阴影深度比较算法： 

$$\text{visibility} = \begin{cases}0.0 & \text{if } d_{\text{current}} - b > d_{\text{closest}} \\1.0 & \text{otherwise}\end{cases}$$

其中： $d_{\text{current}}$：当前片段深度（标准化）， $d_{\text{closest}}$：立方体贴图存储的最接近深度， $b$：阴影偏移量

### Directional Light方向光
DirectionalLight继承自Light基类，通过重写策略方法实现方向光特定行为。
GPU数据结构
| 字段组 | 内容 | 大小 |
|--------|------|------|
| 基础属性 | color, intensity, position, type | 48字节 |
| 方向光参数 | irradiance, 填充 | 16字节 |

核心算法：

级联分割策略：采用对数-均匀混合分割算法 

$$C_i = \lambda \cdot \left(n \cdot \left(\frac{f}{n}\right)^{\frac{i}{k}}\right) + (1-\lambda) \cdot \left(n + (f-n) \cdot \frac{i}{k}\right)$$

其中： $C_i$：第i级级联的分割距离， $n, f$：相机近远平面， $k$：级联总数， $\lambda$：分割参数（0=均匀，1=对数）

级联矩阵计算：每个级联使用正交投影和优化的包围盒
```mermaid
sequenceDiagram
    participant CSM as CSM系统
    participant Frustum as 视锥体
    participant BBox as 包围盒
    participant Matrix as 矩阵生成
    
    CSM->>Frustum: 计算级联角点
    Frustum->>BBox: 计算包围盒中心
    BBox->>Matrix: 生成光源视图矩阵
    Matrix->>BBox: 计算光源空间包围盒
    BBox->>Matrix: 生成正交投影矩阵
    Matrix->>CSM: 存储VP矩阵
````

更新检测机制：采用多因素变化检测策略
```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> NeedsUpdate : 配置改变
    Idle --> DirectionChanged : 光源旋转>1°
    Idle --> CameraChanged : 相机矩阵变化
    DirectionChanged --> NeedsUpdate
    CameraChanged --> NeedsUpdate
    NeedsUpdate --> Updated : CalculateCascadeMatrices
    Updated --> Idle
````

变化检测算法：

- 方向变化： $\theta = \arccos(\vec{d}{\text{new}} \cdot \vec{d}{\text{old}}) > 1^\circ$
- 相机变化： $|M_{\text{new}} - M_{\text{old}}|_F > 0.01$

### Spot Light聚光灯

SpotLight继承自Light基类，通过重写策略方法实现聚光灯特定行为
| 字段组 | 内容 | 大小 |
|--------|------|------|
| 基础属性 | color, intensity, position, type | 48字节 |
| 聚光灯参数 | range, innerAngle, outerAngle, blend | 16字节 |

SpotLight影响半径特性：
- 聚光灯有明确的照射范围
- 影响半径等于设置的range参数
- 超出范围的物体不受光照影响

ShadowMap矩阵计算公式：

- 视图矩阵：lookAt(lightPosition, lightPosition + lightDirection, up)
- 投影矩阵：perspective(fov, aspect, near, far)

### Light Manager光源管理器

LightManager采用单例模式统一管理所有光源实例
```mermaid
classDiagram
    class LightManager {
        -m_Lights: vector~shared_ptr~Light~~
        -m_LightSSBO: shared_ptr~LightShaderStorgeBuffer~
        -m_ShadowInstance: shared_ptr~ShadowInstance~
        +Get() LightManager&
        +Initialize() bool
        +CreatePointLight() shared_ptr~Light~
        +UpdateLightData() bool
    }
    
    LightManager --> Light : 管理
    LightManager --> LightShaderStorgeBuffer : 使用
    LightManager --> ShadowInstance : 使用
````

光源数据更新流程
```mermaid
sequenceDiagram
    participant Scene as 场景系统
    participant LM as LightManager
    participant SSBO as LightSSBO
    participant SI as ShadowInstance
    
    Scene->>LM: UpdateLightData(transforms)
    LM->>LM: PrepareGPULightData
    LM->>SSBO: UpdateLights(gpuData)
    Scene->>LM: UpdateLightShadowUBO(camera)
    LM->>SI: UpdateUBO(lights, transforms, camera)
````


内存管理
- 共享所有权：使用shared_ptr管理光源生命周期
- 连续存储：GPU数据连续排列，提高传输效率
- 预分配策略：SSBO预分配最大容量，避免动态调整

事件驱动集成：通过事件系统与其他模块协同工作
- LightSSBOCreateEvent：通知渲染器SSBO创建
- 光源状态变化事件：支持动态光源管理

Light模块通过统一的光源基类设计和类型特定的策略实现，为引擎提供了完整的光照解决方案。该模块支持多种光源类型和对应的阴影技术，通过高效的管理器和GPU数据优化，确保了大规模光照场景下的性能和视觉效果。

## Input 输入模块

Input模块是MiteEngine的用户交互核心，负责统一管理键盘、鼠标等输入设备的事件处理和状态跟踪。该模块采用上下文栈设计，支持多层次的输入优先级和灵活的输入阻断机制，为不同UI状态和游戏模式提供精确的输入控制。

Input模块采用分层设计，通过统一的接口管理输入事件流
```mermaid
classDiagram
    class InputManager {
        +Init() void
        +PushContext() void
        +PopContext() void
        +ProcessEvent() void
    }
    
    class InputContextStack {
        +Push() void
        +Pop() void
        +ProcessEvent() void
    }
    
    class InputContext {
        +ProcessEvent() void
        +SetBlockInput() void
    }
    
    class InputStateTracker {
        +OnKeyPressed() void
        +IsKeyPressed() bool
        +GetPressedKeys() unordered_set
    }
    
    InputManager --> InputContextStack : 管理
    InputContextStack --> InputContext : 包含
    InputManager --> InputStateTracker : 使用
````

### Input Manager输入管理器

InputManager作为输入系统的中心协调者
```mermaid
sequenceDiagram
    participant ImguiViewport as Imgui Viewport
    participant EB as EventBus
    participant IM as InputManager
    participant ICS as InputContextStack
    participant IC as InputContext
    
    ImguiViewport->>EB: 发布输入事件
    EB->>IM: ProcessEvent
    IM->>ICS: ProcessEvent
    ICS->>IC: 从栈顶向下处理
    IC->>IC: 具体事件处理
````

注意：输入事件的发布者由最早期设计的GLFW窗口（在Window模块还遗留有相关事件发布代码），改为UI模块的Imgui Input Producer，且仅ViewPort Panel窗口设立输入上下文，其他窗口（如SceneTree场景树、Properties属性页）由Imgui内部处理事件逻辑。

### Input Context/Stack 输入上下文/上下文栈

上下文栈机制：采用LIFO（后进先出）栈结构管理输入优先级

处理规则：
- 从栈顶向下遍历处理
- 阻塞上下文停止事件传播
- 处理完成的上下文可标记事件为已消费

由于输入事件完全由Imgui Input Producer发布，输入上下文也仅剩Viewport Input Context，上下文栈也就没有了实际作用。待后续规划游戏上下文/UI上下文等多输入上下文的框架后启用

### Input State Tracker输入状态跟踪器

输入状态跟踪器的职责：
1. 跟踪键盘和鼠标按键的按下/释放状态（处理长按逻辑）
2. 记录按键按下的时间戳
3. 处理Timer的自洁行为
4. 提供当前激活按键的查询接口

输入状态管理：
```mermaid
stateDiagram-v2
    [*] --> NoInput
    NoInput --> KeyPressed : OnKeyPressed
    KeyPressed --> NoInput : OnKeyReleased
    KeyPressed --> MultipleKeys : 多键按下
    MultipleKeys --> KeyPressed : 部分释放
    MultipleKeys --> NoInput : 全部释放
````

Input模块设计之初是希望通过灵活的上下文栈设计和精确的状态跟踪机制，为引擎提供用户交互能力。后续其他模块的开发过程则一步步地削弱了Input模块的存在感。在引擎进一步开发输入相关功能之前，Input暂时保留基本功能即可。

## Scene Core场景核心模块

SceneCore模块是MiteEngine引擎的ECS（Entity-Component-System）架构实现核心，作为引擎场景管理的基石层，承担着以下核心职责

实体组件系统管理：
- 提供完整的ECS框架实现，包括Entity、Component、System三大核心要素
- 管理场景中所有实体的生命周期，从创建、状态更新到销毁的全流程管控
- 实现类型安全的组件注册、存储和访问机制，确保数据完整性

数据驱动场景架构：
- 采用数据导向设计（Data-Oriented Design）原则，优化内存访问模式
- 实现组件数据的连续内存布局，最大化缓存利用率
- 提供高效的实体查询和组件访问接口，支持复杂场景数据操作

事件驱动状态同步：
- 深度集成引擎事件系统，实现组件状态的实时同步和事件响应
- 通过组件事件发布器实现跨模块的状态变更通知
- 支持组件快照机制，为场景序列化和状态恢复提供基础

组合优于继承的核心思想：
- 通过灵活组合不同的Component来构建复杂的实体行为
- 避免深层次的继承树，提高代码的灵活性和可维护性
- 支持运行时的动态组件添加和移除

### Component组件基类

组件作为纯数据载体，存储实体的属性，例如位置、速度等，不包含逻辑。

SceneCore采用多层级组件基类设计，通过模板特征模式实现类型安全的组件系统：
```mermaid
classDiagram
    class Component {
        <<abstract>>
        +GetFamily() Family
        +GetType() type_index
        +GetDependencies() vector~type_index~
        +Serialize(ostream) bool
        +Deserialize(istream) bool
    }
    
    class SnapshotComponent {
        <<abstract>>
        +CreateSnapshot() unique_ptr~ISnapshot~
        +ApplySnapshot(T) bool
    }
    
    class DirtyComponent {
        <<abstract>>
        +MarkDirty() void
        +IsDirty() bool
        +ProcessDirty(float, SceneRegistry) void
    }
    
    class ComponentTraits~T,F~ {
        +family: constexpr Family
    }
    
    Component <|-- SnapshotComponent
    Component <|-- DirtyComponent
    Component <|-- ComponentTraits
    SnapshotComponent <|-- SnapshotComponentTraits
    DirtyComponent <|-- DirtyComponentTraits
````

组件家族优先级系统：采用枚举驱动执行顺序，确保组件更新的正确依赖关系
| 家族类型 | 优先级值 | 执行顺序 | 典型组件 |
|---------|----------|----------|----------|
| Core | 0 | 最先执行 | ID组件、标签组件 |
| Hierarchy | 10 | 早期 | 父子关系组件 |
| Transform | 20 | 中期 | 变换组件 |
| Geometry | 30 | 中后期 | 网格、包围盒组件 |
| Visibility | 40 | 后期 | 可见性组件 |
| Render | 60 | 渲染前 | 材质、光照组件 |
| Cleanup | 80 | 清理阶段 | 销毁标记组件 |
| PostUpdate | 255 | 最后执行 | 特殊用途组件 |

核心设计模式：

1. 模板特征模式
```cpp
template<typename T, Component::Family F>
class ComponentTraits : public Component {
    static constexpr Family family = F;
    // 编译期类型信息，零运行时开销
};
````

2. 策略模式
- 快照策略: SnapshotComponent 提供撤销/重做能力
- 脏标记策略: DirtyComponent 实现增量更新优化
- 序列化策略: 统一的二进制序列化接口

3. 观察者模式（通过）
```mermaid
sequenceDiagram
    participant Component
    participant EventBus
    participant SceneRegistry
    
    Component->>Component: MarkDirty()
    Component->>EventBus: 发布组件变更事件
    EventBus->>SceneRegistry: 通知脏组件状态
    SceneRegistry->>Component: 调用ProcessDirty()
````

### Entity 场景实体

场景实体仅作为唯一标识符存在，不包含任何数据或逻辑。它是组件的容器。

实体标识架构：Entity采用UUID唯一标识模式，确保全局唯一性和类型安全。
```mermaid
classDiagram
    class Entity {
        -m_Name: string
        -m_UUID: UUID
        +IsValid() bool
        +Destroy() void
        +GetUUID() UUID
        +GetName() string
        +operator==() bool
        +operator bool() explicit
    }
    
    class SceneRegistry {
        <<friend>>
        +CreateEntity() Entity
    }
````

核心设计模式：

1. 工厂模式
- 受控构造: 仅SceneRegistry友元可创建有效实体
- 空实体保护: 默认构造函数生成无效实体

```cpp
// 工厂方法 - 仅SceneRegistry可调用
static Entity CreateEntity(const std::string& name = "");
friend SceneRegistry;
````

2. 值对象模式
- 不可变性: 实体标识创建后不可修改
- 相等性比较: 基于UUID的等价关系
- 哈希支持: 可作为STL容器键值

内存与性能优化：

轻量级设计
- 最小数据成员: 仅包含名称和UUID（40+字节）
- 零动态分配: 所有成员栈上存储
- 拷贝高效: 浅拷贝语义，支持移动优化

哈希特化策略：确保实体可以作为哈希键进行查询
```cpp
namespace std {
    template<> struct hash<mite::Entity> {
        size_t operator()(const mite::Entity& entity) const {
            return hash<mite::UUID>()(entity.GetUUID());
        }
    };
}
````

访问控制架构：
| 操作类型 | 访问权限 | 设计意图 |
|---------|----------|----------|
| 实体创建 | `SceneRegistry`友元 | 确保实体来源可控 |
| 实体拷贝 | 公开 | 支持值语义传递 |
| 实体销毁 | 公开 | 允许手动生命周期管理 |
| 标识查询 | 公开 | 支持实体比较和哈希 |

Entity实体设计通过严格的访问控制和轻量级标识模式，为ECS架构提供了高效、安全的实体管理基础，同时确保与STL容器（主要是哈希表）的无缝集成。

### Scene Registry场景注册表
SceneRegistry采用分层哈希映射实现高效的组件存储与查询，第一层哈希表通过TypeIndex组件类型查找第二层，第二层哈希表通过Entity查询具体组件
```mermaid
classDiagram
    class SceneRegistry {
        -m_Components: ComponentTypeMap
        -m_ComponentMutex: shared_mutex
        -m_ComponentEventPublisher: ComponentEventPublisher
        +CreateEntity() Entity
        +AddComponent() T&
        +GetEntitiesWithAllOf() vector~Entity~
    }
    
    class ComponentTypeMap {
        <<unordered_map>>
        key: type_index
        value: ComponentMap
    }
    
    class ComponentMap {
        <<unordered_map>>
        key: Entity
        value: shared_ptr~Component~
    }
````

实体创建流程：创建实体后立即绑定负责唯一标识的ID组件和用于分类的Tag组件，并发布实体创建事件

```mermaid
sequenceDiagram
    participant Caller
    participant Registry
    participant Entity
    participant EventBus
    
    Caller->>Registry: CreateEntity(name)
    Registry->>Entity: CreateEntity(name)
    Registry->>Registry: AddComponent<IDComponent>
    Registry->>Registry: AddComponent<TagComponent>
    Registry->>EventBus: Publish(EntityCreatedEvent)
    Registry->>Caller: 返回Entity
````

组件操作状态机：负责指定实体对应的组件创建、获取、销毁等操作

```mermaid
stateDiagram-v2
    [*] --> EntityCheck
    EntityCheck --> Valid: IsValid(entity)
    EntityCheck --> Error: 无效实体
    
    Valid --> ComponentCheck: Add/Get/Remove
    ComponentCheck --> Exists: HasComponent
    ComponentCheck --> NotExists: Not HasComponent
    
    Exists --> Replace: AddComponent
    Exists --> Return: GetComponent
    Exists --> Remove: RemoveComponent
    
    NotExists --> Create: AddComponent
    NotExists --> Skip: RemoveComponent
    NotExists --> Error: GetComponent
````

多组件查询策略：折叠表达式
```cpp
template<typename... Components> 
std::vector<Entity> GetEntitiesWithAllOf() {
    // 1. 使用第一个组件类型作为基准集
    // 2. 遍历基准集检查其他组件存在性
    // 3. 折叠表达式实现编译期展开
}
````

### Event Publisher组件事件发布器

ComponentEventPublisher采用双重事件分发模式，实现组件生命周期事件的类型安全发布。主要负责实体和组件的创建/删除事件的自动发布。

核心设计模式：类型擦除的适配器模式
```cpp
// 类型擦除适配：将具体组件类型适配为通用Component接口
m_ConstructCallbacks[type] = [callback](Entity entity, Component& comp) {
    callback(entity, static_cast<T&>(comp));  // 安全向下转型
};
````

回调注册机制：
```cpp
template<typename T>
void RegisterComponentCallbacks() {
    // 构造事件：Component -> ComponentAddedEvent转换
    RegisterCallbackComponentConstruct<T>([this](Entity entity, T& component) {
        ComponentAddedEvent<T> event(entity, component);
        EventBus::Publish<ComponentAddedEvent<T>>(event);
    });
    
    // 销毁事件：Component -> ComponentRemovedEvent转换  
    RegisterCallbackComponentDestroy<T>([this](Entity entity, T& component) {
        ComponentRemovedEvent<T> event(entity, component);
        EventBus::Publish<ComponentRemovedEvent<T>>(event);
    });
}
````

回调管理接口：
| 方法 | 功能 | 使用场景 |
|------|------|----------|
| `RegisterComponentCallbacks` | 注册标准生命周期回调 | 组件类型初始化 |
| `UnregisterComponentCallbacks` | 注销特定类型回调 | 动态模块卸载 |
| `UnregisterCallbacks` | 清理所有回调 | 系统关闭 |

### Snapshot组件状态快照

ComponentSnapshot采用事件驱动快照模式，实现组件状态的序列化和撤销/重做功能（目前序列化和撤销/重做功能暂未开发完毕）
```mermaid
classDiagram
    class ISnapshot {
        <<interface>>
        +Apply() void
        +Revert() void
        +GetMemoryUsage() size_t
        +GetDescription() const char*
    }
    
    class ComponentSnapshot~DataT~ {
        -m_entityId: Entity
        -m_snapshotData: DataT
        +GetEntityId() Entity
        +GetData() const DataT&
    }
    
    class EventBus {
        <<external>>
        +Publish() void
    }
    
    ISnapshot <|-- ComponentSnapshot
    ComponentSnapshot --> EventBus : 发布ApplySnapshotEvent
````

快照生命周期：
```mermaid
sequenceDiagram
    participant UndoSystem as 撤销系统
    participant Snapshot as ComponentSnapshot
    participant EventBus as EventBus
    participant ComponentSystem as 组件系统
    
    UndoSystem->>Snapshot: Apply()
    Snapshot->>EventBus: Publish(ApplySnapshotEvent)
    EventBus->>ComponentSystem: 分发ApplySnapshotEvent
    ComponentSystem->>ComponentSystem: 更新组件数据
````

工厂方法优化：
- 类型推导: 自动推导模板参数
- 完美转发: 支持各种数据构造方式
- 异常安全: make_unique确保资源安全
```cpp
template<typename DataT>
std::unique_ptr<ComponentSnapshot<DataT>> 
CreateComponentSnapshot(Entity entityId, const DataT& data) {
    return std::make_unique<ComponentSnapshot<DataT>>(entityId, data);
}
````

撤销/重做语义：对称操作设计
- 状态恢复: 快照应用总是恢复到特定状态
- 幂等性: 多次应用同一快照结果不变
- 可逆操作: 支持完整的撤销/重做链条
```cpp
void Apply() override {
    EventBus::Publish<ApplySnapshotEvent<DataT>>(...);
}
void Revert() override {
    Apply();  // 撤销与应用语义相同
}
````
快照数据类型支持

| 数据类型 | 存储效率 | 适用场景 |
|---------|----------|----------|
| 基础类型 | 直接存储 | 数值、标志位 |
| 结构体 | 值语义 | 变换矩阵、颜色 |
| 容器类型 | 深拷贝 | 顶点数据、配置数组 |

### Component System组件系统

SceneCore采用多层级系统基类设计，通过策略模式实现不同类型的组件系统
```mermaid
classDiagram
    class IComponentSystem {
        <<interface>>
        +GetSystemType() type_index
        +GetExecutionOrder() Family
        +Initialize() void
        +Shutdown() void
        +Update() void
        +GetComponentTypes() vector~type_index~
        +GetSystemDependencies() vector~type_index~
    }
    
    class ComponentSystem~T~ {
        -m_AllComponents: unordered_map~Entity, T*~
        +GetAllComponents() vector~T*~
        +OnComponentAdded() void
        +OnComponentRemoved() void
    }
    
    class SnapshotComponentSystem~T~ {
        +OnSnapshotApplied() void
    }
    
    class DirtyComponentSystemBase {
        <<interface>>
        +Update() void
        +GetDirtyComponentCount() size_t
    }
    
    class DirtyComponentSystem~T~ {
        -m_DirtyComponents: vector~T*~
        +CollectDirtyComponents() void
        +ProcessDirtyComponents() void
        +MarkAllComponentsDirty() void
    }
    
    IComponentSystem <|-- ComponentSystem
    ComponentSystem <|-- SnapshotComponentSystem
    IComponentSystem <|-- DirtyComponentSystemBase
    ComponentSystem <|-- DirtyComponentSystem
    DirtyComponentSystemBase <|.. DirtyComponentSystem
````

核心设计模式：

策略模式：
1. 基础系统: ComponentSystem 管理组件生命周期
2. 快照系统: SnapshotComponentSystem 处理撤销/重做
3. 脏标记系统: DirtyComponentSystem 实现增量更新

模板方法：
```cpp
template<typename T>
class ComponentSystem : public IComponentSystem {
    // 通用组件管理 + 类型特定逻辑
};
````
事件驱动架构
```mermaid
sequenceDiagram
    participant EventBus
    participant System as ComponentSystem
    participant Registry as SceneRegistry
    
    EventBus->>System: ComponentAddedEvent
    System->>System: OnComponentAdded()
    System->>System: Register(entity, component)
    System->>Registry: 更新组件状态
````

脏标记系统更新流程：
```mermaid
flowchart LR
    A[Update调用] --> B[收集脏组件]
    B --> C[并行处理脏组件]
    C --> D[组件内部ProcessDirty]
    D --> E[清除脏标记]
````

内存管理策略：
| 系统类型 | 存储结构 | 访问模式 | 性能特性 |
|---------|----------|----------|----------|
| 基础系统 | `unordered_map<Entity, T*>` | O(1)查找 | 快速组件访问 |
| 脏标记系统 | `vector<T*>` | 连续迭代 | 缓存友好处理 |

类型安全机制：SFINAE组件类型检测
```cpp
template<typename T, typename = void>
struct HasComponentType : std::false_type {};

template<typename T>
struct HasComponentType<T, std::void_t<typename T::ComponentType>> 
    : std::true_type {};
````

### Component System Manager组件系统管理器

ComponentSystemManager采用双重存储结构实现高效的系统管理和依赖解析

系统注册流程：
```mermaid
flowchart LR
    D[创建新系统]
    D --> E[存储到双重结构]
    E --> F[标记需要重新排序]
    F --> G[注册组件事件回调]
    G --> H[返回系统指针]
````

拓扑排序算法：用于解决系统之间的依赖关系问题，将不同系统按照依赖从低到高排序，方便更新时，处在依赖链底端的系统优先更新
```cpp
void SortSystems() {
    // 1. 按执行顺序初步排序
    std::sort(m_Systems.begin(), m_Systems.end(), [](auto& a, auto& b) {
        return a->GetExecutionOrder() < b->GetExecutionOrder();
    });
    
    // 2. 拓扑排序调整依赖
    bool changed;
    do {
        changed = false;
        for (size_t i = 0; i < m_Systems.size(); ++i) {
            for (const auto& depType : m_Systems[i]->GetSystemDependencies()) {
                // 查找依赖系统位置
                auto depIt = find_system_by_type(depType);
                if (depIt > m_Systems.begin() + i) {
                    // 调整顺序确保依赖在前
                    rotate_systems(i, depIt);
                    changed = true;
                    break;
                }
            }
        }
    } while (changed);
}
````

系统初始化流程：
```mermaid
sequenceDiagram
    participant Manager as ComponentSystemManager
    participant Systems as 组件系统列表
    
    Manager->>Manager: SortSystems()拓扑排序
    loop 按顺序初始化
        Manager->>Systems: system->Initialize()
        Systems->>Systems: 订阅组件事件
    end
````

事件回调集成：确保组件注册之后无需向事件发布器注册事件回调
```cpp
template<typename T>
T* RegisterSystem(Args&&... args) {
    // 自动注册组件事件回调
    if constexpr (HasComponentType<T>::value) {
        using U = typename T::ComponentType;
        m_Registry.GetEventPublisher().RegisterComponentCallbacks<U>();
    }
}
````

### Scene Core Components组件具体实现

SceneCore模块实现了10个核心组件，通过统一的模板特征模式构建类型安全的ECS系统

核心基础组件
| 组件类型 | 家族 | 快照支持 | 主要职责 |
|---------|------|----------|----------|
| **IDComponent** | Core | ❌ | 实体唯一标识，持久化引用 |
| **TagComponent** | Core | ❌ | 实体名称标签，编辑器显示 |
| **DestroyComponent** | Cleanup | ❌ | 延迟销毁标记，生命周期管理 |

几何与渲染组件
| 组件类型 | 家族 | 快照支持 | 主要职责 |
|---------|------|----------|----------|
| **TransformComponent** | Transform | ✅ | 局部空间变换，矩阵计算 |
| **CameraComponent** | Render | ✅ | 摄像机参数，投影矩阵 |
| **BoundingVolumeComponent** | Geometry | ✅ | 包围体数据，碰撞检测 |
| **MeshComponent** | Geometry | ✅ | 网格数据引用，LOD管理 |
| **MaterialComponent** | Geometry | ✅ | 材质实例，渲染参数 |
| **LightComponent** | Geometry | ✅ | 光源属性，光照计算 |
| **VisibilityComponent** | Visibility | ✅ | 可见性掩码，分层渲染 |

模板特征模式应用：所有组件通过模板特征基类实现统一的类型系统
```cpp
// 基础组件特征
template<typename T, Component::Family F>
class ComponentTraits : public Component {
    static constexpr Family family = F;
};
// 快照组件特征  
template<typename T, Component::Family F>
class SnapshotComponentTraits : public SnapshotComponent {
    static constexpr Family family = F;
};
````

核心算法特性：
- 快照与撤销系统：快照组件实现事件驱动的状态恢复
- 脏标记优化：特定组件系统实现增量更新策略
- 类型安全事件系统：组件通过模板事件实现类型安全的通信

### Scene Core对外接口

SceneCore作为ECS架构的统一入口点，协调管理所有子系统

核心设计模式：

外观模式：SceneCore作为统一外观，封装内部复杂子系统
```cpp
// 简化外部调用接口
Entity CreateEntity(const std::string& name = "");
void OnUpdate(float timestep);
void DestroyEntity(Entity entity);
````

依赖注入模式：通过引用传递提供模块访问
```cpp
SceneRegistry& GetRegistry() { return m_Registry; }
ComponentSystemManager& GetComponentSystemManager() { return m_SystemManager; }
````

初始化流程：
```mermaid
sequenceDiagram
    participant App as 应用程序
    participant Scene as SceneCore
    participant Registry as SceneRegistry
    participant Manager as ComponentSystemManager
    
    App->>Scene: 构造SceneCore
    Scene->>Scene: RegisterComponentSystems()
    Scene->>Manager: 注册10个核心系统
    Manager->>Registry: 关联事件发布器
    
    App->>Scene: InitializeComponentSystems()
    Scene->>Manager: InitializeAll()
    Manager->>各个系统: 按拓扑顺序初始化
````

更新循环架构：每帧执行
```cpp
void OnUpdate(float timestep) {
    // 阶段1: 更新脏组件系统
    m_SystemManager.UpdateDirtyComponentSystems(timestep);
    
    // 阶段2: 处理延迟销毁
    auto entities = m_Registry.GetEntitiesWith<DestroyComponent>();
    for (auto entity : entities) {
        m_Registry.DestroyEntity(entity);
    }
}
````

SceneCore作为承上启下的核心层，既为上层场景管理提供数据基础，又依赖下层模块的基础设施支持，形成了清晰的架构边界和职责分离。通过高效的ECS实现，为引擎提供了灵活、高性能的场景对象管理能力，是构建复杂3D场景的技术基石。

## Scene Graph场景图模块

SceneGraph模块是MiteEngine引擎的空间管理与层级关系核心，负责维护场景节点树结构、实现高效的空间查询加速，为渲染系统和编辑器提供基础的空间数据支持。该模块通过事件驱动机制与ECS架构深度集成，实现自动化的场景图同步和空间结构优化。

### Scene Node场景节点/Manager场景节点管理器

SceneNode模块采用事件驱动的层级管理架构，负责管理各个节点的父子关系，实现ECS实体与场景图节点的双向同步
```mermaid
classDiagram
    class SceneNode {
        -m_Entity: Entity
        -m_Parent: SceneNode*
        -m_Children: vector~SceneNode*~
        -m_WorldTransform: Transform
        -m_WorldBounds: BoundingVolume
        +UpdateWorldTransform() void
        +UpdateWorldBounds() void
        +UpdateVisibility() void
    }
    
    class SceneNodeManager {
        -m_EntityToNodeMap: unordered_map~Entity, unique_ptr~SceneNode~~
        -m_SpatialPartition: SpatialPartition&
        -m_DirtyNodes: unordered_set~SceneNode*~
        +CreateNode() SceneNode*
        +Update() void
        +MarkNodeDirty() void
    }
    
    class SpatialPartition {
        <<interface>>
        +Insert() void
        +Update() void
        +Remove() void
    }
    
    SceneNodeManager --> SceneNode : 管理
    SceneNodeManager --> SpatialPartition : 依赖注入
````

节点生命周期管理：节点的创建流程
```mermaid
sequenceDiagram
    participant Registry as SceneRegistry
    participant Manager as SceneNodeManager
    participant Node as SceneNode
    participant Spatial as SpatialPartition
    
    Registry->>Manager: CreateNode(entity)
    Manager->>Node: 构造SceneNode
    Manager->>Node: Update(registry, true)
    Node->>Registry: 查询TransformComponent
    Node->>Registry: 查询BoundingVolumeComponent
    Node->>Registry: 查询VisibilityComponent
    Manager->>Spatial: Insert(node)
    Manager->>Manager: 更新路径缓存
````

节点的销毁流程：
```cpp
bool DestroyNode(SceneRegistry& registry, Entity entity) {
    // 1. 从空间划分移除
    m_SpatialPartition.Remove(node);
    // 2. 处理父子关系（子节点提升为根节点）
    for (SceneNode* child : children) {
        SetParent(child, nullptr);
    }
    // 3. 清理映射关系
    m_EntityToNodeMap.erase(it);
}
````

脏标记传播机制
```mermaid
flowchart LR
    A[TransformComponent更新] --> B[标记节点Transform/Bounds Dirty]
    B --> D[递归标记所有子节点]
    D --> E[加入脏节点集合]
    E --> F[Update阶段批量处理]
````

事件处理策略：

同步事件处理：
| 事件类型 | 处理策略 | 优先级 | 影响范围 |
|---------|----------|--------|----------|
| TransformUpdated | 立即标记脏 | High | 当前节点+所有子节点 |
| BoundingVolumeChanged | 立即标记脏 | Normal | 当前节点 |
| VisibilityChanged | 立即标记脏 | High | 当前节点+所有子节点 |

延迟事件处理：
```cpp
// 父子关系变更延迟处理，避免递归深度问题
m_EventSubscriptions.SubscribeDeferred<SceneNodeParentChangeEvent>(
    BIND_DISPATCH_FN(OnSceneNodeParentChange));
````

### Spatiral Partition空间划分

SpatialPartition模块采用策略模式提供统一的空间查询接口，支持多种空间划分算法的运行时切换（目前仅实现BVH一个结构，所以切换部分仅为早期设计，未发挥实际作用）

空间划分类型对比：
| 划分类型 | 空间维度 | 构建复杂度 | 查询性能 | 适用场景 |
|---------|----------|------------|----------|----------|
| **BVH**(目前实现) | 3D | O(n log n) | O(log n) | 动态场景，通用性强 |
| **四叉树** | 2D | O(n log n) | O(log n) | 地形、平面场景 |
| **八叉树** | 3D | O(n log n) | O(log n) | 静态体积场景 |
| **均匀网格** | 3D | O(n) | O(1) | 均匀分布对象 |
| **KD树** | 3D | O(n log n) | O(log n) | 光线追踪，静态场景 |

查询接口语义设计：

射线检测策略：
```cpp
virtual bool Raycast(const Ray& ray, std::vector<SceneNode*>& results) = 0;
virtual bool RaycastFirst(const Ray& ray, SceneNode*& result, float& distance) = 0;
````
应用场景：
- Raycast: 编辑器多对象选择
- RaycastFirst: 游戏交互点击检测

视锥体裁剪优化：
```cpp
virtual size_t FrustumCull(const Frustum& frustum, 
                          const uint32_t visibleMask,
                          std::vector<SceneNode*>& results) = 0;
````
性能优化:
- 可见性掩码: 支持分层渲染，减少测试次数
- 早期剔除: 利用空间结构快速排除不可见区域

体积查询语义：
```cpp
virtual size_t VolumeQuery(const BoundingVolume& volume, 
                          std::vector<SceneNode*>& results) = 0;
````
查询类型支持：
- AABB: 轴对齐包围盒查询
- 球体: 球形区域查询
- OBB: 定向包围盒查询

性能分析：
| 操作类型 | 平均复杂度 | 最坏情况 | 优化策略 |
|---------|------------|----------|----------|
| 插入节点 | O(log n) | O(n) | 增量更新 |
| 删除节点 | O(log n) | O(n) | 惰性删除 |
| 射线检测 | O(log n) | O(n) | 最佳优先遍历 |
| 视锥剔除 | O(log n) | O(n) | 广度优先遍历 |

该空间划分模块通过统一的接口设计和灵活的策略模式，为SceneGraph提供了高效的空间查询能力，同时保持良好的扩展性和可维护性。

### Bounding volume hierarchy层次包围体

[SimpleBVH](./src/engine/scene_graph/simple_bvh.h)实现了一个基于SAH优化的层次包围盒结构，采用自顶向下的构建策略和智能的增量更新机制

自顶向下构建算法：采用基于中位数分割的SAH近似构建平衡BVH树
```cpp
BVHNode* BuildTree(std::vector<SceneNode*>& nodes, int start, int end, int depth) {
    // 1. 计算合并包围盒
    BoundingVolumeAABB totalBounds;
    for (int i = start; i < end; ++i) {
        totalBounds.Expand(nodes[i]->GetWorldBounds().GetAABBApproximation());
    }
    // 2. 终止条件检查
    if (count <= m_MinLeafSize || depth >= m_MaxDepth) {
        // 创建叶子节点
        return CreateLeafNode(nodes, start, end);
    }
    // 3. 寻找最佳分割
    int bestAxis; float bestSplitPos;
    if (FindBestSplit(nodes, start, end, bestAxis, bestSplitPos)) {
        // 4. 分割节点并递归构建
        int splitIndex = PartitionNodes(nodes, start, end, bestAxis, bestSplitPos);
        node->left = BuildTree(nodes, start, splitIndex, depth + 1);
        node->right = BuildTree(nodes, splitIndex, end, depth + 1);
    }
}
````

增量更新策略：
```mermaid
flowchart LR
    A[SceneNode更新] --> B[标记为脏节点]
    B --> C{脏节点比例>25%?}
    C -->|是| D[标记完全重建]
    C -->|否| E[标记增量更新]
    
    D --> F[下次查询时重建]
    E --> G[查找受影响的BVH节点]
    G --> H[局部重构包围盒]
````

射线检测查询算法：最佳优先遍历
- 早期终止: 找到最近交点后可提前终止
- 减少测试: 优先测试更可能命中的节点
```cpp
void RaycastBestFirst(BVHNode* root, const Ray& ray, 
                     std::vector<SceneNode*>& results) const {
    std::priority_queue<NodeWithDistance> queue;  // 优先队列--最小堆
    queue.push({root, 0.0f});
    // 自顶向下处理最小堆
    while (!queue.empty()) {
        auto current = queue.top(); queue.pop();
        // 未击中，不继续处理该节点
        if (!ray.Intersects(current.node->bounds, t)) continue;
        
        if (current.node->IsLeaf()) {
            // 测试叶子节点中的所有场景节点
            TestLeafNodes(current.node, ray, results);
        } else {
            // 按距离排序子节点，近的优先
            SortChildrenByDistance(current.node, ray, queue);
        }
    }
}
````

视锥体裁剪：广度优先遍历
- 缓存友好: 连续内存访问模式
- 批量处理: 适合视锥体的大规模剔除
```cpp
void FrustumCullBFS(BVHNode* root, const Frustum& frustum,
                   uint32_t visibleMask, std::vector<SceneNode*>& results) const {
    std::queue<BVHNode*> queue;
    queue.push(root);
    
    while (!queue.empty()) {
        BVHNode* current = queue.front(); queue.pop();
        
        auto intersection = frustum.TestAABB(current->bounds);
        if (intersection == Outside) continue;
        
        if (current->IsLeaf()) {
            // 批量处理叶子节点
            ProcessLeafNodes(current, frustum, visibleMask, results);
        } else {
            // 广度优先扩展
            queue.push(current->left);
            queue.push(current->right);
        }
    }
}
````

增量更新机制：重构决策策略

| 更新场景 | 处理策略 | 性能影响 |
|---------|----------|----------|
| 单个节点移动 | 局部重构 | O(log n) |
| 25%节点更新 | 增量更新 | O(k log n) |
| 超过25%节点更新 | 完全重建 | O(n log n) |
| 结构变化 | 完全重建 | O(n log n) |

该BVH实现通过智能的构建策略、高效的查询算法和灵活的更新机制，为SceneGraph提供了高性能的空间查询能力，特别适合动态场景的实时渲染需求。

### Scene Graph场景图接口设计

SceneGraph作为场景图系统的统一外观，协调管理节点层级、空间划分和事件响应，为上层应用提供简洁高效的接口
```mermaid
classDiagram
    class SceneGraph {
        -m_NodeManager: unique_ptr~SceneNodeManager~
        -m_SpatialPartition: unique_ptr~SpatialPartition~
        -m_PendingCreateNodes: unordered_map~Entity, EntityProperties~
        -m_PendingDestroyNodes: unordered_set~Entity~
        +Initialize() void
        +Update(SceneRegistry&) void
        +CreateNode() SceneNode*
        +FrustumCull() vector~SceneNode*~
        +Raycast() vector~SceneNode*~
    }
    
    class SceneNodeManager {
        +CreateNode() SceneNode*
        +Update() void
        +GetAllNodes() vector~SceneNode*~
    }
    
    class SpatialPartition {
        <<interface>>
        +FrustumCull() size_t
        +Raycast() bool
    }
    
    SceneGraph --> SceneNodeManager : 委托
    SceneGraph --> SpatialPartition : 委托
````

延迟创建机制：采用拓扑排序的延迟创建策略确保正确的父子关系
```mermaid
sequenceDiagram
    participant ECS as ECS系统
    participant Graph as SceneGraph
    participant Manager as SceneNodeManager
    participant Spatial as SpatialPartition
    
    ECS->>Graph: EntityCreatedEvent
    Graph->>Graph: 加入待创建队列
    
    ECS->>Graph: TransformComponentAdded
    Graph->>Graph: 标记hasTransform=true
    
    ECS->>Graph: BoundingVolumeComponentAdded
    Graph->>Graph: 标记hasBoundingVolume=true
    
    Graph->>Graph: ProcessScheduledCreations()
    Graph->>Manager: CreateNode(registry, entity, parent)
    Manager->>Spatial: Insert(node)
````

拓扑排序算法：
```cpp
void ProcessScheduledCreationsAndDestruction(SceneRegistry& registry) {
    // 1. 构建依赖图
    std::unordered_map<Entity, int> inDegree;
    std::unordered_map<Entity, std::vector<Entity>> childMap;
    // 2. 拓扑排序
    std::queue<Entity> createQueue;
    for (const auto& [entity, props] : m_PendingCreateNodes) {
        if (inDegree[entity] == 0 && props.hasBoundingVolume && props.hasTransform) {
            createQueue.push(entity);
        }
    }
    // 3. 按顺序创建
    while (!createQueue.empty()) {
        Entity entity = createQueue.front();
        CreateNode(registry, entity, props.parent);
        // 更新子节点入度...
    }
}
````

分层查询接口设计：（目前仅使用了渲染剔除功能和编辑器单选功能，分别用于SceneView渲染队列构建和ViewPort Panel界面拾取）

| 查询类型 | 返回结果 | 适用场景 |
|---------|----------|----------|
| `FrustumCull` | 可见节点列表 | 渲染剔除 |
| `Raycast` | 所有相交节点 | 编辑器多选 |
| `RaycastFirst` | 第一个相交节点 | 编辑器单选 |
| `VolumeQuery` | 体积内节点 | 框选操作 |
| `PointQuery` | 包含点节点 | 精确定位 |

更新管理策略：统一更新流程
```cpp
void Update(SceneRegistry& registry) {
    // 1. 处理延迟创建和销毁
    ProcessScheduledCreationsAndDestruction(registry);
    // 2. 更新节点状态（包含空间划分更新）
    m_NodeManager->Update(registry);
    // 3. 同步光照数据
    UpdateLightData(registry);
}
````

该SceneGraph设计通过清晰的责任分离、智能的延迟创建机制和高效的查询接口，为MiteEngine提供了强大而灵活的场景管理能力，同时保持与ECS架构的深度集成。

## Scene View场景视图模块

SceneView模块是渲染系统的数据准备层，负责将场景图数据转换为渲染器可用的格式，实现视锥体剔除、渲染队列管理和拾取交互功能。该模块作为SceneGraph与Renderer之间的桥梁，专注于渲染数据的组织与优化。

### Renderable Item可渲染项/Builder可渲染项构建器

RenderableItem/Builder模块采用建造者模式实现场景节点到渲染数据的转换，通过缓存策略优化性能
```mermaid
classDiagram
    class RenderableItem {
        +entity: Entity
        +worldTransform: Transform
        +mesh: shared_ptr~MeshInstance~
        +material: shared_ptr~MaterialInstance~
        +distanceToCamera: float
        +renderLayer: uint32_t
    }
    
    class RenderableItemBuilder {
        -m_MeshInstanceCache: unordered_map~Entity, shared_ptr~MeshInstance~~
        +BuildFromSceneNodes() vector~RenderableItem~
        +BuildFromSceneNode() RenderableItem
        +GetOrCreateMeshInstance() shared_ptr~MeshInstance~
    }
    
    class SceneNode {
        +GetEntity() Entity
        +GetWorldTransform() Transform
    }
    
    RenderableItemBuilder --> RenderableItem : 构建
    RenderableItemBuilder --> SceneNode : 输入
````

核心设计模式--建造者模式：将复杂对象的构建过程分离，支持批量构建和单对象构建
```cpp
// 批量构建接口
std::vector<RenderableItem> BuildFromSceneNodes(
    SceneRegistry& registry,
    std::shared_ptr<CameraInstance> camera,
    const std::vector<SceneNode*>& sceneNodes);
// 单对象构建接口  
RenderableItem BuildFromSceneNode(
    SceneRegistry& registry,
    std::shared_ptr<CameraInstance> camera,
    SceneNode* sceneNode);
````

LOD选择算法：基于屏幕空间覆盖率的动态LOD选择，支持LOD阈值配置（暂时未向外部开放接口，仅在函数内部使用）
```cpp
// 基于阈值的LOD选择
constexpr float lodThresholds[] = {200.0f, 100.0f, 50.0f, 25.0f, 10.0f, 5.0f};
for (uint32_t i = 0; i < sizeof(lodThresholds)/sizeof(lodThresholds[0]); ++i) {
    if (screenCoverage < lodThresholds[i]) {
        selectedLOD = i + 1;
    } else {
        break;
    }
}
return selectedLOD;
````

缓存模式：通过MeshInstance生命周期管理，避免重复创建开销

| 缓存策略 | 命中条件 | 性能收益 |
|---------|----------|----------|
| Entity映射 | 同一实体重复构建 | 避免UBO重复创建 |
| 智能指针 | 共享资源引用 | 减少内存分配 |

构建流程：
```mermaid
flowchart LR
    subgraph 输入检查 [输入与检查]
        A[SceneNode输入] --> B{可渲染检查}
        B -->|否| C[跳过]
        B -->|是| D[MeshInstance获取]
    end
    
    subgraph 缓存处理 [缓存管理]
        D --> E{缓存命中?}
        E -->|是| F[返回缓存实例]
        E -->|否| G[创建新实例]
        G --> H[初始化UBO]
        H --> I[加入缓存]
    end
    
    subgraph 输出准备 [渲染准备]
        I --> J[材质提取]
        J --> K[距离计算]
        K --> L[RenderableItem输出]
    end
````

MeshInstance生命周期：
```mermaid
sequenceDiagram
    participant Builder as RenderableItemBuilder
    participant Cache as MeshInstance缓存
    participant Renderer as 渲染系统
    
    Builder->>Cache: 查询MeshInstance
    alt 缓存命中
        Cache-->>Builder: 返回缓存实例
        Builder->>Renderer: 更新UBO变换
    else 缓存未命中
        Builder->>Builder: 创建新MeshInstance
        Builder->>Renderer: 发布创建事件
        Builder->>Cache: 加入缓存
    end
````

该构建器通过智能的缓存机制和高效的LOD算法，为渲染系统提供了数据准备能力，同时保持架构的简洁性和可扩展性。

### Render Queue渲染队列

RenderQueue模块采用策略模式实现多队列管理和灵活排序，为渲染器提供优化的渲染数据组织方式。
```mermaid
classDiagram
    class RenderQueue {
        -m_OpaqueQueue: QueueData
        -m_TransparentQueue: QueueData
        -m_AlphaTestQueue: QueueData
        -m_CustomQueue: QueueData
        +AddItem() void
        +SortQueue() void
        +GetItems() vector~RenderableItem~
    }
    
    class QueueData {
        -items: vector~RenderableItem~
        -sortStrategy: SortStrategy
        -isVisible: bool
        -customSortFunc: function
    }
    
    class RenderableItem {
        +distanceToCamera: float
        +material: shared_ptr~MaterialInstance~
    }
    
    RenderQueue --> QueueData : 包含
    QueueData --> RenderableItem : 管理
````

策略模式：支持多种排序顺序
```cpp
enum class SortStrategy {
    None,         // 不排序
    FrontToBack,  // 从前到后（减少overdraw）
    BackToFront,  // 从后到前（透明混合）
    ByMaterial,   // 按材质排序（减少状态切换）
};
````

多队列分离模式：将渲染对象按渲染特性分类管理

| 队列类型 | 排序策略 | 渲染特性 | 性能优化目标 |
|---------|----------|----------|-------------|
| **Opaque** | FrontToBack | 不透明物体 | 减少overdraw |
| **Transparent** | BackToFront | 透明物体 | 正确alpha混合 |
| **AlphaTest** | ByMaterial | Alpha测试 | 减少状态切换 |
| **Custom** | 自定义 | 特殊效果 | 灵活扩展 |

### Scene View场景视图接口设计

SceneView模块作为渲染数据管道，协调场景图、相机系统和渲染队列，实现完整的渲染数据准备流程
```mermaid
classDiagram
    class SceneView {
        -m_SceneCore: SceneCore&
        -m_SceneGraph: SceneGraph&
        -m_Builder: RenderableItemBuilder
        -m_RenderQueue: shared_ptr~RenderQueue~
        -m_CameraInstance: shared_ptr~CameraInstance~
        +Update() void
        +Pick() bool
        +GetRenderQueue() shared_ptr~RenderQueue~
    }
    
    class SceneCore {
        +GetRegistry() SceneRegistry&
        +CreateEntity() Entity
    }
    
    class SceneGraph {
        +FrustumCull() vector~SceneNode*~
        +RaycastFirst() SceneNode*
    }
    
    class RenderableItemBuilder {
        +BuildFromSceneNodes() vector~RenderableItem~
    }
    
    class RenderQueue {
        +AddItems() void
        +SortAll() void
    }
    
    SceneView --> SceneCore : 依赖注入
    SceneView --> SceneGraph : 依赖注入
    SceneView --> RenderableItemBuilder : 组合
    SceneView --> RenderQueue : 组合
````

事件驱动架构：通过立即订阅模式响应外部交互
| 事件类型 | 处理逻辑 | 传播控制 |
|---------|----------|----------|
| `ViewportResize` | 更新相机宽高比 | Failed |
| `ViewportPicked` | 执行射线拾取 | HandledAndStop |
| `ViewportCameraUpdate` | 更新相机变换 | HandledAndStop |
| `ViewportPickedUpdate` | 更新选中对象 | HandledAndStop |
| `SceneNodeSelected` | 同步选中状态 | Handled |

渲染数据流水线：
```mermaid
flowchart LR
    A[SceneView::Update] --> B[更新相机UBO]
    B --> C[构建视锥体]
    C --> D[执行视锥体裁剪]
    D --> E[构建渲染项]
    E --> F[更新渲染队列]
    F --> G[排序渲染队列]
    G --> H[交付渲染数据]
````

交互功能设计：射线拾取系统
```cpp
bool Pick(glm::vec2 screenPosUV) {
    // 1. 生成屏幕射线
    Ray ray = Ray::GenerateRayFromScreenUV(screenPosUV, cameraView, cameraProjection);
    // 2. 执行射线检测
    SceneNode* node = m_SceneGraph.RaycastFirst(ray);
    if (node) {
        // 3. 发布选中事件
        EventBus::Publish<SceneNodeSelectedEvent>(SceneNodeSelectedEvent(node));
        return true;
    }
    return false;
}
````
目前该系统还存在较大的问题。这套射线拾取系统是基于场景图使用包围盒构建的BVH树简化实现的，仅进行了最简单的包围盒检测，导致鼠标命中的并不精确。

若在模型分布较为均匀的情况下，这套系统还能正常运行。但当多个Mesh的包围盒叠在一起，射线检测的精度就大大缩减。

正常的解决方案是：为每个网格创建ID，通过将ID编码为颜色，渲染到ID纹理上，这样CPU端就能根据鼠标位置实时获取SelectMesh，从而实现精确到像素级的拾取逻辑（待后续完善该模式）

变换同步机制：支持父子关系感知的变换更新。主要用于Gizmo小组件的拖拽操作。
```cpp
void SetPickedWorldTransform(const Transform& worldTransform) {
    SceneNode* pickedNode = m_SceneGraph.GetNode(m_PickedEntity);
    SceneNode* parent = pickedNode->GetParent();
    if (parent) {
        // 考虑父节点变换：Local = inv(Parent) * World
        Transform localTransform = glm::inverse(parent->GetWorldTransform()) * worldTransform;
        UpdateLocalTransform(localTransform);
    } else {
        // 直接更新本地变换
        UpdateLocalTransform(worldTransform);
    }
}
````

该SceneView设计通过职责分离和数据流水线，基于SceneGraph获取的原始数据，为渲染系统提供了渲染用的数据源。


## Render 渲染模块

Render模块是MiteEngine的渲染核心，基于OpenGL图形API，采用分层架构设计，实现渲染命令抽象、多API支持和可扩展的渲染管线。该模块通过统一的接口设计，并提供灵活的渲染阶段管理机制。

### Render Command渲染命令

RenderCommand模块采用命令模式实现渲染操作的抽象与封装，支持多线程安全的命令队列管理。

```mermaid
classDiagram
    class RenderCommand {
        <<abstract>>
        -m_CommandQueue: queue~Command~
        -m_QueueMutex: mutex
        +Clear() void
        +BindFrameBuffer() void
        +SetViewport() void
        +SubmitDrawCall() void
        +Flush() void
    }
    
    class OpenGLRenderCommand {
        +Init() void
        +ApplyOpenGLState() void
    }
    
    class Command {
        -type: CommandType
        -execute: function
        -debugName: string
    }
    
    RenderCommand <|-- OpenGLRenderCommand
    RenderCommand --> Command : 管理
````

核心设计模式：

命令模式：将渲染操作封装为可序列化的命令对象
```cpp
struct Command {
    CommandType type;              // 命令类型标识
    std::function<void()> execute; // 执行函数
    std::string debugName;         // 调试名称
};
````

策略模式：支持多种渲染API实现（目前仅有OpenGL）
| 命令类型 | 抽象接口 | OpenGL实现 |
|---------|----------|------------|
| 清屏 | `Clear()` | `glClear()` |
| 绑定帧缓冲 | `BindFrameBuffer()` | `glBindFramebuffer()` |
| 设置视口 | `SetViewport()` | `glViewport()` |
| 绘制网格 | `SubmitDrawCall()` | `glDrawElements()` |

执行流程控制：

```mermaid
flowchart LR
    A[命令提交]
    A --> C[入队命令]
    C --> D{Flush调用?}
    D -->|是| E[顺序执行]
    D -->|否| F[等待执行]
    E --> G[异常处理、命令出队]
````

入队命令（以绑定FBO为例）：
```cpp
void BindFrameBuffer(const std::shared_ptr<FrameBuffer>& framebuffer) {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    m_CommandQueue.push({
        CommandType::BindFrameBuffer,
        [framebuffer] { framebuffer->Bind(); },
        "BindFrameBuffer"
    });
}
````
Flush命令顺序执行：包含异常捕获机制
```cpp
void Flush() {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    while (!m_CommandQueue.empty()) {
        const auto& cmd = m_CommandQueue.front();
        try {
            cmd.execute();  // 批量执行
        } catch (const std::exception& e) {
            m_Logger->error("Failed to execute command {}: {}", 
                           cmd.debugName, e.what());
        }
        m_CommandQueue.pop();
    }
}
````

命令执行策略：
- 延迟执行: 命令加入队列，不立即执行
- 批量提交: 所有阶段完成后统一提交
- 错误处理: 命令执行异常捕获与日志记录

该渲染命令系统通过清晰的命令抽象和灵活的扩展机制，为渲染管线提供了稳定可靠的操作接口。

### Render Device渲染设备

Render Device采用抽象工厂模式实现跨API的GPU资源管理，通过事件驱动的资源委托机制和命令队列封装，为渲染系统提供统一的GPU资源操作接口
```mermaid
classDiagram
    class RenderDevice {
        <<abstract>>
        -m_EventSubscriptions: SubscriptionGroup
        +CreateTexture() TextureGPUHandle
        +CreateModel() ModelGPUHandle
        +BindMesh() void
        +DrawMeshLOD() void
        #OnModelLoaded() void
        #OnTextureLoaded() void
    }
    
    class OpenGLDevice {
        -m_ActiveTextures: unordered_set~GLuint~
        -m_ActiveVAOs: unordered_set~GLuint~
        -m_WhiteTexture: GLuint
        +CreateTexture() TextureGPUHandle
        +BindMesh() void
        +DrawIndexed() void
        +OnModelLoaded() void
        +OnTextureLoaded() void
    }
    
    class AssetManager {
        +LoadModel() void
        +LoadTexture() void
    }
    
    class RenderCommand {
        +SubmitDrawCall() void
        +BindTexture() void
    }
    
    RenderDevice <|-- OpenGLDevice
    AssetManager --> RenderDevice : 事件委托
    RenderCommand --> RenderDevice : 资源操作

````

核心设计模式：

事件驱动的资源委托模式：AssetManager通过事件发布-订阅机制委托RenderDevice创建GPU资源

```mermaid
graph LR
    A[AssetManager] -->|发布事件| B[事件总线]
    B -->|分发事件| C[RenderDevice]
    C -->|委托实现| D[OpenGLDevice]
    D -->|创建资源| E[GPU资源]
    D -->|更新句柄| A
````
事件驱动的资源委托模式：AssetManager通过发布-订阅机制委托RenderDevice创建GPU资源
```mermaid
sequenceDiagram
    participant AM as AssetManager
    participant EB as 事件总线
    participant RD as RenderDevice
    participant GL as OpenGLDevice
    
    AM->>EB: ModelLoadEvent
    EB->>RD: 分发事件
    RD->>GL: OnModelLoaded()
    GL->>GL: CreateModel()
    GL->>AM: 更新GPU句柄
````

事件处理流程：
1. Asset加载完成 → 发布ModelLoadEvent/TextureLoadEvent
2. RenderDevice订阅 → 接收事件并创建GPU资源
3. 资源创建完成 → 更新Asset中的GPU句柄
4. 事件消费完成 → 阻断事件传播

抽象工厂模式：提供跨API统一的资源创建接口
| 资源类型 | 抽象接口 | OpenGL实现 |
|---------|----------|------------|
| 纹理 | `CreateTexture()` | `glGenTextures()` |
| 模型 | `CreateModel()` | `glGenVertexArrays()` |
| 帧缓冲 | `CreateFrameBuffer()` | `glGenFramebuffers()` |
| 运行时纹理 | `CreateRuntimeTexture()` | `glTexImage2D()` |

资源委托机制：

模型资源委托：
```mermaid
flowchart LR
    A[AssetManager加载模型] --> B[发布ModelLoadEvent]
    B --> C[RenderDevice接收事件]
    C --> D[创建VAO/VBO/EBO]
    D --> E[更新ModelAsset句柄]
    E --> F[记录资源追踪]
````
纹理资源委托：
```mermaid
flowchart LR
    A[纹理创建请求] --> B{纹理类型}
    B -->|Asset纹理| C[发布外部纹理加载事件]
    B -->|运行时纹理| D[发布运行时纹理创建]
    C --> E[RenderDevice创建纹理]
    D --> E
    E --> F[回调返回句柄]
    F --> G[资源追踪记录]
````

资源句柄管理：
```mermaid
classDiagram
    class TextureGPUHandle {
        +apiHandle: uintptr_t
    }
    
    class ModelGPUHandle {
        +vertexArray: uintptr_t
        +vertexBuffer: uintptr_t  
        +indexBuffer: uintptr_t
        +bboxMin: vec3
        +bboxMax: vec3
    }
    
    class FrameBuffer {
        +GetID() uint32_t
        +Bind() void
        +Unbind() void
    }
````

资源生命周期追踪与清理策略：
```cpp
class OpenGLDevice {
private:
    std::unordered_set<GLuint> m_ActiveTextures;  // 活动纹理
    std::unordered_set<GLuint> m_ActiveVAOs;      // 活动顶点数组
    std::unordered_set<GLuint> m_ActiveVBOs;      // 活动顶点缓冲区
    std::unordered_set<GLuint> m_ActiveEBOs;      // 活动索引缓冲区
    std::unordered_set<GLuint> m_ActiveFBOs;      // 活动帧缓冲
};
````

- 析构时清理: 设备销毁时强制清理所有资源
- 泄漏检测: 统计未释放资源数量
- 防御性编程: 确保资源完全释放

网格渲染：

顶点属性映射：
| 属性类型 | 分量数 | 数据类型 | 用途 |
|----------|--------|----------|------|
| Position | 3 | GL_FLOAT | 顶点位置 |
| Normal | 3 | GL_FLOAT | 法线向量 |
| TexCoord | 2 | GL_FLOAT | 纹理坐标 |
| Tangent | 3 | GL_FLOAT | 切线向量 |

LOD支持：
```mermaid
flowchart TD
    A[网格数据] --> B[基础LOD]
    B --> C[LOD1]
    C --> D[LOD2]
    D --> E[...]
    
    F[相机距离] --> G[LOD选择]
    G --> H[绘制指定LOD]
````

该渲染设备设计通过事件驱动的资源委托机制，实现了Asset系统与GPU资源创建的高效解耦，为渲染管线提供了稳定可靠的GPU资源管理能力。

### Render Context渲染上下文

RenderContext模块采用上下文模式统一管理渲染数据流，通过分层纹理管理和阶段着色器注册机制，为渲染管线提供统一的数据共享接口。

核心设计模式：

上下文模式：提供统一的数据访问接口，解耦渲染阶段与数据源
```mermaid
flowchart TD
    A[SceneView] -->|设置数据| B[RenderContext]
    B -->|共享数据| C[GBufferStage]
    B -->|共享数据| D[ShadowMapStage]
    B -->|共享数据| E[DeferredLightingStage]
    C -->|输出纹理| B
    D -->|输出纹理| B
    E -->|输出纹理| B
````

注册表模式：管理阶段着色器注册和纹理资源映射

| 注册类型 | 数据结构 | 用途 |
|---------|----------|------|
| 阶段着色器 | `map<string, OpenGLShader>` | 阶段着色器管理 |
| GBuffer纹理 | `array<RuntimeTexturePtr>` | 几何缓冲纹理 |
| 阴影贴图 | `map<LightType, RuntimeTexturePtr>` | 阴影纹理 |
| 渲染目标 | `map<string, RuntimeTexturePtr>` | 自定义渲染目标 |

分层纹理管理系统：

G-Buffer纹理管理：采用固定数组索引管理几何缓冲纹理
```mermaid
graph LR
    A[GBuffer纹理] --> B[位置纹理]
    A --> C[法线纹理]
    A --> D[漫反射纹理]
    A --> E[金属粗糙度纹理]
    A --> F[深度纹理]
    
    B --> G[索引0]
    C --> H[索引1]
    D --> I[索引2]
    E --> J[索引3]
    F --> K[索引4]
````

阴影贴图管理：采用按光源类型分类的动态管理
| 光源类型 | 阴影贴图格式 | 用途 |
|----------|--------------|------|
| 方向光 | 2D纹理数组 | 级联阴影贴图 |
| 点光源 | 立方体贴图数组 | 全向阴影 |
| 聚光灯 | 2D纹理数组 | 锥形阴影 |

渲染目标管理：支持自定义命名的渲染目标
```cpp
// 设置自定义渲染目标
context.SetRenderTarget("BloomBuffer", bloomTexture);
context.SetRenderTarget("SSAOBuffer", ssaoTexture);
context.SetRenderTarget("MotionVector", motionVectorTexture);
// 后续阶段访问
auto bloomTex = context.GetRenderTarget("BloomBuffer");
auto ssaoTex = context.GetRenderTarget("SSAOBuffer");
````

纹理生命周期管理：
1. 每帧清理: 渲染开始前清空纹理引用
2. 引用计数: 纹理由创建者管理生命周期、
3. 防御性编程: 避免悬空指针

着色器生命周期管理：
1. 注册阶段: 渲染阶段初始化时注册着色器
2. 验证阶段: 检查着色器链接状态和UBO绑定
3. 使用阶段: 渲染时通过上下文获取着色器
4. 清理阶段: 上下文销毁时自动释放引用

数据共享策略：渲染数据流
```mermaid
flowchart TD
    subgraph Input [输入数据]
        A[RenderQueue]
        B[CameraInstance]
        C[LightManager]
    end
    
    subgraph Processing [处理阶段]
        D[GBufferStage]
        E[ShadowMapStage]
        F[DeferredLightingStage]
    end
    
    subgraph Output [输出纹理]
        G[GBuffer纹理]
        H[阴影贴图]
        I[最终渲染目标]
    end
    
    Input --> RenderContext
    RenderContext --> Processing
    Processing --> Output
    Output --> RenderContext
````

该渲染上下文设计通过统一的数据管理和分层纹理系统，为渲染管线提供了清晰的数据流接口，支持复杂的多阶段渲染流程和灵活的资源共享机制。

### Render Pipeline渲染管线架构

RenderPipeline模块采用**管道过滤器模式**实现多阶段渲染流程，通过可**配置的阶段管理**和**统一的状态控制**，构建现代渲染管线架构
```mermaid
classDiagram
    class RenderPipeline {
        <<abstract>>
        -m_Context: unique_ptr~RenderContext~
        -m_Stages: vector~unique_ptr~RenderStage~~
        -m_ClearColor: vec4
        +Initialize() void
        +BeginFrame() void
        +RenderScene() void
        +EndFrame() void
        +AddStage() void
    }
    
    class OpenGLPipeline {
        -m_DefaultState: shared_ptr~RenderState~
        -m_IsRenderingScene: bool
        +Initialize() void
        +RenderScene() void
    }
    
    class RenderStage {
        <<abstract>>
        +Execute() void
        +GetName() string
        +IsEnabled() bool
    }
    
    class RenderContext {
        +SetSceneData() void
        +RegisterStageShader() void
    }
    
    RenderPipeline <|-- OpenGLPipeline
    RenderPipeline --> RenderStage : 管理
    RenderPipeline --> RenderContext : 包含
    OpenGLPipeline --> RenderStage : 执行
````

核心设计模式：

管道过滤器模式 (Pipe-Filter Pattern)：构建可扩展的多阶段渲染流水线
```mermaid
flowchart TD
    A[输入数据] --> B[ShadowMapStage]
    B --> C[GBufferStage]
    C --> D[DeferredLightingStage]
    D --> E[ForwardStage]
    E --> F[输出结果]
    
    G[RenderContext] --> B
    G --> C
    G --> D
    G --> E
````

模板方法模式：定义标准化的渲染流程框架
```cpp
// 标准渲染流程模板
void RenderPipeline::RenderFrame() {
    BeginFrame();           // 帧开始
    RenderScene(...);       // 场景渲染
    EndFrame();             // 帧结束
}
````

阶段管理系统：

阶段注册和配置
```mermaid
sequenceDiagram
    participant Pipeline as OpenGLPipeline
    participant Context as RenderContext
    participant Stage as RenderStage
    participant Shader as ShaderCache
    
    Pipeline->>Shader: 获取着色器
    Pipeline->>Context: RegisterStageShader()
    Pipeline->>Pipeline: AddStage()
    Pipeline->>Stage: Initialize()
    Note over Pipeline,Stage: 阶段初始化
````

阶段配置表
| 阶段名称 | 着色器路径 | 功能描述 |
|---------|-----------|----------|
| ShadowMapStage | `shaders/shadowmap/*` | 阴影贴图生成 |
| GBufferStage | `shaders/gbuffer/*` | 几何缓冲填充 |
| DeferredLightingStage | `shaders/lighting/*` | 延迟光照计算 |
| ForwardStage | `shaders/forward/*` | 前向渲染 |

阶段启用控制
```cpp
// 动态启用/禁用阶段
pipeline.SetStageEnabled("ShadowMapStage", true);   // 启用阴影
pipeline.SetStageEnabled("ForwardStage", false);    // 禁用前向渲染
// 获取阶段引用
auto* stage = pipeline.GetStage("GBufferStage");
````

渲染流程控制
```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> BeginFrame: 帧开始
    BeginFrame --> RenderScene: 设置渲染状态
    RenderScene --> StageExecution: 执行阶段
    StageExecution --> EndFrame: 阶段完成
    EndFrame --> Idle: 帧结束
    
    state StageExecution {
        [*] --> ShadowMap
        ShadowMap --> GBuffer
        GBuffer --> DeferredLighting
        DeferredLighting --> Forward
        Forward --> [*]
    }
````

状态机设计：
```cpp
class OpenGLPipeline {
private:
    bool m_IsRenderingScene = false;  // 渲染状态标志
    
    void BeginFrame() {
        m_IsRenderingScene = true;
        // 清屏、设置默认状态...
    }
    
    void RenderScene(...) {
        if (!m_IsRenderingScene) {
            LOG_WARN("RenderScene called outside of scene rendering phase");
            return;
        }
        // 执行阶段渲染...
    }
    
    void EndFrame() {
        m_IsRenderingScene = false;
        // 重置状态、执行命令...
    }
};
````
该渲染管线设计通过清晰的阶段管理和统一的状态控制，为渲染引擎提供了灵活、可扩展的渲染架构，支持复杂的多阶段渲染流程和性能优化策略。

### ShadowMap Stage阴影贴图阶段

ShadowMap Stage采用分层渲染策略实现多光源阴影贴图生成，通过纹理数组技术和级联阴影映射，为现代渲染管线提供高质量的阴影支持
```mermaid
classDiagram
    direction LR
    
    class ShadowMapStage {
        +Initialize() void
        +Execute() void
        +Shutdown() void
        +SetShadowQuality() void
    }
    
    class FrameBufferManager {
        +DirectionalFBO
        +PointFBO
        +SpotFBO
    }
    
    class ShadowContextUBO {
        +LightIndex
        +CascadeIndex
        +FaceIndex
        +ShadowMapType
    }
    
    class LightManager {
        +GetDirectionalLights()
        +GetPointLights()
        +GetSpotLights()
    }
    
    ShadowMapStage --> FrameBufferManager : 管理
    ShadowMapStage --> ShadowContextUBO : 配置
    ShadowMapStage --> LightManager : 查询
    ShadowMapStage --> RenderContext : 输出
````

渲染执行流程：
```mermaid
flowchart TD
    subgraph Prepare
    direction LR
        Start[Execute开始] --> Validate[输入验证]
        Validate --> UpdateUBO[更新阴影UBO]
        UpdateUBO --> SetupState[设置渲染状态]
        SetupState --> BindShader[绑定着色器]
    end

    subgraph Render
    direction LR
        RenderDirectional[渲染方向光阴影]
        RenderDirectional --> RenderPoint[渲染点光源阴影]
        RenderPoint --> RenderSpot[渲染聚光灯阴影]
    end

    subgraph Post
    direction LR
        StoreContext[存储到上下文]
        StoreContext --> Flush[执行命令]
        Flush --> End[完成]
    end

    Prepare --> Render --> Post
````

分层渲染机制
```mermaid
sequenceDiagram
    participant Stage as ShadowMapStage
    participant FBO as FrameBuffer
    participant Command as RenderCommand
    participant Context as RenderContext
    
    Stage->>FBO: 绑定阴影FBO
    loop 每个光源
        loop 每个级联/面
            Stage->>Command: BindFrameBufferDepthLayer
            Stage->>Command: Clear深度缓冲
            Stage->>Stage: BindShadowRenderContext
            Stage->>Stage: RenderSceneToShadowMap
        end
    end
    Stage->>Context: StoreShadowMapsToContext
````

方向光阴影管理：
```mermaid
graph TD
    subgraph "2D纹理数组结构"
        direction LR
        A0[光源0-级联0] --> A1[光源0-级联1] --> A2[光源0-级联2]
        B0[光源1-级联0] --> B1[光源1-级联1] --> B2[光源1-级联2]
    end
    
    subgraph "级联阴影策略"
        direction LR
        C[近裁剪面] --> D[中距离] --> E[远距离]
    end
````

点光源阴影管理：
```mermaid
graph TB
    subgraph "立方体贴图数组"
    direction TB
        A[光源0] --> A0[+X面]
        A --> A1[-X面]
        A --> A2[+Y面]
        A --> A3[-Y面]
        A --> A4[+Z面]
        A --> A5[-Z面]
    end
````

该阴影贴图阶段通过先进的纹理数组技术和分层渲染策略，为渲染引擎提供了高效、高质量的多光源阴影支持。

### G-Buffer Stage几何缓冲阶段

GBufferStage采用多渲染目标技术实现几何信息采集，通过材质参数编码和分层渲染策略，为延迟渲染管线提供完整的几何数据
```mermaid
classDiagram
    direction LR
    
    class GBufferStage {
        +Initialize() void
        +Execute() void
        +Shutdown() void
        +GetGBuffer() GBufferPtr
    }
    
    class GBuffer {
        +GetFramebuffer() FrameBufferPtr
        +GetTexture() RuntimeTexturePtr
        +IsValid() bool
    }
    
    class RenderContext {
        +GetViewportSize() glm::vec2
        +SetGBufferTexture() void
    }
    
    class MaterialInstance {
        +BindUBO() void
    }
    
    GBufferStage --> GBuffer : 管理
    GBufferStage --> RenderContext : 交互
    GBufferStage --> MaterialInstance : 绑定
````

多渲染目标策略：采用MRT技术同时输出多个几何属性
| 渲染目标 | 存储格式 | 编码内容 | 用途 |
|---------|---------|----------|------|
| **位置缓冲区** | RGB32F | 世界空间位置 | 光照计算、深度重建 |
| **法线缓冲区** | RGB16F | 世界空间法线 | 法线贴图、光照方向 |
| **漫反射缓冲区** | RGBA8 | 基础颜色/反照率 | 漫反射光照 |
| **金属粗糙度缓冲区** | RG8 | 金属度 + 粗糙度 | PBR材质参数 |
| **深度缓冲区** | DEPTH24 | 深度值 | 深度测试、SSAO |

执行流程：
```mermaid
flowchart TD
    Start[Execute开始] --> Prepare[准备阶段]
    Prepare --> Render[渲染阶段]
    Render --> Post[后处理阶段]
    Post --> End[完成]

    subgraph Prepare [准备阶段]
        direction LR
        P1[检查初始化状态] --> P2[验证上下文有效性]
        P2 --> P3[获取GBuffer着色器]
        P3 --> P4[检查尺寸匹配]
        P4 -->|不匹配| P5[调整GBuffer尺寸]
        P4 -->|匹配| P6[绑定GBuffer FBO]
        P5 --> P6
    end

    subgraph Render [渲染阶段]
        direction LR
        R1[清除缓冲区] --> R2[绑定着色器]
        R2 --> R3[绑定相机UBO]
        R3 --> R4[渲染不透明队列]
        R4 --> R5[渲染Alpha测试队列]
    end

    subgraph Post [后处理阶段]
        direction LR
        Po1[解绑FBO] --> Po2[解绑着色器]
        Po2 --> Po3[存储纹理到上下文]
        Po3 --> Po4[发布完成事件]
    end
````

尺寸自适应机制：
```mermaid
sequenceDiagram
    participant Stage as GBufferStage
    participant GBuffer as GBuffer对象
    participant Context as RenderContext
    
    Stage->>Context: GetViewportSize()
    Context->>Stage: 返回当前视口尺寸
    Stage->>GBuffer: GetFramebuffer()->GetSize()
    GBuffer->>Stage: 返回当前GBuffer尺寸
    alt 尺寸不匹配
        Stage->>GBuffer: resize(新宽度, 新高度)
        GBuffer->>Stage: 调整完成
    end
````

G-Buffer纹理布局：
```mermaid
graph TB
    subgraph "GBuffer纹理结构"
        A0[位置缓冲区] --> A1[RGB32F<br/>世界空间位置]
        B0[法线缓冲区] --> B1[RGB16F<br/>世界空间法线]
        C0[漫反射缓冲区] --> C1[RGBA8<br/>基础颜色]
        D0[金属粗糙度缓冲区] --> D1[RG8<br/>金属度+粗糙度]
        E0[深度缓冲区] --> E1[DEPTH24<br/>深度值]
    end
    
    subgraph "渲染目标绑定"
        F[FrameBuffer] --> G[颜色附件0: 位置]
        F --> H[颜色附件1: 法线]
        F --> I[颜色附件2: 漫反射]
        F --> J[颜色附件3: 金属粗糙度]
        F --> K[深度附件: 深度]
    end
````

渲染队列处理：
```mermaid
flowchart TD
    Start[渲染不透明队列] --> GetQueue[获取队列项]
    GetQueue --> CheckEmpty[检查是否为空]
    CheckEmpty -->|空| End[跳过渲染]
    CheckEmpty -->|非空| SetState[设置不透明状态]
    
    SetState --> LoopStart[遍历队列项]
    LoopStart --> ValidateItem[验证渲染项]
    ValidateItem -->|无效| Skip[跳过此项]
    ValidateItem -->|有效| BindMaterial[绑定材质UBO]
    
    BindMaterial --> SubmitDraw[提交绘制调用]
    SubmitDraw --> NextItem[下一项]
    Skip --> NextItem
    
    NextItem -->|还有项| LoopStart
    NextItem -->|完成| Flush[执行命令]
    Flush --> End
````

该GBuffer阶段通过先进的多渲染目标技术和优化的状态管理，为延迟渲染管线提供了高效、高质量的几何数据采集能力，同时保持灵活的配置选项和良好的扩展性。

### Deferred Lighting Stage延迟光照阶段

DeferredLightingStage采用全屏四边形渲染技术实现延迟光照计算，通过多纹理采样和SSBO光源数据，为延迟渲染管线提供高效的光照计算能力
```mermaid
classDiagram
    direction LR
    
    class DeferredLightingStage {
        +Initialize() void
        +Execute() void
        +Shutdown() void
        +GetLightingOutputTexture() RuntimeTexturePtr
    }
    
    class LightingFramebuffer {
        +HDR颜色附件: RGBA16F
        +深度附件: DEPTH_COMPONENT16
        +IsComplete() bool
    }
    
    class GBufferTextures {
        +位置纹理
        +法线纹理
        +漫反射纹理
        +金属粗糙度纹理
        +深度纹理
    }
    
    class LightSSBO {
        +光源数据数组
        +Bind() void
    }
    
    DeferredLightingStage --> LightingFramebuffer : 管理
    DeferredLightingStage --> GBufferTextures : 采样
    DeferredLightingStage --> LightSSBO : 绑定
````

渲染流程：
```mermaid
flowchart TD
    Start[Execute开始] --> Prepare[准备阶段]
    Prepare --> Render[渲染阶段]
    Render --> Post[后处理阶段]
    Post --> End[完成]

    subgraph Prepare [准备阶段]
        direction LR
        P1[检查初始化状态] --> P2[验证上下文有效性]
        P2 --> P3[获取光照着色器]
        P3 --> P4[验证输入纹理]
        P4 --> P5[调整光照FBO尺寸]
        P5 --> P6[绑定光照FBO]
    end

    subgraph Render [渲染阶段]
        direction LR
        R1[清除缓冲区] --> R2[设置光照状态]
        R2 --> R3[绑定着色器]
        R3 --> R4[绑定相机UBO]
        R4 --> R5[绑定GBuffer纹理]
        R5 --> R6[绑定光源SSBO]
        R6 --> R7[绑定阴影纹理]
        R7 --> R8[渲染全屏四边形]
    end

    subgraph Post [后处理阶段]
        direction LR
        Po1[解绑着色器] --> Po2[解绑FBO]
        Po2 --> Po3[存储输出纹理]
        Po3 --> Po4[发布完成事件]
    end
````

纹理绑定策略：
```mermaid
graph TD
    subgraph "GBuffer纹理绑定"
        A[位置纹理] --> A1[RuntimeTextureType::GBuffer_WorldPosDepth]
        B[法线纹理] --> B1[RuntimeTextureType::GBuffer_NormalScale]
        C[漫反射纹理] --> C1[RuntimeTextureType::GBuffer_BaseColorMatType]
        D[金属粗糙度纹理] --> D1[RuntimeTextureType::GBuffer_MetallicRoughness]
        E[深度纹理] --> E1[RuntimeTextureType::GBuffer_Depth]
    end
    
    subgraph "阴影纹理绑定"
        F[方向光阴影] --> F1[RuntimeTextureType::ShadowMap_Directional]
        G[点光源阴影] --> G1[RuntimeTextureType::ShadowMap_Point]
        H[聚光灯阴影] --> H1[RuntimeTextureType::ShadowMap_Spot]
    end
````

光源数据处理：
```mermaid
sequenceDiagram
    participant Stage as DeferredLightingStage
    participant Context as RenderContext
    participant LightManager as LightManager
    participant SSBO as LightSSBO
    participant Command as RenderCommand
    
    Stage->>Context: GetLightManager()
    Context->>Stage: 返回LightManager引用
    Stage->>LightManager: IsInitialized()
    LightManager->>Stage: 返回初始化状态
    Stage->>SSBO: GetLightSSBO()
    SSBO->>Stage: 返回SSBO指针
    Stage->>Command: BindLightSSBO(SSBO)
````

该延迟光照阶段通过高效的全屏四边形渲染和优化的纹理绑定策略，为延迟渲染管线提供了高质量的光照计算能力，同时保持灵活的配置选项和良好的性能特性。

### Forward Stage前向渲染阶段

Render模块通过清晰的分层架构和灵活的渲染阶段设计，为MiteEngine提供了高性能、可扩展的渲染能力，支持现代渲染管线。