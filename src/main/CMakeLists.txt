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
    # 应用严格警告设置
    mite_engine_compile_options
    # 第三方头文件接口库
    mite_engine_thirdparty_headers
    # 第三方库依赖
    OpenGL::GL
    glfw
    shaderc
    glslang
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
    
    MiteEngine_Engine_Light
    MiteEngine_Engine_Material
    MiteEngine_Engine_Renderer
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_Graph
    MiteEngine_Engine_Scene_View
    MiteEngine_Engine_Window
    MiteEngine_Engine_Ui
)


# 添加自定义目标来复制assets文件夹到可执行文件目录（用于开发时方便）

# 创建过滤复制脚本
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/copy_filtered_assets.cmake "
file(GLOB_RECURSE ASSET_FILES 
    LIST_DIRECTORIES false
    RELATIVE \${SOURCE_DIR}
    \${SOURCE_DIR}/*)
# 过滤掉不需要的文件后缀(主要是blend模型工程文件)
list(FILTER ASSET_FILES EXCLUDE REGEX \"\\\\.(tmp|bak|log|blend|blend1)$\")
# 确保目标目录存在
file(MAKE_DIRECTORY \${DEST_DIR})
# 复制文件
foreach(ASSET_FILE \${ASSET_FILES})
    get_filename_component(DEST_SUBDIR \${DEST_DIR}/\${ASSET_FILE} DIRECTORY)
    file(MAKE_DIRECTORY \${DEST_SUBDIR})
    file(COPY \${SOURCE_DIR}/\${ASSET_FILE}
          DESTINATION \${DEST_SUBDIR})
endforeach()
")
# 复制资产文件到可执行文件目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
        -D "SOURCE_DIR=${CMAKE_SOURCE_DIR}/assets"
        -D "DEST_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        -P "${CMAKE_CURRENT_BINARY_DIR}/copy_filtered_assets.cmake"
    COMMENT "Copying filtered assets to executable directory"
)
# 复制config/imgui.ini文件到可执行文件目录
if(EXISTS "${CMAKE_SOURCE_DIR}/config/imgui.ini")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/config/imgui.ini"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imgui.ini"
        COMMENT "Copying imgui.ini to executable directory"
    )
else()
    message(WARNING "imgui.ini not found at ${CMAKE_SOURCE_DIR}/config/imgui.ini")
endif()
# 在Linux平台下复制mite_ubuntu.sh脚本
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # 检查脚本文件是否存在
    if(EXISTS "${CMAKE_SOURCE_DIR}/mite_ubuntu.sh")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/mite_ubuntu.sh"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/mite_ubuntu.sh"
            COMMENT "Copying mite_ubuntu.sh to executable directory"
        )
    else()
        message(WARNING "mite_ubuntu.sh not found at ${CMAKE_SOURCE_DIR}/mite_ubuntu.sh")
    endif()
endif()