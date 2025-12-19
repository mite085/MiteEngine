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