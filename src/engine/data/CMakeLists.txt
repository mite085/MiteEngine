cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Data
set(ENGINE_DATA_LIB_NAME MiteEngine_Engine_Data)

# 创建源代码组(筛选器)
set(BASIC_TYPE_SRCS
    basic_type/asset_type.h
    basic_type/handle_type.h
    basic_type/camera_type.h
    basic_type/bounding_volumes_types.h       # 基础几何类型定义（无依赖）
    basic_type/bounding_volumes_types.cpp
)
source_group("basic_type" FILES ${BASIC_TYPE_SRCS})

set(BASIC_DATA_SRCS
    basic_data/camera.h
    basic_data/camera.cpp
    basic_data/framebuffer.h
    basic_data/framebuffer.cpp
    basic_data/mesh.h
    basic_data/mesh.cpp
    basic_data/model.h
    basic_data/model.cpp
    basic_data/shader.h
    basic_data/shader.cpp
    basic_data/shader_cache.h
    basic_data/shader_cache.cpp
    basic_data/texture.h
    basic_data/texture.cpp
    basic_data/transform.h
    basic_data/transform.cpp


    basic_data/bounding_volumes.h             # AABB/OBB/Sphere等包围盒实现
    basic_data/bounding_volumes.cpp
    basic_data/ray.h                          # 射线类型定义和基础方法
    basic_data/ray.cpp
    basic_data/frustum.h                      # 视锥体类型定义和基础方法  
    basic_data/frustum.cpp
)
source_group("basic_data" FILES ${BASIC_DATA_SRCS})

set(BASIC_EVENT_SRCS
    basic_event/asset_event.h
)
source_group("basic_event" FILES ${BASIC_EVENT_SRCS})

# 创建静态库
add_library(${ENGINE_DATA_LIB_NAME}  
    STATIC  
    ${BASIC_TYPE_SRCS}
    ${BASIC_DATA_SRCS}
    ${BASIC_EVENT_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_DATA_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_DATA_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
)