cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Data
set(DATA_LIB_NAME MiteEngine_Data)

# 创建源代码组(筛选器)
set(BASIC_TYPE_SRCS
    basic_type/asset_type.h
    basic_type/asset_type.cpp
    basic_type/handle_type.h
    basic_type/handle_type.cpp
)
source_group("basic_type" FILES ${BASIC_TYPE_SRCS})

set(DATA_SRCS
    data/camera.h
    data/camera.cpp
    data/mesh.h
    data/mesh.cpp
    data/model.h
    data/model.cpp
    data/texture.h
    data/texture.cpp
)
source_group("data" FILES ${DATA_SRCS})

# 创建静态库
add_library(${DATA_LIB_NAME}  
    STATIC  
    ${BASIC_TYPE_SRCS}
    ${DATA_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${DATA_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${DATA_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
)