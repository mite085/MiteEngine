cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Scene_Graph
set(ENGINE_SCENE_GRAPH_LIB_NAME MiteEngine_Engine_Scene_Graph)

# 创建源代码组(筛选器)
set(SCENE_GRAPH_INTERFACE_SRCS
    scene_node.h                   # 场景节点（依赖基础几何类型）
    scene_node.cpp
    scene_node_manager.h           # 场景节点管理器（负责节点的创建、销毁、更新等）
    scene_node_manager.cpp
    spatial_partition.h            # 空间划分抽象接口（依赖几何类型）
    spatial_partition.cpp
    simple_bvh.h                   # BVH实现（依赖SceneNode和几何类型）
    simple_bvh.cpp
    transform_scene_node_system.h  # SceneNode的变换系统
    transform_scene_node_system.cpp
    scene_graph.h                  # 主管理器，场景图核心逻辑（依赖所有上述模块）
    scene_graph.cpp
    scene_graph_system.h           # 场景图系统，负责ECS与SceneGraph之间的数据同步和协调
    scene_graph_system.cpp
)
source_group("scene graph" FILES ${SCENE_GRAPH_INTERFACE_SRCS})

# 创建静态库
add_library(${ENGINE_SCENE_GRAPH_LIB_NAME}  STATIC  ${SCENE_GRAPH_INTERFACE_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_SCENE_GRAPH_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

# 链接核心库
target_link_libraries(${ENGINE_SCENE_GRAPH_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Material
    MiteEngine_Engine_Scene_Core
)