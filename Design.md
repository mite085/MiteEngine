# MiteEngine模块设计文档


## 目录
1. [MiteEngine模块设计文档](#miteengine模块设计文档)
   1. [目录](#目录)
   2. [1. 系统架构总览](#1-系统架构总览)
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

## 1. 系统架构总览
```mermaid
graph TD
  A[Main] --> B[Application]
  B --> C[Editor]
  B --> D[Runtime]
  C --> E[Engine]
  D --> E
  E --> F[Renderer]
  E --> G[Scene_Core]
  E --> H[UI]
  F --> I[OpenGL]
  F -.-> J[Vulkan Future]
  G --> K[Scene_Graph]
  G --> L[Scene_Serialization]
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
