cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为Mite_Application
set(EDITOR_LIB_NAME MiteEngine_Editor)

# 创建源代码组(筛选器)
set(EDITOR_SRCS
    editor.h
    editor.cpp
    object_factor.h
)
source_group("editor" FILES ${EDITOR_SRCS})

# 创建静态库
add_library(${EDITOR_LIB_NAME}  
    STATIC  
    ${EDITOR_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${EDITOR_LIB_NAME}
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${EDITOR_LIB_NAME}
    PRIVATE 
    
    # 第三方库依赖
    OpenGL::GL
    glfw
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