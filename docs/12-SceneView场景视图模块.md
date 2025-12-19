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