cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_Core
set(SCENE_CORE_LIB_NAME MiteEngine_Scene_Core)

# 创建源代码组(筛选器)
set(SCENE_CORE_INTERFACE_SRCS
    entity.h
    entity.cpp
    scene_core.h
    scene_core.cpp
    scene_system.h
    scene_system.cpp
)
source_group("scene core" FILES ${SCENE_CORE_INTERFACE_SRCS})

# 创建静态库
add_library(${SCENE_CORE_LIB_NAME}  STATIC  ${SCENE_CORE_INTERFACE_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${SCENE_CORE_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${SCENE_CORE_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
)