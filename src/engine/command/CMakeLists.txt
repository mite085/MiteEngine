cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Command
set(ENGINE_COMMAND_LIB_NAME MiteEngine_Engine_Command)

# 创建源代码组(筛选器)

set(COMMAND_CORE_SRCS
    command_core/command.h              # 命令抽象基类
    command_core/command_type.h         # 基本命令类型定义
    command_core/command_registry.h     # 命令注册管理
    command_core/command_registry.cpp
    command_core/command_factory.h      # 命令工厂
    command_core/command_factory.cpp
)
source_group("command core" FILES ${COMMAND_CORE_SRCS})

set(COMMAND_HISTORY_SRCS
    command_history/command_history.h       # 历史管理核心
    command_history/command_redo_stack.h    # 重做堆栈
    command_history/command_undo_stack.h    # 撤销堆栈
)
source_group("command history" FILES ${COMMAND_HISTORY_SRCS})

set(COMMAND_EXECUTOR_SRCS
    command_executor/command_executor.h              # 执行上下文
    command_executor/command_execution_context.h     # 命令执行器
)
source_group("command executor" FILES ${COMMAND_EXECUTOR_SRCS})

set(COMMAND_BASE_COMMANDS_SRCS
    command_base_commands/command_ui_viewport.h  # 视口操作命令
)
source_group("command base commands" FILES ${COMMAND_BASE_COMMANDS_SRCS})

# 创建静态库，添加源代码组
add_library(${ENGINE_COMMAND_LIB_NAME}
    STATIC
    ${COMMAND_CORE_SRCS}
    ${COMMAND_HISTORY_SRCS}
    ${COMMAND_EXECUTOR_SRCS}
    ${COMMAND_BASE_COMMANDS_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_COMMAND_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开当前目录
)

# 链接核心库
target_link_libraries(${ENGINE_COMMAND_LIB_NAME}
    PRIVATE 
    glfw
    spdlog::spdlog
    
    # 强依赖
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Scene_Core
    MiteEngine_Engine_Scene_Graph
)