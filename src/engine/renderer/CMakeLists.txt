cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Renderer
set(ENGINE_RENDERER_LIB_NAME MiteEngine_Engine_Renderer)

# 创建源代码组(筛选器)
set(RENDERER_CORE_SRCS
    render_core/render_command.h    # 渲染命令系统，负责渲染操作整合命令
    render_core/render_command.cpp
    render_core/render_context.h    # 渲染上下文，负责统一数据传递和资源共享
    render_core/render_context.cpp
    render_core/render_device.h     # 渲染设备，负责具体渲染操作封装
    render_core/render_device.cpp
    render_core/render_pipeline.h   # 渲染管线，负责stage管理
    render_core/render_pipeline.cpp
)
source_group("render core" FILES ${RENDERER_CORE_SRCS})

set(RENDERER_OPENGL_SRCS
    render_opengl/opengl_command.h   # opengl无需独立实现上下文，其他均需要独立实现
    render_opengl/opengl_command.cpp
    render_opengl/opengl_device.h
    render_opengl/opengl_device.cpp
    render_opengl/opengl_pipeline.h
    render_opengl/opengl_pipeline.cpp
)
source_group("render opengl" FILES ${RENDERER_OPENGL_SRCS})

set(RENDERER_STAGES_SRCS
    render_stages/render_stage.h        # 渲染阶段基类，目前规划顺序为shadow map->gbuffer->deffered lighting->forward
    render_stages/render_stage.cpp

    render_stages/blend_stage.h               # 结果混合阶段
    render_stages/blend_stage.cpp
    render_stages/deferred_lighting_stage.h     # 延迟光照计算阶段
    render_stages/deferred_lighting_stage.cpp
    render_stages/forward_stage.h               # 前向渲染阶段
    render_stages/forward_stage.cpp
    render_stages/gbuffer_stage.h               # 几何处理阶段
    render_stages/gbuffer_stage.cpp
    render_stages/shadow_map_stage.h            # 阴影贴图阶段
    render_stages/shadow_map_stage.cpp
)
source_group("render stage" FILES ${RENDERER_STAGES_SRCS})

# 创建静态库
add_library(${ENGINE_RENDERER_LIB_NAME}  
    STATIC
    ${RENDERER_CORE_SRCS}
    ${RENDERER_OPENGL_SRCS}
    ${RENDERER_STAGES_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_RENDERER_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${ENGINE_RENDERER_LIB_NAME}
    PRIVATE 
    glfw
    shaderc
    glslang
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Light
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_View
    MiteEngine_Engine_Scene_Graph
)