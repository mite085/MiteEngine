cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Renderer
set(RENDERER_LIB_NAME MiteEngine_Renderer)

# 创建源代码组(筛选器)
set(RENDERER_INTERFACE_SRCS
    renderer.h
    renderer.cpp
    render_command.h
    render_command.cpp
    render_device.h
    render_device.cpp
    texture.h
    texture.cpp
    shader.h
    shader.cpp
    shader_cache.h
    shader_cache.cpp
    material_instance.h
    material_instance.cpp
    mesh.h
    mesh.cpp
    model.h
    model.cpp
)
source_group("interface" FILES ${RENDERER_INTERFACE_SRCS})

set(OPENGL_RENDERER_SRCS
    opengl_renderer/opegl_device.h
    opengl_renderer/opegl_device.cpp
    opengl_renderer/opengl_renderer.h
    opengl_renderer/opengl_renderer.cpp
)
source_group("opengl renderer" FILES ${OPENGL_RENDERER_SRCS})

# 创建静态库
add_library(${RENDERER_LIB_NAME}  STATIC  ${RENDERER_INTERFACE_SRCS}  ${OPENGL_RENDERER_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${RENDERER_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${RENDERER_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Core
    MiteEngine_Window
    MiteEngine_Input
    MiteEngine_Asset
)