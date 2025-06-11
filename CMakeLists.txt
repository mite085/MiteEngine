cmake_minimum_required(VERSION 3.10)

# 项目基本属性
project(MiteEngine
  DESCRIPTION
    "MiteEngine"
  LANGUAGES
    CXX C
)


# 全局编译设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 可选，生成编译数据库


# Visual Studio编译spdlog时出现编码问题。
if(MSVC)
    add_compile_options(/utf-8)  # MSVC专用选项，强制MSVC编译器使用UTF-8编码（无效）
    add_compile_options(/wd4828)  # 禁用MSVC的字符集警告（并非正规解决方案，后续需要整改）
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)  # GCC/Clang
endif()


# 启用文件夹功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# 定义第三方库的筛选器名称
set(THIRDPARTY_FOLDER "ThirdParty")

# 添加第三方目录
add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glm)
include_directories(thirdparty/glad)
include_directories(thirdparty/imgui)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/entt)
add_subdirectory(thirdparty/stduuid)

# 为第三方库设置文件夹属性
if(TARGET spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${THIRDPARTY_FOLDER})
endif()



# 添加子目录（核心代码）
add_subdirectory(src)