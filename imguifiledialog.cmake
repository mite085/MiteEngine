# file: imguifiledialog.cmake (完整版)
cmake_minimum_required(VERSION 3.10)

# 设置库名称
set(IMGUIFILEDIALOG_LIB_NAME ImGuiFileDialog)

# 选项：是否安装（默认关闭，作为子项目使用）
option(IMGUIFILEDIALOG_INSTALL "Install ImGuiFileDialog library and headers" OFF)

# 创建源代码组(筛选器)
set(IMGUIFILEDIALOG_CORE_SRCS
    thirdparty/ImGuiFileDialog/ImGuiFileDialog.cpp
    thirdparty/ImGuiFileDialog/ImGuiFileDialog.h
    thirdparty/ImGuiFileDialog/ImGuiFileDialogConfig.h
)
source_group("core" FILES ${IMGUIFILEDIALOG_CORE_SRCS})

# 创建静态库
add_library(${IMGUIFILEDIALOG_LIB_NAME}
    STATIC
    ${IMGUIFILEDIALOG_CORE_SRCS}
)

# 设置头文件路径
target_include_directories(${IMGUIFILEDIALOG_LIB_NAME}
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ImGuiFileDialog>
    $<INSTALL_INTERFACE:include>
)

# 链接核心库
target_link_libraries(${IMGUIFILEDIALOG_LIB_NAME}
    PRIVATE 
    Imgui  # 依赖ImGui库
)

# 设置编译定义
target_compile_definitions(${IMGUIFILEDIALOG_LIB_NAME}
    PRIVATE
)

# 如果是Unix系统，添加编译选项
if(UNIX)
    target_compile_options(${IMGUIFILEDIALOG_LIB_NAME} 
        PRIVATE -Wno-unknown-pragmas
    )
endif()

# 创建别名目标
if(NOT TARGET ImGuiFileDialog::ImGuiFileDialog)
    add_library(ImGuiFileDialog::ImGuiFileDialog ALIAS ${IMGUIFILEDIALOG_LIB_NAME})
endif()

# 安装配置（可选）
if(IMGUIFILEDIALOG_INSTALL)
    include(GNUInstallDirs)
    
    # 安装库文件
    install(TARGETS ${IMGUIFILEDIALOG_LIB_NAME}
        EXPORT ImGuiFileDialogTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    # 安装头文件
    install(FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ImGuiFileDialog/ImGuiFileDialog.h
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ImGuiFileDialog/ImGuiFileDialogConfig.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    # 安装导出文件
    install(EXPORT ImGuiFileDialogTargets
        FILE ImGuiFileDialogConfig.cmake
        NAMESPACE ImGuiFileDialog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiFileDialog
    )
endif()

# 输出状态信息
message(STATUS "ImGuiFileDialog configured: ${IMGUIFILEDIALOG_LIB_NAME}")
