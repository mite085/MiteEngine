cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Ui
set(UI_LIB_NAME MiteEngine_Ui)

# 创建源代码组(筛选器)
set(UI_CORE_SRCS
    ui_core/ui_module.h
    ui_core/ui_panel.h
    ui_core/ui_panel.cpp
    ui_core/ui_system.h
    ui_core/ui_system.cpp
)
source_group("ui_core" FILES ${UI_CORE_SRCS})

set(UI_PANELS_SRCS
    ui_panels/inspector_panel.h
    ui_panels/inspector_panel.cpp
    ui_panels/viewport_panel.h
    ui_panels/viewport_panel.cpp
)
source_group("ui_panels" FILES ${UI_PANELS_SRCS})

set(UI_WIDGETS_SRCS
    ui_widgets/component_widget.h
    ui_widgets/component_widget.cpp
)
source_group("ui_widgets" FILES ${UI_WIDGETS_SRCS})

set(UI_RESOURCES_SRCS
    ui_resources/icon.h
    ui_resources/icon.cpp
    ui_resources/imgui_style.h
    ui_resources/imgui_style.cpp
)
source_group("ui_resources" FILES ${UI_RESOURCES_SRCS})

# 创建静态库，添加源代码组
add_library(${UI_LIB_NAME}
    STATIC
    ${UI_CORE_SRCS}
    ${UI_PANELS_SRCS}
    ${UI_WIDGETS_SRCS}
    ${UI_RESOURCES_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${UI_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开当前目录
)

# 链接核心库
target_link_libraries(${UI_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Asset
    MiteEngine_Core
    MiteEngine_Input

    MiteEngine_Renderer
    MiteEngine_Scene_Core
    MiteEngine_Scene_View
)