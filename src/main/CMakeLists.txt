cmake_minimum_required(VERSION 3.10)

# 创建源代码组(筛选器)
set(MAIN_SRCS
    main.cpp
    application.h
    application.cpp
)
source_group("main" FILES ${MAIN_SRCS})

# 创建可执行目标，添加源代码组
add_executable(${PROJECT_NAME} ${MAIN_SRCS})

# 查找 OpenGL 库（跨平台）
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found! Please install OpenGL development libraries.")
endif()

# 设置头文件路径
target_include_directories(${PROJECT_NAME}
    PRIVATE 
    "../../thirdparty/imgui/"
)

# 链接核心库
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    
    # 第三方库依赖
    OpenGL::GL
    glfw
    spdlog::spdlog

    # 各模块库依赖
    MiteEngine_Assert
    MiteEngine_Core
    MiteEngine_Input
    
    MiteEngine_Material
    MiteEngine_Renderer
    MiteEngine_Scene_Core
    MiteEngine_Scene_View
    MiteEngine_Window
    MiteEngine_Ui
)