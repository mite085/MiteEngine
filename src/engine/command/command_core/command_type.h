#ifndef MITE_ENGINE_COMMAND_TYPE
#define MITE_ENGINE_COMMAND_TYPE

#include <cstdint>

namespace mite {
/**
 * @brief 命令句柄
 *
 * 用于在不同线程内传递命令。命令本身由注册表负责创建和维护，
 * 注册表负责多线程安全，CommandHandle为命令传递和获取的唯一凭证。
 */
struct CommandHandle {
  UUID id{};  // 唯一标识，用于在注册表内索引CommandPtr对象

  CommandHandle() = default;
  explicit CommandHandle(const UUID &uuid) : id(uuid) {}
  bool IsValid() const
  {
    return !id.is_nil();
  }
  bool operator==(const CommandHandle &other) const
  {
    return id == other.id;
  }
  bool operator!=(const CommandHandle &other) const
  {
    return id != other.id;
  }
  bool operator<(const CommandHandle &other) const
  {
    return id < other.id;
  }
  std::string ToString() const
  {
    return UUIDGenerator::UUIDToString(id);
  }

  // 静态创建方法
  static CommandHandle Create()
  {
    return CommandHandle(UUIDGenerator::Generate());
  }
};

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

/**
 * @brief 命令执行结果
 * 
 * 负责在命令执行结束时作为Command::Execute()的返回值返回
 * 
 * 使用示例：
 * 
 * // 1: 默认构造
 * mite::CommandResult result1;
 * // result1: success=false, state=PENDING, message="Default constructed result", command = nullptr
 * 
 * // 2: 使用静态工厂方法（注意CommandResult所有权移交）
 * auto result2 = mite::CommandResult::Success(std::move(cmd), "Operation completed");
 * // result2: success=true, state=SUCCEEDED, message="Operation completed"
 * 
 * auto result3 = mite::CommandResult::Failure("File not found", std::move(cmd));
 * // result3: success=false, state=FAILED, message="File not found"
 * 
 * // 3: 参数化构造（注意CommandResult所有权移交）
 * mite::CommandResult result4(true, mite::CommandExecutionState::SUCCEEDED, "Created entity", std::move(cmd));
 * // result4: success=true, state=SUCCEEDED, message="Created entity"
 * 
 * // 4: 布尔检查
 * if (result2) {
 *     LOG_INFO("Command succeeded: {}", result2.message);
 * }
 * 
 * // 5: 字符串表示
 * LOG_DEBUG("Result: {}", result3.ToString());
 * // 输出: "Result: CommandResult{success: false, state: FAILED, message: "File not found"}"
 * 
 * // 6: 有效性检查
 * if (!result1.IsValid()) {
 *     LOG_WARN("Result is default constructed");
 * }
 */
struct CommandResult {
  bool success;                 // 执行是否成功
  CommandExecutionState state;  // 执行后的状态
  std::string message;          // 执行结果消息（可选填充）
  CommandHandle commandHandle;  // 命令句柄（用于撤销重做）

  // ==================== 构造函数 ====================
  /**
   * @brief 默认构造函数
   * 创建默认的失败结果，状态为PENDING
   */
  CommandResult()
      : success(false),
        state(CommandExecutionState::PENDING),
        message("Default constructed result"),
        commandHandle()
  {
  }
  /**
   * @brief 参数化构造函数
   * @param success 执行是否成功
   * @param state 执行状态
   * @param message 结果消息（可选）
   */
  CommandResult(bool success,
                CommandExecutionState state,
                std::string message = "",
                CommandHandle handle = CommandHandle())
      : success(success), state(state), message(message), commandHandle(handle)
  {
  }

  // ==================== 静态工厂方法 ====================
  /**
   * @brief 创建成功结果
   * @param handle 命令句柄（可选）
   * @param message 成功消息
   */
  static CommandResult Success(CommandHandle handle = CommandHandle(),
                               std::string message = "Success")
  {
    return {true, CommandExecutionState::SUCCEEDED, message, handle};
  }
  /**
   * @brief 创建失败结果
   * @param message 失败消息
   * @param handle 命令句柄（可选）
   */
  static CommandResult Failure(std::string message = "Failure",
                               CommandHandle handle = CommandHandle())
  {
    return {false, CommandExecutionState::FAILED, message, handle};
  }
  /**
   * @brief 创建待定结果
   * @param message 待定消息
   */
  static CommandResult Pending(std::string message = "Pending")
  {
    return {false, CommandExecutionState::PENDING, message};
  }

  // ==================== 工具方法 ====================
  /**
   * @brief 检查结果是否有效（非默认构造状态）
   * @return bool 是否有效
   */
  bool IsValid() const
  {
    return state != CommandExecutionState::PENDING || message != "Default constructed result";
  }
  /**
   * @brief 转换为字符串表示（用于调试）
   * @return std::string 字符串表示
   */
  std::string ToString() const
  {
    std::string stateStr;
    switch (state) {
      case CommandExecutionState::PENDING:
        stateStr = "PENDING";
        break;
      case CommandExecutionState::EXECUTING:
        stateStr = "EXECUTING";
        break;
      case CommandExecutionState::SUCCEEDED:
        stateStr = "SUCCEEDED";
        break;
      case CommandExecutionState::FAILED:
        stateStr = "FAILED";
        break;
      case CommandExecutionState::UNDONE:
        stateStr = "UNDONE";
        break;
      case CommandExecutionState::REDONE:
        stateStr = "REDONE";
        break;
      default:
        stateStr = "UNKNOWN";
        break;
    }
    std::string handleStr = commandHandle.IsValid() ? commandHandle.ToString() : "Invalid";

    return "CommandResult{success: " + std::string(success ? "true" : "false") +
           ", state: " + stateStr + ", handle: " + handleStr + ", message: \"" + message + "\"}";
  }
  bool HasCommandHandle() const
  {
    return commandHandle.IsValid();
  }
  const CommandHandle &GetCommandHandle() const
  {
    return commandHandle;
  }
  // ==================== 操作符重载 ====================
  /**
   * @brief 布尔转换操作符
   * @return bool 执行是否成功
   */
  explicit operator bool() const
  {
    return success;
  }
  /**
   * @brief 相等比较操作符
   * @param other 另一个CommandResult
   * @return bool 是否相等
   */
  bool operator==(const CommandResult &other) const
  {
    return success == other.success && state == other.state && message == other.message &&
           commandHandle == other.commandHandle;
  }
  /**
   * @brief 不等比较操作符
   * @param other 另一个CommandResult
   * @return bool 是否不等
   */
  bool operator!=(const CommandResult &other) const
  {
    return !(*this == other);
  }
};

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_TYPE
