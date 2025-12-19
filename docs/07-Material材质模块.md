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