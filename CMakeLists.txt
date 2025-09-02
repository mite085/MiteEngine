cmake_minimum_required(VERSION 3.10)

# 版本组件
set(MiteEngine_VERSION_MAJOR 0)
set(MiteEngine_VERSION_MINOR 1)
set(MiteEngine_VERSION_PATCH 0)
set(MiteEngine_VERSION "${MiteEngine_VERSION_MAJOR}.${MiteEngine_VERSION_MINOR}.${MiteEngine_VERSION_PATCH}")

# 项目基本属性
project(MiteEngine
  VERSION ${MiteEngine_VERSION}
  DESCRIPTION "3D Scene Editor based on OpenGL"
  LANGUAGES CXX C
  HOMEPAGE_URL "https://github.com/mite085/MiteEngine"
)

# 项目配置选项
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

# 打印Make信息
message(STATUS "Building Project name: ${PROJECT_NAME}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Major version: ${MiteEngine_VERSION_MAJOR}")
message(STATUS "Minor version: ${MiteEngine_VERSION_MINOR}")

# 全局编译设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 明确设置编译选项
    add_compile_options(
        "$<$<CONFIG:Release>:/MD>"
        "$<$<CONFIG:Debug>:/MDd>"
        "$<$<CONFIG:RelWithDebInfo>:/MD>"
        "$<$<CONFIG:MinSizeRel>:/MD>"
    )
endif()

# Visual Studio编译spdlog时出现编码Warning。
if(MSVC)
    add_compile_options(/wd4828)  # 禁用MSVC的字符集警告
endif()

# 添加第三方依赖目录
include(thirdparty.cmake)

# 添加配置目录
add_subdirectory(config)

# 添加核心目录
add_subdirectory(src)

# 添加测试目录
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()