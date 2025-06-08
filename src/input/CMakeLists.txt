cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Input
set(INPUT_LIB_NAME MiteEngine_Input)

# 创建源代码组(筛选器)
set(INPUT_SRCS
    input.h
    input.cpp
    input_context.h
    input_context.cpp
    input_manager.h
    input_manager.cpp
    input_event.h
    input_event.cpp
)
source_group("input" FILES ${INPUT_SRCS})


# 创建静态库
add_library(${INPUT_LIB_NAME}  
    STATIC  
    ${INPUT_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${INPUT_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${INPUT_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
)