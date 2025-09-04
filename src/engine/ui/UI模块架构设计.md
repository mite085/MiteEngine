# MiteEngine 事件驱动的UI模块设计文档

## 文件层次结构
src/engine/ui/
├── core/                    # 核心抽象层
│   ├── UISystem.h          # UI系统单例，管理全局UI状态
│   ├── UIBackend.h         # 后端抽象接口
│   ├── events/             # UI事件定义层
│   │   ├── UIEvent.h          # UI事件类型定义
│   │   ├── UIInteractionEvents.h   # 用户交互事件
│   │   ├── UILifecycleEvents.h     # UI生命周期事件
│   │   ├── EditorUIEvents.h        # 编辑器专用事件
│   │   └── RuntimeUIEvents.h       # 运行时专用事件（预留）
│   ├── UIStyle.h           # 样式管理抽象
│   └── UILocalization.h    # 本地化管理抽象
│
├── widgets/                # 抽象控件层（事件驱动重构）
│   ├── UIWidget.h          # 控件基类（包含事件发布接口）
│   ├── UIPanel.h           # 面板基类（事件订阅管理）
│   ├── basic/              # 基础控件
│   │   ├── UIButton.h      # 发布ButtonClickEvent
│   │   ├── UISlider.h      # 发布SliderChangeEvent
│   │   ├── UICheckbox.h    # 发布CheckboxToggleEvent
│   │   ├── UITextInput.h   # 发布TextInputEvent
│   │   └── UIComboBox.h    # 发布ComboBoxSelectEvent
│   └── layout/             # 布局管理器
│       ├── UILayout.h
│       ├── UIHorizontalLayout.h
│       └── UIVerticalLayout.h
│
├── backends/               # 后端实现层
│   └── imgui/
│       ├── ImGuiBackend.h          # ImGui后端实现
│       ├── ImGuiStyleAdapter.h     # ImGui样式适配器
│       ├── ImGuiLocalization.h     # ImGui本地化实现
│       └── ImGuiInputAdapter.h     # ImGui输入适配器（事件转换）
│
├── editor/                 # 编辑器专用UI组件
│   ├── EditorUISystem.h   # 编辑器UI系统扩展（事件订阅中心）
│   ├── events/            # 编辑器事件处理器
│   │   ├── ViewportEventHandlers.h    # 视口事件处理
│   │   ├── HierarchyEventHandlers.h   # 层级事件处理
│   │   ├── InspectorEventHandlers.h   # 检查器事件处理
│   │   └── AssetBrowserEventHandlers.h # 资源浏览器事件处理
│   ├── panels/            # 编辑器面板（事件订阅者）
│   │   ├── ViewportPanel.h        # 订阅/发布视图相关事件
│   │   ├── HierarchyPanel.h       # 订阅场景变化事件
│   │   ├── InspectorPanel.h       # 订阅实体选择事件
│   │   └── AssetBrowserPanel.h    # 订阅资源管理事件
│   ├── widgets/           # 编辑器专用控件
│   │   ├── ComponentWidgets.h     # 发布组件修改事件
│   │   └── MaterialEditorWidget.h # 发布材质编辑事件
│   └── tools/             # 编辑器工具层
│       ├── GizmoSystem.h          # 订阅/发布Gizmo事件
│       ├── events/                # Gizmo事件处理器
│       │   ├── GizmoEventHandlers.h      # Gizmo交互事件处理
│       │   └── TransformEventHandlers.h  # 变换事件处理
│       ├── ViewportGizmo.h        # 发布Gizmo交互事件
│       └── ViewportManipulate.h   # 发布视图操作事件
│
├── runtime/               # 运行时UI组件（预留）
│   ├── RuntimeUISystem.h  # 运行时UI系统（事件订阅中心）
│   └── events/            # 运行时事件处理器
│       └── RuntimeEventHandlers.h # 运行时UI事件处理
│
└── event_handlers/        # 通用事件处理器
    ├── UIInputHandler.h       # 输入事件到UI事件转换
    ├── UIRenderHandler.h      # 渲染事件处理
    └── UIStateHandler.h       # UI状态管理事件处理

