cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Material
set(ENGINE_MATERIAL_LIB_NAME MiteEngine_Engine_Material)

# 创建源代码组(筛选器)
set(MATERIAL_SRCS
    material_factory.h              # 材质工厂类，负责模板的注册和实例的创建（材质实例资产化，由Asset管理）
    material_factory.cpp
    material_template.h             # 材质模板类，同模板的材质实例共用同一个Shader对象
    material_template.cpp
    gbuffer_material_template.h     # 支持GBuffer的材质基类，GBuffer会将BaseColor、Metallic等PBR参数加载进Buffer，所以必须支持这些参数
    gbuffer_material_template.cpp
)
source_group("material system" FILES ${MATERIAL_SRCS})

set(MATERIAL_TEMPLATES_SRCS
    material_templates/material_template_gltf_pbr.h     # GLTF的pbr实现
    material_templates/material_template_gltf_pbr.cpp
    material_templates/material_template_pure_color.h   # 测试用纯色材质，不包含光照反射。
    material_templates/material_template_pure_color.cpp
)
source_group("material templates" FILES ${MATERIAL_TEMPLATES_SRCS})

# 创建静态库
add_library(${ENGINE_MATERIAL_LIB_NAME}  
    STATIC  
    ${MATERIAL_SRCS}  
    ${MATERIAL_TEMPLATES_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_MATERIAL_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_MATERIAL_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
)