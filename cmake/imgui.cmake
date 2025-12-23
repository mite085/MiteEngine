cmake_minimum_required(VERSION 3.10)

# 设置库名称
set(IMGUI_LIB_NAME Imgui)

# 创建源代码组(筛选器)
set(IMGUI_CORE_SRCS
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui.h
    thirdparty/imgui/imconfig.h
    thirdparty/imgui/imgui_internal.h
    thirdparty/imgui/imstb_textedit.h
    thirdparty/imgui/imstb_rectpack.h
    thirdparty/imgui/imstb_truetype.h

    # 以下为需要根据引擎特性，手动添加的部分
    thirdparty/imgui/backends/imgui_impl_glfw.h
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.h
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3_loader.h
)
source_group("core" FILES ${IMGUI_CORE_SRCS})

set(IMGUI_MISC_SRCS
    thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
    thirdparty/imgui/misc/cpp/imgui_stdlib.h
)
source_group("misc" FILES ${IMGUI_MISC_SRCS})

# 创建静态库
add_library(${IMGUI_LIB_NAME}
    STATIC
    ${IMGUI_CORE_SRCS}
    ${IMGUI_MISC_SRCS}
)

# 设置头文件路径
target_include_directories(${IMGUI_LIB_NAME}
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui # 公开ImGui自己的头文件
)

# 链接核心库
target_link_libraries(${IMGUI_LIB_NAME}
    PRIVATE 
    # 第三方库依赖
    OpenGL::GL
    glfw
)

# 设置编译定义(如果需要)
target_compile_definitions(${IMGUI_LIB_NAME}
    PRIVATE
)

# 如果是苹果系统，需要特殊处理
if(APPLE)
    enable_language(OBJCXX)
endif()