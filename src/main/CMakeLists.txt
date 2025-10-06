cmake_minimum_required(VERSION 3.10)

# 创建源代码组(筛选器)
set(MAIN_SRCS
    main.cpp
)
source_group("main" FILES ${MAIN_SRCS})

# 创建可执行目标，添加源代码组
add_executable(${PROJECT_NAME} ${MAIN_SRCS})

# 设置头文件路径
target_include_directories(${PROJECT_NAME}
    PRIVATE 
)

# 链接核心库
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    # 第三方库依赖
    OpenGL::GL
    glfw
    shaderc
    spdlog::spdlog
    Imgui
    ImGuizmo

    # 应用程序依赖
    MiteEngine_Application

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
    MiteEngine_Engine_Scene_Graph
    MiteEngine_Engine_Scene_View
    MiteEngine_Engine_Window
    MiteEngine_Engine_Ui
)

# 添加自定义目标来复制assets文件夹到可执行文件目录（用于开发时方便）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to executable directory"
)