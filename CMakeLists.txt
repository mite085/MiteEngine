cmake_minimum_required(VERSION 3.10)

# 版本组件
set(MiteEngine_VERSION_MAJOR 0)
set(MiteEngine_VERSION_MINOR 1)
set(MiteEngine_VERSION_PATCH 0)
set(MiteEngine_VERSION "${MiteEngine_VERSION_MAJOR}.${MiteEngine_VERSION_MINOR}.${MiteEngine_VERSION_PATCH}")

# 项目基本属性
project(MiteEngine
  VERSION ${MiteEngine_VERSION}
  DESCRIPTION "3D Scene Editor based on OpenGL 430"
  LANGUAGES CXX C
  HOMEPAGE_URL "https://github.com/mite085/MiteEngine"
)

# 项目配置选项
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)  # 控制动态/静态库
option(MITEENGINE_INSTALL "Generate install target" OFF) # 安装选项

# 打印Make信息
message(STATUS "Building Project name: ${PROJECT_NAME}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Major version: ${MiteEngine_VERSION_MAJOR}")
message(STATUS "Minor version: ${MiteEngine_VERSION_MINOR}")

# 全局编译设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用编译器扩展，确保跨平台一致性

# 跨平台编译警告设置
if(MSVC)
    # MSVC 编译器设置
    add_compile_options(
        /W4           # 警告等级 4
        /WX           # 将警告视为错误
        /permissive-  # 标准一致性模式
        "$<$<CONFIG:Release>:/MD>"
        "$<$<CONFIG:Debug>:/MDd>"
        "$<$<CONFIG:RelWithDebInfo>:/MD>"
        "$<$<CONFIG:MinSizeRel>:/MD>"
    )
    # 禁用特定警告
    add_compile_options(
        /wd4828  # 字符集警告
        /wd4251  # DLL接口警告（如果构建DLL可能需要）
        /wd4275  # 非DLL接口类警告
    )
else()
    # GCC/Clang 编译器设置
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    # 针对不同编译器优化
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
    endif()
endif()

# 添加第三方依赖目录
include(thirdparty.cmake)

# 添加核心目录
add_subdirectory(src)

# 添加配置目录
add_subdirectory(config)

# 添加测试目录
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()