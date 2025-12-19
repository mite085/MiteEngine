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