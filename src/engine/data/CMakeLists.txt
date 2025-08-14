cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Data
set(ENGINE_DATA_LIB_NAME MiteEngine_Engine_Data)

# 创建源代码组(筛选器)
set(BASIC_TYPE_SRCS
    basic_type/asset_type.h
    basic_type/asset_type.cpp
    basic_type/handle_type.h
    basic_type/handle_type.cpp
)
source_group("basic_type" FILES ${BASIC_TYPE_SRCS})

set(BASIC_DATA_SRCS
    basic_data/camera.h
    basic_data/camera.cpp
    basic_data/gizmo.h
    basic_data/gizmo.cpp
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