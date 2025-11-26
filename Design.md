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
      7. [RuntimeTexture运行时纹理对象](#runtimetexture运行时纹理对象)
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

### RuntimeTexture运行时纹理对象

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