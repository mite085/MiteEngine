cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Ui
set(UI_LIB_NAME MiteEngine_Ui)

# 创建源代码组(筛选器)
set(UI_SRCS
    main_window.cpp
    main_window.h
)
source_group("UI" FILES ${UI_SRCS})

# 创建静态库，添加源代码组
add_library(${UI_LIB_NAME} STATIC ${UI_SRCS})

# 设置头文件路径（供其他项目包含）
target_include_directories(${UI_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开当前目录
)

# 链接核心库
target_link_libraries(${UI_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    MiteEngine_Core
)