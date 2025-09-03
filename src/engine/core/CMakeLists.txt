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
set(ENGINE_CORE_LIB_NAME MiteEngine_Engine_Core)

# 创建源代码组(筛选器)
set(FILESYSTEM_SRCS
    filesystem/filesystem.h
    filesystem/filesystem.cpp
    filesystem/filesystem_config.h
)
source_group("filesystem" FILES ${FILESYSTEM_SRCS})

set(FILESYSTEM_CONFIG_SRCS
    ${CMAKE_BINARY_DIR}/src/engine/core/filesystem/filesystem_config.h
)
source_group("filesystem config" FILES ${FILESYSTEM_CONFIG_SRCS})

set(HEADERS_SRCS
    headers/headers.h
)
source_group("header" FILES ${HEADERS_SRCS})

set(LOGGER_SRCS
    logger/logger.h
    logger/logger.cpp
)
source_group("logger" FILES ${LOGGER_SRCS})

set(EVENT_SRCS
    event/callback_adapter.h
    event/callback_adapter.cpp
    event/dispatcher.h
    event/dispatcher.cpp
    event/event.h
    event/event.cpp
    event/event_bus.h
    event/event_bus.cpp
    event/event_types.h
    event/subscription_group.h
    event/subscription_group.cpp
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

set(UUID_SRCS
    uuid/mite_uuid.h
)
source_group("uuid" FILES ${UUID_SRCS})

# 创建静态库
add_library(${ENGINE_CORE_LIB_NAME}
    STATIC
    ${FILESYSTEM_SRCS}
    ${FILESYSTEM_CONFIG_SRCS}
    ${HEADERS_SRCS}
    ${LOGGER_SRCS}
    ${EVENT_SRCS}
    ${TIME_SRCS}
    ${TIMER_SRCS}
    ${UUID_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_CORE_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
    PUBLIC ${CMAKE_BINARY_DIR}/src/engine/core/filesystem/
)

target_link_libraries(${ENGINE_CORE_LIB_NAME}
    PRIVATE 
    spdlog::spdlog
)