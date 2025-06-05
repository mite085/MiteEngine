cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Assert
set(ASSERT_LIB_NAME MiteEngine_Assert)

# 创建源代码组(筛选器)
set(ASSERT_SRCS
    assert_manager.cpp
    assert_manager.h
)
source_group("assert_manager" FILES ${ASSERT_SRCS})


# 创建静态库
add_library(${ASSERT_LIB_NAME}  
    STATIC  
    ${ASSERT_SRCS}  
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ASSERT_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ASSERT_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
    MiteEngine_Core
)