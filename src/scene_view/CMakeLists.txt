cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_View
set(SCENE_VIEW_LIB_NAME MiteEngine_Scene_View)

# 创建源代码组(筛选器)
set(SCENE_VIEW_INTERFACE_SRCS
    scene_view.h
    scene_view.cpp
    scene_data.h
    scene_data.cpp
)
source_group("scene core" FILES ${SCENE_VIEW_INTERFACE_SRCS})

# 创建静态库
add_library(${SCENE_VIEW_LIB_NAME}  STATIC  ${SCENE_VIEW_INTERFACE_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${SCENE_VIEW_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PRIVATE "../../thirdparty/"
)

# 链接核心库
target_link_libraries(${SCENE_VIEW_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
    MiteEngine_Scene_Core
)