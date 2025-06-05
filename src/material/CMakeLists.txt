cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Material
set(MATERIAL_LIB_NAME MiteEngine_Material)

# 创建源代码组(筛选器)
set(MATERIAL_SRCS
    material_system.cpp
    material_system.h
)
source_group("material_system" FILES ${MATERIAL_SRCS})


# 创建静态库
add_library(${MATERIAL_LIB_NAME}  
    STATIC  
    ${MATERIAL_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${MATERIAL_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PRIVATE "../../thirdparty/"
)

target_link_libraries(${MATERIAL_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
)