## 开发顺序原则

核心抽象层优先于具体实现
基础控件优先于复杂组件
事件系统优先于具体功能
通用功能优先于专用功能

第一阶段：核心抽象层（1-7）
core/UIEvent.h - UI相关的事件定义
core/UIBackend.h - 后端抽象接口，定义UI系统与渲染后端的契约
core/UISystem.h - UI系统单例，管理全局UI状态和事件总线
core/UIStyle.h - 样式管理抽象，为控件提供统一的样式接口
core/UILocalization.h - 本地化管理抽象，支持多语言
events/UIInteractionEvents.h - 用户交互事件定义
events/UILifecycleEvents.h - UI生命周期事件定义

第二阶段：基础控件和事件处理（8-16）
widgets/UIWidget.h - 控件基类，包含事件发布接口
widgets/UIPanel.h - 面板基类，管理事件订阅
widgets/layout/UILayout.h - 布局管理器基类
widgets/layout/UIHorizontalLayout.h - 水平布局实现
widgets/layout/UIVerticalLayout.h - 垂直布局实现
event_handlers/UIInputHandler.h - 输入事件到UI事件转换
event_handlers/UIRenderHandler.h - 渲染事件处理
event_handlers/UIStateHandler.h - UI状态管理事件处理
backends/imgui/ImGuiInputAdapter.h - ImGui输入适配器

第三阶段：基础控件实现（17-22）
widgets/basic/UIButton.h - 按钮控件实现
widgets/basic/UISlider.h - 滑块控件实现
widgets/basic/UICheckbox.h - 复选框控件实现
widgets/basic/UITextInput.h - 文本输入控件实现
widgets/basic/UIComboBox.h - 下拉框控件实现
backends/imgui/ImGuiBackend.h - ImGui后端核心实现

第四阶段：样式和本地化适配（23-24）
backends/imgui/ImGuiStyleAdapter.h - ImGui样式适配器
backends/imgui/ImGuiLocalization.h - ImGui本地化实现

第五阶段：编辑器核心事件（25-26）
events/EditorUIEvents.h - 编辑器专用事件定义
events/RuntimeUIEvents.h - 运行时专用事件定义（预留）

第六阶段：编辑器事件处理器（27-31）
editor/events/ViewportEventHandlers.h - 视口事件处理
editor/events/HierarchyEventHandlers.h - 层级事件处理
editor/events/InspectorEventHandlers.h - 检查器事件处理
editor/events/AssetBrowserEventHandlers.h - 资源浏览器事件处理
runtime/events/RuntimeEventHandlers.h - 运行时事件处理（预留）

第七阶段：编辑器面板实现（32-35）
editor/panels/ViewportPanel.h - 视口面板实现
editor/panels/HierarchyPanel.h - 层级面板实现
editor/panels/InspectorPanel.h - 检查器面板实现
editor/panels/AssetBrowserPanel.h - 资源浏览器面板实现

第八阶段：编辑器专用控件（36-37）
editor/widgets/ComponentWidgets.h - 组件控件实现
editor/widgets/MaterialEditorWidget.h - 材质编辑控件实现

第九阶段：编辑器工具系统（38-42）
editor/tools/GizmoSystem.h - Gizmo系统核心
editor/tools/events/GizmoEventHandlers.h - Gizmo事件处理
editor/tools/events/TransformEventHandlers.h - 变换事件处理
editor/tools/ViewportGizmo.h - 视口Gizmo实现
editor/tools/ViewportManipulate.h - 视图操作实现

第十阶段：系统集成（43-44）
editor/EditorUISystem.h - 编辑器UI系统扩展
runtime/RuntimeUISystem.h - 运行时UI系统（预留）