cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Event
set(ENGINE_EVENT_LIB_NAME MiteEngine_Engine_Event)

set(EVENT_SRCS
    callback_adapter.h
    dispatcher.h
    event.h
    event.cpp
    event_bus.h
    event_bus.cpp
    event_types.h
    subscription_group.h
    subscription_flags.h
)
source_group("event" FILES ${EVENT_SRCS})


# 创建静态库
add_library(${ENGINE_EVENT_LIB_NAME}
    STATIC
    ${EVENT_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_EVENT_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_EVENT_LIB_NAME}
    PRIVATE 
    # 第三方头文件接口库
    mite_engine_thirdparty_headers
    # 第三方库依赖
    glfw
    spdlog::spdlog
    MiteEngine_Engine_Core
)