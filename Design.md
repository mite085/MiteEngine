# MiteEngine模块设计文档


## 目录
1. [MiteEngine模块设计文档](#miteengine模块设计文档)
   1. [目录](#目录)
   2. [系统架构总览](#系统架构总览)
   3. [系统概述](#系统概述)
      1. [Main程序入口](#main程序入口)
      2. [Application主循环](#application主循环)
      3. [Asset资产管理](#asset资产管理)
      4. [Engine核心引擎](#engine核心引擎)
      5. [Editor编辑模块](#editor编辑模块)
      6. [Runtime运行时模块](#runtime运行时模块)
   4. [Application主循环](#application主循环-1)
      1. [MainLoop主循环](#mainloop主循环)
   5. [Asset资产管理](#asset资产管理-1)
      1. [Asset资产管理时序图](#asset资产管理时序图)
      2. [AssetManager资产管理器](#assetmanager资产管理器)
      3. [AssetCache资产缓存](#assetcache资产缓存)
      4. [ModelLoader模型加载](#modelloader模型加载)
      5. [TextureLoader贴图加载](#textureloader贴图加载)
   6. [Engine核心引擎](#engine核心引擎-1)
      1. [Core基础模块](#core基础模块)
         1. [Event事件系统](#event事件系统)
         2. [FileSystem文件管理系统](#filesystem文件管理系统)
         3. [Logger日志系统](#logger日志系统)
         4. [Timer时间系统](#timer时间系统)
         5. [UUID唯一编码系统](#uuid唯一编码系统)
      2. [Data渲染数据模块](#data渲染数据模块)
         1. [Type基本类型](#type基本类型)
         2. [Data数据类型](#data数据类型)
         3. [Event渲染数据相关事件](#event渲染数据相关事件)
      3. [Input输入系统](#input输入系统)
      4. [Material材质系统](#material材质系统)
      5. [Renderer渲染系统](#renderer渲染系统)
      6. [SceneCore场景核心](#scenecore场景核心)
      7. [SceneView场景视图](#sceneview场景视图)
      8. [SceneGraph场景层级](#scenegraph场景层级)
      9. [SceneSerializer场景序列化](#sceneserializer场景序列化)
      10. [Window窗口管理](#window窗口管理)
      11. [UI用户界面](#ui用户界面)
   7. [Editor编辑器](#editor编辑器)
   8. [Runtime运行时](#runtime运行时)
---

## 系统架构总览
```mermaid
graph TD
    %% 最底层模块
    Core[Core模块]
    Event[Event模块]
    
    %% 第二层：仅依赖最底层
    Input[Input模块] --> Core
    Input --> Event
    Data[Data模块] --> Core
    Data --> Event
    
    %% 第三层：依赖Data和最底层
    Material[Material模块] --> Data
    Light[Light模块] --> Data
    
    %% 第四层：依赖Material/Light和SceneCore
    Asset[Asset模块] --> Data
    Asset --> Material
    SceneCore[SceneCore模块] --> Material
    SceneCore --> Data
    SceneCore --> Light
    
    %% 第五层：依赖SceneCore
    SceneGraph[SceneGraph模块] --> SceneCore
    SceneSerializer[SceneSerializer模块-未开发] --> SceneCore
    
    %% 第六层：依赖SceneGraph和SceneCore
    SceneView[SceneView模块] --> SceneGraph
    
    %% 第七层：依赖SceneView
    Renderer[Renderer模块] --> SceneView
    
    %% 第八层：依赖最底层和Input
    Window[Window模块] --> Input
    
    %% 第九层：依赖Renderer和Window
    UI[UI模块] --> Renderer
    UI --> Window
    
    %% 最顶层：依赖所有功能模块
    Application[Application模块] --> Asset
    Application --> UI

    %% 样式定义
    classDef bottom fill:#000000
    classDef middle fill:#000000
    classDef top fill:#000000
    
    class Core,Event bottom
    class Input,Data,Command,Material,Light,Asset,SceneCore,SceneGraph,SceneSerializer,SceneView,Renderer,Window middle
    class UI,Application top
````
## 系统概述
> 本系统xxx  
> 本系统xxx

### Main程序入口
> 本系统xxx  
> 本系统xxx

### Application主循环
> 本系统xxx  
> 本系统xxx

### Asset资产管理
> 本系统xxx  
> 本系统xxx

### Engine核心引擎
> 本系统xxx  
> 本系统xxx

### Editor编辑模块
> 本系统xxx  
> 本系统xxx

### Runtime运行时模块
> 本系统xxx  
> 本系统xxx

## Application主循环
### MainLoop主循环
> 本系统xxx  
> 本系统xxx

## Asset资产管理
### Asset资产管理时序图
```mermaid
sequenceDiagram
    participant AssetManager
    participant ModelLoader
    participant EventBus
    
    AssetManager->>ModelLoader: 申请加载模型
    ModelLoader->>EventBus: 发布模型加载事件
    EventBus-->>AssetManager: 返回xxx
````

### AssetManager资产管理器
> 本系统xxx  
> 本系统xxx

### AssetCache资产缓存
> 本系统xxx  
> 本系统xxx

### ModelLoader模型加载
> 本系统xxx  
> 本系统xxx

### TextureLoader贴图加载
> 本系统xxx  
> 本系统xxx

## Engine核心引擎
### Core基础模块
#### Event事件系统
> 本系统xxx  
> 本系统xxx

#### FileSystem文件管理系统
> 本系统xxx  
> 本系统xxx

#### Logger日志系统
> 本系统xxx  
> 本系统xxx

#### Timer时间系统
> 本系统xxx  
> 本系统xxx

#### UUID唯一编码系统
> 本系统xxx  
> 本系统xxx

### Data渲染数据模块
> 本系统xxx  
> 本系统xxx

#### Type基本类型
> 本系统xxx  
> 本系统xxx

#### Data数据类型
> 本系统xxx  
> 本系统xxx

#### Event渲染数据相关事件
> 本系统xxx  
> 本系统xxx

### Input输入系统
> 键盘/鼠标输入处理流程
```mermaid
graph TB
    A[GLFWWindow] --> B[Input Events<br/>Mouse/Key]
    
    B --> C[EventBus<br/>事件总线]
    
    C --> D[ImGui InputAdapter<br/>UI输入路由]
    D --> E{WantCapture?}
    E -->|true| F[ImGui Widgets<br/>UI交互]
    E -->|false| G[Input Contexts<br/>业务输入映射]
    
    G --> H[Input Processor<br/>命令管理器]
    H --> I[Command System<br/>业务操作封装]
    
    I --> J[ECS Components<br/>数据修改]
    J --> K[Component Events<br/>数据变更事件]
    
    K --> L[Render System<br/>渲染同步]
    K --> M[Other Systems<br/>其他系统响应]
    
    F --> N[UI Command<br/>UI操作命令]
    N --> H

````

```mermaid
sequenceDiagram
    participant EventBus
    participant ModularInputContext
    participant InputProcessor

    EventBus->>ModularInputContext: 1. 调用 _HandleEvent(e)
    ModularInputContext->>ModularInputContext: 2. 按优先级排序处理器，检查事件订阅
    ModularInputContext->>EventBus: 3. 返回 e.handled 标记
    EventBus->>InputProcessor: 4. 根据 e.handled 决定是否调用处理器的订阅函数
````
### Material材质系统
### Renderer渲染系统
### SceneCore场景核心
### SceneView场景视图
### SceneGraph场景层级
### SceneSerializer场景序列化
### Window窗口管理
### UI用户界面
## Editor编辑器
## Runtime运行时
