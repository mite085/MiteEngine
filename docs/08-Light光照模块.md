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