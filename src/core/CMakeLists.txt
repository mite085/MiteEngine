cmake_minimum_required(VERSION 3.10)

# Core应当包含的内容

# // 基础设施
# - 内存管理（Allocator/MemoryPool）    
# - 智能指针（RefCounted）              直接使用std标准库的智能指针
# - 日志系统（Logger）                  已集成
# - 调试工具（DebugDraw）               

# // 数学库
# - 向量/矩阵（glm风格的math库）        直接使用glm
# - 几何体（AABB/Ray等）
# - 随机数生成

# // 通用工具
# - 字符串处理
# - 文件系统抽象
# - UUID生成
# - 事件总线（EventBus）

# // 可选模块 
# - 协程支持
# - 序列化框架
# - 配置管理系统
# - 异步任务调度

# 设定VCXProject名为MiteEngine_Core
set(CORE_LIB_NAME MiteEngine_Core)

# 创建源代码组(筛选器)
set(CORE_SRCS
    core_code.cpp
    core_code.h
)
source_group("core" FILES ${CORE_SRCS})

set(HEADERS_SRCS
    headers.h
)
source_group("headers" FILES ${HEADERS_SRCS})

set(LOGGER_SRCS
    logger/logger.h
    logger/logger.cpp
)
source_group("logger" FILES ${LOGGER_SRCS})

set(EVENT_SRCS
    event/event.h
    event/event.cpp
    event/dispatcher.h
    event/dispatcher.cpp
    event/event_types.h
)
source_group("event" FILES ${EVENT_SRCS})

set(TIME_SRCS
    time/time.h
    time/time.cpp
)
source_group("time" FILES ${TIME_SRCS})

set(TIMER_SRCS
    timer/timer.h
    timer/timer.cpp
)
source_group("timer" FILES ${TIMER_SRCS})


# 创建静态库
add_library(${CORE_LIB_NAME}  
    STATIC  
    ${CORE_SRCS}  
    ${HEADERS_SRCS}  
    ${LOGGER_SRCS} 
    ${EVENT_SRCS}
    ${TIME_SRCS}
    ${TIMER_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${CORE_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${CORE_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
)