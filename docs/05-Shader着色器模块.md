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