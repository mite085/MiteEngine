cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为Mite_Application
set(RUNTIME_LIB_NAME MiteEngine_Runtime)

# 创建源代码组(筛选器)
set(RUNTIME_SRCS
    runtime.h
    runtime.cpp
)
source_group("application" FILES ${RUNTIME_SRCS})

# 创建静态库
add_library(${RUNTIME_LIB_NAME}  
    STATIC  
    ${RUNTIME_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${RUNTIME_LIB_NAME}
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${RUNTIME_LIB_NAME}
    PRIVATE 
    
    # 第三方库依赖
    OpenGL::GL
    glfw
    shaderc
    spdlog::spdlog

    # 资产模块依赖
    MiteEngine_Asset

    # 引擎各模块库依赖
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Input
    
    MiteEngine_Engine_Material
    MiteEngine_Engine_Renderer
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_View
    MiteEngine_Engine_Window
    MiteEngine_Engine_Ui
)