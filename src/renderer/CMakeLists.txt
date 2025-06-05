cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Renderer
set(RENDERER_LIB_NAME MiteEngine_Renderer)

# 创建源代码组(筛选器)
set(RENDERER_INTERFACE_SRCS
    renderer.h
    renderer.cpp
    texture.h
    texture.cpp
    buffer.h
    buffer.cpp
)
source_group("interface" FILES ${RENDERER_INTERFACE_SRCS})

set(OPENGL_RENDERER_SRCS
    opengl_renderer/opengl_renderer.h
    opengl_renderer/opengl_renderer.cpp
)
source_group("opengl renderer" FILES ${OPENGL_RENDERER_SRCS})

# 创建静态库
add_library(${RENDERER_LIB_NAME}  STATIC  ${RENDERER_INTERFACE_SRCS}  ${OPENGL_RENDERER_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${RENDERER_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PRIVATE "../../thirdparty/"
)

# 链接核心库
target_link_libraries(${RENDERER_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Core
    MiteEngine_Window
    MiteEngine_Scene_View
)