#ifndef MITE_ENGINE_COMMAND_TYPE
#define MITE_ENGINE_COMMAND_TYPE

#include <cstdint>

namespace mite {

// 命令类别，支持掩码判断
enum CommandCategory : uint64_t {
  COMMAND_CATEGORY_NONE = 0,

  // 基础操作类别
  COMMAND_CATEGORY_SCENE = 1 << 0,      // 场景操作命令
  COMMAND_CATEGORY_ENTITY = 1 << 1,     // 实体操作命令
  COMMAND_CATEGORY_TRANSFORM = 1 << 2,  // 变换操作命令
  COMMAND_CATEGORY_ASSET = 1 << 3,      // 资产操作命令

  // 编辑器特定类别
  COMMAND_CATEGORY_EDITOR = 1 << 8,    // 编辑器操作命令
  COMMAND_CATEGORY_VIEWPORT = 1 << 9,  // 视口操作命令
  COMMAND_CATEGORY_UI = 1 << 10,       // UI操作命令

  // 系统类别
  COMMAND_CATEGORY_SYSTEM = 1 << 15,  // 系统级别命令
};

// 命令执行状态
enum class CommandExecutionState {
  PENDING,    // 命令等待执行
  EXECUTING,  // 命令正在执行
  SUCCEEDED,  // 命令执行成功
  FAILED,     // 命令执行失败
  UNDONE,     // 命令已被撤销
  REDONE      // 命令已被重做
};

// 命令优先级（用于执行顺序控制）
enum class CommandPriority : uint8_t {
  LOW = 0,        // 低优先级命令
  NORMAL = 64,    // 普通优先级命令
  HIGH = 128,     // 高优先级命令
  CRITICAL = 255  // 关键优先级命令
};

// 命令执行上下文标志
enum CommandContextFlags : uint32_t {
  CONTEXT_NONE = 0,
  CONTEXT_EDITOR = 1 << 0,   // 编辑器上下文
  CONTEXT_RUNTIME = 1 << 1,  // 运行时上下文
  CONTEXT_PREVIEW = 1 << 2,  // 预览模式上下文
  CONTEXT_PLAY = 1 << 3,     // 播放模式上下文
};

// 命令合并策略
enum class CommandMergePolicy {
  NONE,           // 不合并命令
  BY_TYPE,        // 按类型合并（相同类型命令合并）
  BY_TARGET,      // 按目标合并（相同目标实体的命令合并）
  BY_TYPE_TARGET  // 按类型和目标合并
};

// 命令执行结果
struct CommandResult {
  bool success;                 // 执行是否成功
  CommandExecutionState state;  // 执行后的状态
  const char *message;          // 执行结果消息（可选）

  CommandResult(bool success, CommandExecutionState state, const char *message = nullptr)
      : success(success), state(state), message(message)
  {
  }

  static CommandResult Success()
  {
    return {true, CommandExecutionState::SUCCEEDED};
  }
  static CommandResult Failure(const char *message = nullptr)
  {
    return {false, CommandExecutionState::FAILED, message};
  }
};

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_TYPE
