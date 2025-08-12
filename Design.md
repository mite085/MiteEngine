# MiteEngine模块设计文档


## 目录
- [MiteEngine模块设计文档](#miteengine模块设计文档)
  - [目录](#目录)
  - [系统架构总览](#系统架构总览)
  - [系统概述](#系统概述)
    - [Main程序入口](#main程序入口)
    - [Application主循环](#application主循环)
    - [Asset资产管理](#asset资产管理)
    - [Engine核心引擎](#engine核心引擎)
    - [Editor编辑模块](#editor编辑模块)
    - [Runtime运行时模块](#runtime运行时模块)
  - [Application主循环](#application主循环-1)
    - [MainLoop主循环](#mainloop主循环)
  - [Asset资产管理](#asset资产管理-1)
    - [架构总览](#架构总览)
    - [AssetManager资产管理器](#assetmanager资产管理器)
    - [AssetCache资产缓存](#assetcache资产缓存)
    - [ModelLoader模型加载](#modelloader模型加载)
    - [TextureLoader贴图加载](#textureloader贴图加载)
  - [Engine核心引擎](#engine核心引擎-1)
    - [Core基础模块](#core基础模块)
    - [Data渲染数据模块](#data渲染数据模块)
    - [Input输入系统](#input输入系统)
    - [Material材质系统](#material材质系统)
    - [Renderer渲染系统](#renderer渲染系统)
    - [SceneCore场景核心](#scenecore场景核心)
    - [SceneView场景视图](#sceneview场景视图)
    - [SceneGraph场景层级](#scenegraph场景层级)
    - [SceneSerializer场景序列化](#sceneserializer场景序列化)
    - [Window窗口管理](#window窗口管理)
    - [UI用户界面](#ui用户界面)
  - [Editor编辑器](#editor编辑器)
  - [Runtime运行时](#runtime运行时)
---

## 系统架构总览
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
### Main程序入口
### Application主循环
### Asset资产管理
### Engine核心引擎
### Editor编辑模块
### Runtime运行时模块

## Application主循环
### MainLoop主循环
## Asset资产管理
### 架构总览
### AssetManager资产管理器
### AssetCache资产缓存
### ModelLoader模型加载
### TextureLoader贴图加载
## Engine核心引擎
### Core基础模块
### Data渲染数据模块
### Input输入系统
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
