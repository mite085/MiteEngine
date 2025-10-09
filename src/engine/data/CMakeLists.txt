cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Data
set(ENGINE_DATA_LIB_NAME MiteEngine_Engine_Data)

# 创建源代码组(筛选器)
set(BASIC_TYPE_SRCS
    basic_type/asset_id.h                       # 类型安全的资产UUID
    basic_type/asset_type.h                     # 资产基本数据类型
    basic_type/handle_type.h                    # GPU相关的句柄类型
    basic_type/material_type.h                  # 材质相关类型定义
    basic_type/framebuffer_type.h               # 帧缓冲相关的类型定义
    basic_type/visible_type.h                   # 可见性掩码定义
    basic_type/bounding_volumes_types.h         # 包围盒类型定义
    basic_type/bounding_volumes_types.cpp

    basic_type/snapshot.h                       # 快照抽象类
    basic_type/material_param_variant.h         # 材质专用的variant类型
    basic_type/material_param_variant.cpp
    basic_type/texture_type.h                    # 纹理相关类型定义
    basic_type/light_type.h                      # 光源相关类型定义
)
source_group("basic type" FILES ${BASIC_TYPE_SRCS})

set(BASIC_DATA_SRCS
    basic_data/camera.h         # 相机
    basic_data/camera.cpp

    basic_data/mesh.h           # 网格体
    basic_data/mesh.cpp
    basic_data/model.h          # 模型
    basic_data/model.cpp
    basic_data/runtime_texture.h        # 纹理（运行时纹理类，专门用于程序运行时动态创建的纹理）
    basic_data/runtime_texture.cpp      # （外部纹理资产化，由Asset负责管理，共用一个TextureGPUHandle）
    basic_data/transform.h      # 变换
    basic_data/transform.cpp
    basic_data/bounding_volume.h    # AABB/OBB/Sphere等包围盒实现
    basic_data/bounding_volume.cpp
    basic_data/bounding_volumes_intersects.h    # 包围盒相交测试
    basic_data/bounding_volumes_intersects.cpp
    basic_data/ray.h                # 射线类型定义和基础方法
    basic_data/ray.cpp
    basic_data/frustum.h            # 视锥体类型定义和基础方法  
    basic_data/frustum.cpp

)
source_group("basic data" FILES ${BASIC_DATA_SRCS})

set(BASIC_INSTANCE_SRCS
    basic_instance/camera_instance.h  # 相机实例
    basic_instance/camera_instance.cpp
    basic_instance/material_instance.h  # 材质实例
    basic_instance/material_instance.cpp
    basic_instance/mesh_instance.h  # 网格体实例
    basic_instance/mesh_instance.cpp
)
source_group("basic instance" FILES ${BASIC_INSTANCE_SRCS})

set(BASIC_SHADER_SRCS
    basic_shader/framebuffer.h              # 帧缓冲
    basic_shader/framebuffer.cpp
    basic_shader/gbuffer.h                  # 几何缓冲
    basic_shader/gbuffer.cpp
    basic_shader/uniform_buffer.h           # UBO
    basic_shader/shader.h                   # 着色器（目前仅有OpenGL，后续扩展需要重新规划抽象）
    basic_shader/shader.cpp
    basic_shader/shader_cache.h             # 着色器缓存
    basic_shader/shader_cache.cpp
    basic_shader/shader_ubo.h               # Uniform Buffer，固定size的数据结构体
    basic_shader/shader_ubo.cpp
    basic_shader/shader_ssbo.h              # Shader Storge Buffer，可变size的数据结构体
    basic_shader/shader_ssbo.cpp
    basic_shader/shader_binding_point_manager.h     # UBO/SSBO绑定点管理器
    basic_shader/shader_binding_point_manager.cpp
)
source_group("basic shader" FILES ${BASIC_SHADER_SRCS})

set(BASIC_EVENT_SRCS
    basic_event/asset_event.h
    basic_event/render_event.h
    basic_event/instance_event.h
)
source_group("basic event" FILES ${BASIC_EVENT_SRCS})

# 创建静态库
add_library(${ENGINE_DATA_LIB_NAME}  
    STATIC  
    ${BASIC_TYPE_SRCS}
    ${BASIC_DATA_SRCS}
    ${BASIC_INSTANCE_SRCS}
    ${BASIC_EVENT_SRCS}
    ${BASIC_SHADER_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_DATA_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_DATA_LIB_NAME}
    PRIVATE 
    glfw
    shaderc
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Event
)