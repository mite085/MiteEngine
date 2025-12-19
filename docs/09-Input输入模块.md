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