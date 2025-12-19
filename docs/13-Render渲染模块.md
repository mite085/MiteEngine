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

纹理绑定：
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

ForwardStage采用混合渲染策略处理透明物体和特殊材质，通过深度复用和预乘Alpha混合，为延迟渲染管线提供完整的透明物体渲染支持
```mermaid
classDiagram
    direction LR
    
    class ForwardStage {
        +Initialize() void
        +Execute() void
        +Shutdown() void
        +RenderTransparentQueue() void
    }
    
    class ForwardFramebuffer {
        +颜色附件: RGBA16F
        +外部深度附件: GBuffer深度
        +IsComplete() bool
    }
    
    class GBufferDepth {
        +复用深度缓冲
        +避免深度冲突
    }
    
    class TransparentQueue {
        +按距离排序
        +Alpha混合渲染
    }
    
    ForwardStage --> ForwardFramebuffer : 管理
    ForwardStage --> GBufferDepth : 复用
    ForwardStage --> TransparentQueue : 渲染
````

深度复用：GBuffer深度共享策略
```mermaid
graph TD
    subgraph "深度缓冲复用架构"
        A[GBuffer阶段] --> B[生成深度缓冲]
        B --> C[深度纹理: DEPTH_COMPONENT16]
        C --> D[Forward阶段复用]
        D --> E[避免深度冲突]
        E --> F[正确深度排序]
    end
````
```mermaid
graph TD
    subgraph "深度绑定流程"
        G[每帧执行] --> H[获取GBuffer深度纹理]
        H --> I[绑定到Forward FBO]
        I --> J[验证绑定成功]
        J --> K[开始透明渲染]
    end
````

深度复用优势:
- 内存节省: 避免重复深度缓冲分配
- 深度一致性: 确保透明/不透明物体正确遮挡
- 性能优化: 减少深度缓冲清除开销

混合渲染策略：
| 渲染类型 | 处理策略 | 混合方式 |
|---------|----------|----------|
| **不透明物体** | GBuffer阶段处理 | 无混合 |
| **Alpha测试物体** | GBuffer阶段处理 | 丢弃片段 |
| **透明物体** | Forward阶段处理 | Alpha混合 |
| **特殊效果** | Forward阶段处理 | 自定义混合 |

该前向渲染阶段通过深度复用和优化的透明物体处理策略，为混合渲染管线提供了完整的透明物体支持，同时保持与延迟渲染的高度一致性和良好的性能特性。

###Blend Stage混合阶段

BlendStage采用全屏后处理技术实现最终图像合成，通过预乘Alpha混合算法将延迟光照结果与前向透明结果合并，输出最终渲染图像
```mermaid
classDiagram
    direction LR
    
    class BlendStage {
        +Initialize() void
        +Execute() void
        +Shutdown() void
        +GetBlendFramebuffer() FrameBufferPtr
    }
    
    class BlendFramebuffer {
        +颜色附件: RGBA16F
        +最终输出缓冲
        +IsComplete() bool
    }
    
    class InputTextures {
        +Deferred Lighting纹理
        +Forward透明纹理
    }
    
    class BlendShader {
        +预乘Alpha混合
        +最终色调映射
    }
    
    BlendStage --> BlendFramebuffer : 管理
    BlendStage --> InputTextures : 采样
    BlendStage --> BlendShader : 执行
````

混合算法：预乘Alpha混合
```mermaid
graph TB
    subgraph "混合算法原理"
        A[延迟光照结果] --> B[RGB: 完整光照]
        B --> C[Alpha: 1.0]
        
        D[前向透明结果] --> E[RGB: 预乘颜色]
        E --> F[Alpha: 透明度]
        
        G[混合公式] --> H["$$C_{final} = C_{forward} + C_{deferred} \times (1 - \alpha)$$"]
        H --> I[最终Alpha: 1.0]
    end
````

```mermaid
graph TB
    subgraph "着色器实现"
        J[采样延迟纹理] --> K[采样前向纹理]
        K --> L[应用混合公式]
        L --> M[输出最终颜色]
    end
````

混合公式:
- $C_{forward}$: 前向透明颜色（已预乘Alpha）
- $C_{deferred}$: 延迟光照颜色（不透明）
- $\alpha$: 前向纹理的Alpha通道
- 结果: 正确叠加的最终颜色，Alpha固定为1.0

调试支持：阴影贴图可视化
（通过修改[blend.glsl](./assets/shaders/blend/blend.frag.glsl)的主函数实现，在单独启用ShadowMap Preview Stage之前，仅支持手动修改着色器）

```mermaid
graph TB
    subgraph "调试模式切换"
        A[正常混合模式] --> B[输出最终图像]
        
        C[阴影调试模式] --> D[显示方向光级联]
        C --> E[显示点光源立方体贴图]
        C --> F[显示聚光灯阴影]
    end
    
    subgraph "可视化布局"
        G[方向光级联] --> H[2×2网格布局]
        I[点光源立方体贴图] --> J[3×4网格布局]
        K[聚光灯阴影] --> L[单纹理显示]
    end
````

Render模块通过清晰的分层架构和灵活的渲染阶段设计，为MiteEngine提供了高性能、可扩展的渲染能力，支持现代渲染管线。
