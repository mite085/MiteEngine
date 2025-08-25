cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_View
set(ENGINE_SCENE_VIEW_LIB_NAME MiteEngine_Engine_Scene_View)

# 创建源代码组(筛选器)
set(SCENE_VIEW_INTERFACE_SRCS
    scene_view.h
    scene_view.cpp
    renderable_item.h
    renderable_item.cpp
)
source_group("scene view" FILES ${SCENE_VIEW_INTERFACE_SRCS})

# 创建静态库
add_library(${ENGINE_SCENE_VIEW_LIB_NAME}  STATIC  ${SCENE_VIEW_INTERFACE_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_SCENE_VIEW_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${ENGINE_SCENE_VIEW_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Material
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_Graph
)