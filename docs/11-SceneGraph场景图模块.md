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