cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_View
set(ENGINE_SCENE_VIEW_LIB_NAME MiteEngine_Engine_Scene_View)

# 创建源代码组(筛选器)
set(SCENE_VIEW_INTERFACE_SRCS
    renderable_item.h           # Renderer模块接收的基础数据结构
    renderable_item_builder.h   # RenderableItem构建器，将SceneNode*转换为渲染所需格式
    renderable_item_builder.cpp
    render_queue.h              # Renderer模块接收的核心数据结构
    render_queue.cpp
    scene_view.h                # 主控制器
    scene_view.cpp
    lod_processor.h             # LOD系统(基于距离和屏幕占比计算LOD级别，管理不同LOD层次的网格切换)
    lod_processor.cpp
)
source_group("scene view" FILES ${SCENE_VIEW_INTERFACE_SRCS})

set(SCENE_VIEW_TODO_INTERFACE_SRCS
    scene_view_todo/batch_optimizer.h           # 批处理(未实现)
    scene_view_todo/batch_optimizer.cpp
    scene_view_todo/multi_view_manager.h        # 多视口管理器(未实现)
    scene_view_todo/multi_view_manager.cpp
    scene_view_todo/debug_visualizer.h          # 调试绘制(未实现)
    scene_view_todo/debug_visualizer.cpp
)
source_group(scene view TODO)

# 创建静态库
add_library(${ENGINE_SCENE_VIEW_LIB_NAME}  STATIC  ${SCENE_VIEW_INTERFACE_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_SCENE_VIEW_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${ENGINE_SCENE_VIEW_LIB_NAME}
    PRIVATE 
    glfw
    shaderc
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_Graph
)