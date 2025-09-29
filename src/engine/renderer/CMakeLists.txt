cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Renderer
set(ENGINE_RENDERER_LIB_NAME MiteEngine_Engine_Renderer)

# 创建源代码组(筛选器)
set(RENDERER_CORE_SRCS
    render_core/render_command.h
    render_core/render_command.cpp
    render_core/render_context.h
    render_core/render_context.cpp
    render_core/render_device.h
    render_core/render_device.cpp
    render_core/render_pipeline.h
    render_core/render_pipeline.cpp
)
source_group("render core" FILES ${RENDERER_CORE_SRCS})

set(RENDERER_OPENGL_SRCS
    render_opengl/opengl_command.h
    render_opengl/opengl_command.cpp
    render_opengl/opengl_device.h
    render_opengl/opengl_device.cpp
    render_opengl/opengl_pipeline.h
    render_opengl/opengl_pipeline.cpp
)
source_group("render opengl" FILES ${RENDERER_OPENGL_SRCS})

set(RENDERER_STAGES_SRCS
    render_stages/render_stage.h
    render_stages/render_stage.cpp

    render_stages/deferred_lighting_stage.h
    render_stages/deferred_lighting_stage.cpp
    render_stages/forward_stage.h
    render_stages/forward_stage.cpp
    render_stages/gbuffer_stage.h
    render_stages/gbuffer_stage.cpp
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
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_View
    MiteEngine_Engine_Scene_Graph
)