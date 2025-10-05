cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Light
set(ENGINE_LIGHT_LIB_NAME MiteEngine_Engine_Light)

set(LIGHT_CORE_SRCS
    light_core/light.h                  # 光源基类，所有光源实现均应继承
    light_core/light.cpp
    light_core/light_ssbo.h             # 光源数据SSBO管理器
    light_core/light_ssbo.cpp
    light_core/light_manager.h          # 光源管理器，负责创建和统一处理
    light_core/light_manager.cpp

    light_core/shadow_map_type.h        # 阴影贴图类型定义
    light_core/shadow_map.h             # 阴影贴图基类，所有阴影贴图实现均应继承
    light_core/shadow_map.cpp
)
source_group("light core" FILES ${LIGHT_CORE_SRCS})

set(LIGHT_DATA_SRCS
    light_data/area_light.h            # 面光源
    light_data/area_light.cpp
    light_data/directional_light.h     # 方向光
    light_data/directional_light.cpp
    light_data/point_light.h           # 点光源
    light_data/point_light.cpp
    light_data/spot_light.h            # 聚光灯
    light_data/spot_light.cpp
)
source_group("light data" FILES ${LIGHT_DATA_SRCS})

set(LIGHT_SHADOW_MAP_SRCS
    light_shadow_map/area_shadow_map.h           # 面光源阴影贴图
    light_shadow_map/area_shadow_map.cpp
    light_shadow_map/directional_shadow_map.h    # 方向光阴影贴图
    light_shadow_map/directional_shadow_map.cpp
    light_shadow_map/point_shadow_map.h          # 点光源阴影贴图
    light_shadow_map/point_shadow_map.cpp
    light_shadow_map/spot_shadow_map.h           # 聚光灯阴影贴图
    light_shadow_map/spot_shadow_map.cpp
)
source_group("light shadow" FILES ${LIGHT_SHADOW_MAP_SRCS})

# 创建静态库
add_library(${ENGINE_LIGHT_LIB_NAME}
    STATIC
    ${LIGHT_CORE_SRCS}
    ${LIGHT_DATA_SRCS}
    ${LIGHT_SHADOW_MAP_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_LIGHT_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_LIGHT_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
)