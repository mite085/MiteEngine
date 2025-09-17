cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Asset
set(ASSET_LIB_NAME MiteEngine_Asset)

# 创建源代码组(筛选器)
set(ASSET_SRCS
    asset_cache.h
    asset_cache.cpp
    asset_manager.h
    asset_manager.cpp
    asset_utils.h
    asset_utils.cpp
    model_loader.h
    model_loader.cpp
    texture_loader.h
    texture_loader.cpp
)
source_group("asset_manager" FILES ${ASSET_SRCS})


# 创建静态库
add_library(${ASSET_LIB_NAME}  
    STATIC  
    ${ASSET_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ASSET_LIB_NAME}
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/assimp/include
    ${CMAKE_BINARY_DIR}/thirdparty/assimp/include   # Assimp 生成的头文件（config.h）
)

target_link_libraries(${ASSET_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    assimp
    meshoptimizer
    
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
)