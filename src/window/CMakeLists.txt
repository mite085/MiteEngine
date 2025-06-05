cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Window
set(WINDOW_LIB_NAME MiteEngine_Window)

# 创建源代码组(筛选器)
set(WINDOW_LIB_SRCS
    window.h
    window.cpp
)
source_group("interface" FILES ${WINDOW_LIB_SRCS})

set(OPENGL_WINDOW_SRCS
    glfw_window/glfw_window.h
    glfw_window/glfw_window.cpp
    ../../thirdparty/glad/glad.c
)
source_group("opengl window" FILES ${OPENGL_WINDOW_SRCS})

# 创建静态库
add_library(${WINDOW_LIB_NAME}  STATIC  ${WINDOW_LIB_SRCS}  ${OPENGL_WINDOW_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${WINDOW_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PRIVATE "../../thirdparty/"
)

# 链接核心库
target_link_libraries(${WINDOW_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Core
)