cmake_minimum_required(VERSION 3.10)

# 设置库名称
set(IMGUIZMO_LIB_NAME ImGuizmo)

# 创建源代码组(筛选器)
set(IMGUIZMO_CORE_SRCS
    thirdparty/imguizmo/ImGuizmo.cpp
    thirdparty/imguizmo/ImGuizmo.h
)
source_group("core" FILES ${IMGUIZMO_CORE_SRCS})

set(IMGUIZMO_EXTRA_SRCS
    thirdparty/imguizmo/GraphEditor.cpp
    thirdparty/imguizmo/GraphEditor.h
    thirdparty/imguizmo/ImCurveEdit.cpp
    thirdparty/imguizmo/ImCurveEdit.h
    thirdparty/imguizmo/ImGradient.cpp
    thirdparty/imguizmo/ImGradient.h
    thirdparty/imguizmo/ImSequencer.cpp
    thirdparty/imguizmo/ImSequencer.h
    thirdparty/imguizmo/ImZoomSlider.h
)
source_group("extra" FILES ${IMGUIZMO_EXTRA_SRCS})

# 创建静态库
add_library(${IMGUIZMO_LIB_NAME}
    STATIC
    ${IMGUIZMO_CORE_SRCS}
    ${IMGUIZMO_EXTRA_SRCS}
)

# 设置头文件路径
target_include_directories(${IMGUIZMO_LIB_NAME}
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imguizmo  # 公开ImGuizmo自己的头文件
)

# 链接核心库
target_link_libraries(${IMGUIZMO_LIB_NAME}
    PRIVATE 
    # 第三方库依赖
    OpenGL::GL
    glfw
    Imgui
)

# 设置编译定义(如果需要)
target_compile_definitions(${IMGUIZMO_LIB_NAME}
    PRIVATE
)
