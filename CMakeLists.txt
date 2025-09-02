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

# Visual Studio编译spdlog时出现编码Warning。
if(MSVC)
    add_compile_options(/wd4828)  # 禁用MSVC的字符集警告
endif()

# 查找 OpenGL 库（跨平台）
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found! Please install OpenGL development libraries.")
endif()

# 启用文件夹功能（Visual Studio专用？）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# 定义第三方库的筛选器名称
set(THIRDPARTY_FOLDER "ThirdParty")
# 添加第三方目录
add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glm)
include_directories(thirdparty/glad)
add_subdirectory(thirdparty/spdlog)
include_directories(thirdparty/stduuid)
include_directories(thirdparty/stduuid/include)
include_directories(thirdparty/cereal/include)
include_directories(thirdparty/stbimg)
add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/meshoptimizer)
add_subdirectory(thirdparty/googletest)

# imgui和imguizmo无cmakelist，为避免污染依赖库，此处手动添加
include(imgui.cmake)
include(imguizmo.cmake)

# 为第三方库设置文件夹属性
if(TARGET spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET assimp)
    set_target_properties(assimp PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET unit)
    set_target_properties(unit PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET Imgui)
    set_target_properties(Imgui PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET ImGuizmo)
    set_target_properties(ImGuizmo PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
if(TARGET meshoptimizer)
    set_target_properties(meshoptimizer PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()
# 添加配置目录
add_subdirectory(config)

# 添加核心目录
add_subdirectory(src)

# 添加测试目录
# 添加测试（如果启用）
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()