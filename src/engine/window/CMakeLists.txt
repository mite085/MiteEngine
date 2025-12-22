cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Window
set(ENGINE_WINDOW_LIB_NAME MiteEngine_Engine_Window)

# 创建源代码组(筛选器)
set(WINDOW_LIB_SRCS
    window.h
    window.cpp
    window_event.h
    window_event.cpp
)
source_group("interface" FILES ${WINDOW_LIB_SRCS})

set(OPENGL_WINDOW_SRCS
    glfw_window/glfw_window.h
    glfw_window/glfw_window.cpp
    glfw_window/glfw_window_callback_adapter.h
    glfw_window/glfw_window_callback_adapter.cpp
    
    ../../../thirdparty/glad/glad.c
)
source_group("opengl window" FILES ${OPENGL_WINDOW_SRCS})

# 创建静态库
add_library(${ENGINE_WINDOW_LIB_NAME}  STATIC  ${WINDOW_LIB_SRCS}  ${OPENGL_WINDOW_SRCS})

# 为glad.c文件禁用pedantic警告（解决函数指针转换错误）
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # 只为glad.c文件禁用特定警告
    set_source_files_properties(
        ../../../thirdparty/glad/glad.c
        PROPERTIES
        COMPILE_FLAGS "-Wno-pedantic -Wno-incompatible-pointer-types"
    )
elseif(MSVC)
    # MSVC编译器：禁用C4055警告（函数指针转换）
    set_source_files_properties(
        ../../../thirdparty/glad/glad.c
        PROPERTIES
        COMPILE_FLAGS "/wd4055"
    )
endif()

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_WINDOW_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PRIVATE ../../../thirdparty/glad    # GLAD头文件路径
)

# 链接核心库
target_link_libraries(${ENGINE_WINDOW_LIB_NAME}
    PRIVATE 
    # 应用严格警告设置
    mite_engine_compile_options
    # 第三方头文件接口库
    mite_engine_thirdparty_headers
    # 第三方库依赖
    glfw
    shaderc
    glslang
    spdlog::spdlog
    ImGuizmo
    MiteEngine_Engine_Core
    MiteEngine_Engine_Event
    MiteEngine_Engine_Input
)