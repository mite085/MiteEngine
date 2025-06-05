cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Ui
set(UI_LIB_NAME MiteEngine_Ui)

# 创建源代码组(筛选器)
set(UI_SRCS
    main_window.cpp
    main_window.h
)
source_group("UI" FILES ${UI_SRCS})

# 添加Imgui相关的源文件和头文件
set(IMGUI_SRCS
    ../../thirdparty/imgui/imconfig.h
    ../../thirdparty/imgui/imgui.h
    ../../thirdparty/imgui/imgui.cpp
    ../../thirdparty/imgui/imgui_demo.cpp
    ../../thirdparty/imgui/imgui_draw.cpp
    ../../thirdparty/imgui/imgui_internal.h
    ../../thirdparty/imgui/imgui_tables.cpp
    ../../thirdparty/imgui/imgui_widgets.cpp

    ../../thirdparty/imgui/backends/imgui_impl_glfw.cpp
    ../../thirdparty/imgui/backends/imgui_impl_glfw.h
    ../../thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    ../../thirdparty/imgui/backends/imgui_impl_opengl3.h
    ../../thirdparty/imgui/backends/imgui_impl_opengl3_loader.h
)
source_group("IMGUI" FILES ${IMGUI_SRCS})

# 创建静态库，添加源代码组
add_library(${UI_LIB_NAME} STATIC ${UI_SRCS} ${IMGUI_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${UI_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开当前目录
    PRIVATE "../../thirdparty/imgui/"
)

# 链接核心库
target_link_libraries(${UI_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Core
)