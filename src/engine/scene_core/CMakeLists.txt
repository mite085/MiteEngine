cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_Core
set(ENGINE_SCENE_CORE_LIB_NAME MiteEngine_Engine_Scene_Core)

# 创建源代码组(筛选器)
set(SCENE_CORE_SRCS
    scene_core/component.h
    scene_core/component.cpp
    scene_core/component_id.h
    scene_core/component_id.cpp
    scene_core/component_event_publisher.h
    scene_core/component_event_publisher.cpp
    scene_core/component_snapshot.h
    scene_core/component_snapshot.cpp
    scene_core/component_system.h
    scene_core/component_system.cpp
    scene_core/component_system_manager.h
    scene_core/component_system_manager.cpp
    scene_core/entity.h
    scene_core/entity.cpp
    scene_core/scene_core.h
    scene_core/scene_core.cpp
    scene_core/scene_core_event.h
    scene_core/scene_core_event.cpp
    scene_core/scene_registry.h
    scene_core/scene_registry.cpp
)
source_group("core" FILES ${SCENE_CORE_SRCS})

set(SCENE_COMPONENTS_SRCS
    scene_core_components/component_headers.h
    scene_core_components/bounding_volume_component.h
    scene_core_components/bounding_volume_component.cpp
    scene_core_components/camera_component.h
    scene_core_components/camera_component.cpp
    scene_core_components/id_component.h
    scene_core_components/id_component.cpp
    scene_core_components/mesh_component.h
    scene_core_components/mesh_component.cpp
    scene_core_components/material_component.h
    scene_core_components/material_component.cpp
    scene_core_components/destroy_component.h
    scene_core_components/destroy_component.cpp
    scene_core_components/tag_component.h
    scene_core_components/tag_component.cpp
    scene_core_components/transform_component.h
    scene_core_components/transform_component.cpp
    scene_core_components/visibility_component.h
    scene_core_components/visibility_component.cpp
)
source_group("components" FILES ${SCENE_COMPONENTS_SRCS})

# 创建静态库
add_library(${ENGINE_SCENE_CORE_LIB_NAME}
    STATIC
    ${SCENE_CORE_SRCS}
    ${SCENE_COMPONENTS_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_SCENE_CORE_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${ENGINE_SCENE_CORE_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Material
)