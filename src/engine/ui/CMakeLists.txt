cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Ui
set(ENGINE_UI_LIB_NAME MiteEngine_Engine_Ui)

# 创建源代码组(筛选器)
set(UI_CORE_SRCS
    ui_core/ui_system.h         # UI系统单例，管理全局UI状态
    ui_core/ui_system.cpp

    # 后端与渲染相关
    ui_core/ui_backend.h        # 后端抽象接口（包含后端的初始化、Begin/End Frame、样式语言统一管理等）
    ui_core/ui_render.h         # 渲染接口（包含具体渲染操作的抽象，如绘制button、float edit等）
    ui_core/ui_render.cpp
    ui_core/ui_render_props.h   # 渲染属性（执行渲染操作所需要的属性，如button上的text，float edit的值等）

    # 样式相关
    ui_core/ui_style.h          # 样式抽象
    ui_core/ui_style.cpp
    ui_core/ui_style_manager.h  # 样式管理
    ui_core/ui_style_manager.cpp

    # 翻译相关
    ui_core/ui_localization.h   # 本地化（翻译）
    ui_core/ui_localization.cpp
    ui_core/ui_localization_json.h # 基于Json配置文件的本地化实现
    ui_core/ui_localization_json.cpp
    ${CMAKE_BINARY_DIR}/src/engine/ui/ui_core/localization_json_config.h # Json配置文件索引

)
source_group("ui core" FILES ${UI_CORE_SRCS})

set(UI_ELEMENT_SRCS
    ui_element/ui_element.h     # 元素基类（所有UI可交互元素的抽象基类）
    ui_element/ui_element.cpp

    # 以下为基本控件
    ui_element/ui_button.h      # 发布ButtonClickEvent
    ui_element/ui_slider.h      # 发布SliderChangeEvent
    ui_element/ui_checkbox.h    # 发布CheckboxToggleEvent
    ui_element/ui_text_input.h   # 发布TextInputEvent
    ui_element/ui_combobox.h    # 发布ComboBoxSelectEvent

    # 以下为部件与面板
    ui_element/ui_menu.h        # 菜单栏（原则上仅Editor顶部应当存在）
    ui_element/ui_menu.cpp
    ui_element/ui_widget.h      # 控件基类（包含事件发布接口）
    ui_element/ui_widget.cpp
    ui_element/ui_panel.h       # 面板基类（事件订阅管理）
    ui_element/ui_panel.cpp
)
source_group("ui elements" FILES ${UI_ELEMENT_SRCS})

set(UI_LAYOUT_SRCS
    ui_layout/ui_layout.h              # 布局基类
    ui_layout/ui_layout.cpp
    ui_layout/ui_layout_horizontal.h   # 水平布局
    ui_layout/ui_layout_horizontal.cpp
    ui_layout/ui_layout_vertical.h     # 垂直布局
    ui_layout/ui_layout_vertical.cpp
)
source_group("ui layouts" FILES ${UI_LAYOUT_SRCS})

set(UI_IMGUI_BACKEND_SRCS
    ui_imgui_backend/ui_imgui_backend.h         # ImGui后端实现
    ui_imgui_backend/ui_imgui_backend.cpp
    ui_imgui_backend/ui_imgui_render.h          # ImGui渲染实现
    ui_imgui_backend/ui_imgui_render.cpp

    ui_imgui_backend/ui_imgui_font_manager.h    # ImGui字体管理器
    ui_imgui_backend/ui_imgui_font_manager.cpp
    ui_imgui_backend/ui_imgui_style_adapter.h   # ImGui样式适配器
    ui_imgui_backend/ui_imgui_style_adapter.cpp
    ui_imgui_backend/ui_imgui_input_adapter.h   # ImGui输入适配器（事件转换）
    ui_imgui_backend/ui_imgui_input_adapter.cpp
)
source_group("ui imgui backend" FILES ${UI_IMGUI_BACKEND_SRCS})

set(UI_EVENT_SRCS
    ui_event/ui_event.h                 # 事件基本类型
    ui_event/ui_events_layout.h         # 布局修改事件，包含控件位置、大小、尺寸修改等
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

set(UI_VIEWPORT_SRCS
    ui_viewport/ui_viewport_panel.h     # 视口面板，核心接口
    ui_viewport/ui_viewport_panel.cpp
    ui_viewport/ui_viewport_overlay.h   # 视口覆盖层，绘制统计信息、参考线等
    ui_viewport/ui_viewport_overlay.cpp
    ui_viewport/ui_viewport_toolbar.h   # 视口工具层，提供视口相关的操作按钮和控件
    ui_viewport/ui_viewport_toolbar.cpp
)
source_group("ui viewport" FILES ${UI_VIEWPORT_SRCS})

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
    ${UI_ELEMENT_SRCS}
    ${UI_LAYOUT_SRCS}
    ${UI_IMGUI_BACKEND_SRCS}
    ${UI_EVENT_SRCS}
    ${UI_VIEWPORT_SRCS}
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
    MiteEngine_Engine_Event
    MiteEngine_Engine_Renderer
    MiteEngine_Engine_Window
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