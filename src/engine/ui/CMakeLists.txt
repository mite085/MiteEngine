cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Ui
set(ENGINE_UI_LIB_NAME MiteEngine_Engine_Ui)

# 创建源代码组(筛选器)
set(UI_CORE_SRCS
    ui_core/ui_backend.h        # 后端抽象接口
    ui_core/ui_style.h          # 样式管理抽象
    ui_core/ui_localization.h   # 本地化管理抽象

    ui_core/ui_system.h         # UI系统单例，管理全局UI状态
    ui_core/ui_system.cpp
)
source_group("ui core" FILES ${UI_CORE_SRCS})

set(UI_WIDGET_SRCS
    ui_widgets/ui_widget.h      # 控件基类（包含事件发布接口）
    ui_widgets/ui_panel.h       # 面板基类（事件订阅管理）

    # 以下为基础控件
    ui_widgets/ui_button.h      # 发布ButtonClickEvent
    ui_widgets/ui_slider.h      # 发布SliderChangeEvent
    ui_widgets/ui_checkbox.h    # 发布CheckboxToggleEvent
    ui_widgets/ui_text_input.h   # 发布TextInputEvent
    ui_widgets/ui_combobox.h    # 发布ComboBoxSelectEvent

    # 以下为布局管理器
    ui_widgets/ui_layout.h              # 布局基类
    ui_widgets/ui_layout_horizontal.h   # 水平布局
    ui_widgets/ui_layout_vertical.h     # 垂直布局
)
source_group("ui widgets" FILES ${UI_WIDGET_SRCS})

set(UI_IMGUI_BACKEND_SRCS
    ui_imgui_backend/ui_imgui_backend.h         # ImGui后端实现
    ui_imgui_backend/ui_imgui_style_adapter.h   # ImGui样式适配器
    ui_imgui_backend/ui_imgui_localization.h    # ImGui本地化实现
    ui_imgui_backend/ui_imgui_input_adapter.h   # ImGui输入适配器（事件转换）
)
source_group("ui imgui backend" FILES ${UI_IMGUI_BACKEND_SRCS})

set(UI_EVENT_SRCS
    ui_event/ui_event.h                 # 事件基本类型
    ui_event/ui_events_interaction.h    # 交互事件，包括按钮点击，滑块值改变等
    ui_event/ui_events_lifecycle.h      # 生命周期事件，包括窗口创建，关闭等
    ui_event/ui_events_editor.h         # 编辑器专属事件，包括Component属性编辑等
    ui_event/ui_events_runtime.h        # 运行时专属事件，包括游戏逻辑等

    # 以下为事件处理器
    ui_event/ui_event_handler_input.h     # 输入事件到UI事件转换
    ui_event/ui_event_handler_render.h    # 渲染事件处理
    ui_event/ui_event_handler_state.h     # UI状态管理事件处理
)
source_group("ui events" FILES ${UI_EVENT_SRCS})

set(UI_FOR_EDITOR_SRCS
    ui_for_editor/ui_editor_system.h    # 编辑器UI系统扩展（事件订阅中心）
)
source_group("ui for editor" FILES ${UI_FOR_EDITOR_SRCS})

set(UI_FOR_RUNTIME_SRCS
    ui_for_runtime/ui_runtime_system.h  # 运行时UI系统扩展（事件订阅中心）
)
source_group("ui for runtime" FILES ${UI_FOR_RUNTIME_SRCS})

# 创建静态库，添加源代码组
add_library(${ENGINE_UI_LIB_NAME}
    STATIC
    ${UI_CORE_SRCS}
    ${UI_WIDGET_SRCS}
    ${UI_IMGUI_BACKEND_SRCS}
    ${UI_EVENT_SRCS}
    ${UI_FOR_EDITOR_SRCS}
    ${UI_FOR_RUNTIME_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_UI_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开当前目录
)

# 链接核心库
target_link_libraries(${ENGINE_UI_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    Imgui
    ImGuizmo
    
    # 强依赖
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Renderer
    MiteEngine_Engine_Input

    # 弱依赖：创建资产浏览器界面
    MiteEngine_Asset

    # 弱依赖：创建材质修改界面
    MiteEngine_Engine_Material

    # 弱依赖：创建Component组件属性界面
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_Graph
    MiteEngine_Engine_Scene_View
)