#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY
#define MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY

#include "command.h"
#include "command_type.h"

namespace mite {
/**
 * @brief 命令注册表类（线程安全）
 *
 * 负责管理所有命令类型的注册信息，提供类型安全的查找和创建功能
 * 使用type_index确保类型正确性
 *
 * 使用示例：
 * 1. 注册命令类型（类型安全）
 * CommandRegistry::Get().RegisterCommandType<CreateEntityCommand>();
 *
 * 2. 创建命令实例并执行（编译时类型检查）
 * CommandHandle handle = CommandRegistry::Get().CreateCommand<CreateEntityCommand>();
 * CommandPtr command = CommandRegistry::Get().AcquireCommand(handle)
 * if(command)
 *     CommandResult result = command->Execute();
 * 
 *
 * 3. 创建命令实例（通过type_index创建）
 * std::type_index createEntityType = typeid(CreateEntityCommand);
 * CommandHandle handle2 = CommandRegistry::Get().CreateCommand(createEntityType);
 */
class CommandRegistry {
 public:
  // 命令创建函数类型
  using CommandCreator = std::function<CommandPtr()>;
  // 命令类型列表
  using CommandTypeList = std::vector<std::type_index>;

  // ==================== 单例访问接口 ====================
  /**
   * @brief 获取命令注册表单例实例
   * @return CommandRegistry& 注册表引用
   */
  static CommandRegistry &Get();

  // ==================== 命令类型注册接口 ====================
  /**
   * @brief 注册命令类型
   * @tparam T 命令类型（必须继承自Command）
   * @return bool 注册是否成功
   */
  template<typename T> bool RegisterCommandType();
  /**
   * @brief 注销命令类型
   * @tparam T 要注销的命令类型
   * @return bool 注销是否成功
   */
  template<typename T> bool UnregisterCommandType();
  /**
   * @brief 检查命令类型是否已注册
   * @tparam T 命令类型
   * @return bool 是否已注册
   */
  template<typename T> bool IsCommandTypeRegistered() const;
  bool IsCommandTypeRegistered(std::type_index index) const;

  // ==================== 命令创建/获取/存储/释放接口 ====================
  /**
   * @brief 创建命令实例并返回句柄
   * @tparam T 命令类型
   * @return CommandHandle 命令句柄，无效时返回空句柄
   */
  template<typename T> CommandHandle CreateCommand();
  /**
   * @brief 创建命令实例并返回句柄
   * @param typeIndex 命令类型
   * @return CommandHandle 命令句柄
   */
  CommandHandle CreateCommand(std::type_index typeIndex);
  /**
   * @brief 根据句柄获取命令对象（转移所有权）
   * @param handle 命令句柄
   * @param expectedType 期望的命令类型（可选，用于类型安全检查）
   * @return CommandPtr 命令对象，无效时返回nullptr
   */
  CommandPtr AcquireCommand(const CommandHandle &handle,
                            std::type_index expectedType = std::type_index(typeid(void)));
  /**
   * @brief 根据句柄获取命令对象（不转移所有权，仅查看）
   * @param handle 命令句柄
   * @return const Command* 命令对象指针，无效时返回nullptr
   */
  const Command *PeekCommand(const CommandHandle &handle) const;
  /**
   * @brief 存储命令对象并返回句柄
   * @param command 命令对象
   * @return CommandHandle 命令句柄
   */
  CommandHandle StoreCommand(CommandPtr command);
  /**
   * @brief 重新存储命令对象到现有句柄（UndoStack需要该功能）
   * @param handle 现有句柄
   * @param command 命令对象
   * @return bool 是否成功（句柄必须有效且当前无命令关联）
   */
  bool ReStoreCommand(const CommandHandle &handle,
                      CommandPtr command);
  /**
   * @brief 检查句柄是否有关联的命令对象
   * @param handle 命令句柄
   * @return bool 是否有关联的命令
   */
  bool HasCommand(const CommandHandle &handle) const;
  /**
   * @brief 预分配句柄（不关联命令对象）
   * @return CommandHandle 预分配的句柄
   */
  CommandHandle PreAllocateHandle();
  /**
   * @brief 将命令关联到预分配的句柄
   * @param handle 预分配的句柄
   * @param command 命令对象
   * @return bool 是否成功
   */
  bool AssociateCommand(const CommandHandle &handle, CommandPtr command);
  /**
   * @brief 释放命令对象
   * @param handle 命令句柄
   * @return bool 释放是否成功
   */
  bool ReleaseCommand(const CommandHandle &handle);

  // ==================== 命令状态管理接口 ====================
  /**
   * @brief 设置命令状态
   * @param handle 命令句柄
   * @param state 新的状态
   * @return bool 设置是否成功
   */
  bool SetCommandState(const CommandHandle &handle, CommandExecutionState state);
  /**
   * @brief 获取命令状态
   * @param handle 命令句柄
   * @return CommandExecutionState 命令状态，如果句柄无效返回PENDING
   */
  CommandExecutionState GetCommandState(const CommandHandle &handle) const;
  /**
   * @brief 检查命令是否可被执行
   * @param handle 命令句柄
   * @return bool 是否正在执行
   */
  bool IsCommandExecutable(const CommandHandle &handle) const;
  /**
   * @brief 检查命令是否正在执行
   * @param handle 命令句柄
   * @return bool 是否正在执行
   */
  bool IsCommandExecuting(const CommandHandle &handle) const;
  /**
   * @brief 检查命令是否已完成
   * @param handle 命令句柄
   * @return bool 是否已完成
   */
  bool IsCommandCompleted(const CommandHandle &handle) const;

  // ==================== 批量操作接口 ====================
  /**
   * @brief 获取所有活跃的命令句柄
   * @return std::vector<CommandHandle> 活跃句柄列表
   */
  std::vector<CommandHandle> GetActiveHandles() const;
  /**
   * @brief 清空所有命令实例
   */
  void ClearCommands();
  /**
   * @brief 获取活跃命令数量
   * @return size_t 命令数量
   */
  size_t GetActiveCommandCount() const;

  // ==================== 命令类型信息查询接口 ====================
  /**
   * @brief 获取命令类型的友好名称
   * @tparam T 命令类型
   * @return const char* 类型名称
   */
  template<typename T> std::string GetCommandTypeName() const;
  /**
   * @brief 获取命令类型的友好名称
   * @param typeIndex 命令类型的type_index
   * @return const char* 类型名称，如果类型不存在返回nullptr
   */
  std::string GetCommandTypeName(std::type_index typeIndex) const;
  /**
   * @brief 获取命令类型的类别
   * @param typeIndex 命令类型的type_index
   * @return CommandCategory 命令类别
   */
  CommandCategory GetCommandCategory(std::type_index typeIndex) const;
  /**
   * @brief 获取命令类型的默认优先级
   * @param typeIndex 命令类型的type_index
   * @return CommandPriority 默认优先级
   */
  CommandPriority GetCommandDefaultPriority(std::type_index typeIndex) const;

  // ==================== 批量查询接口 ====================
  /**
   * @brief 获取所有已注册的命令类型索引
   * @return CommandTypeList 类型索引列表
   */
  CommandTypeList GetRegisteredCommandTypeIndices() const;
  /**
   * @brief 按类别获取命令类型索引
   * @param category 命令类别掩码
   * @return CommandTypeList 符合条件的类型索引列表
   */
  CommandTypeList GetCommandTypesByCategory(CommandCategory category) const;
  /**
   * @brief 按优先级获取命令类型索引
   * @param priority 命令优先级
   * @return CommandTypeList 符合条件的类型索引列表
   */
  CommandTypeList GetCommandTypesByPriority(CommandPriority priority) const;
  /**
   * @brief 获取注册的命令类型数量
   * @return size_t 类型数量
   */
  size_t GetCommandTypeCount() const;

  // ==================== 注册表管理接口 ====================
  /**
   * @brief 清空所有注册的命令类型
   */
  void Clear();
  /**
   * @brief 检查注册表是否为空
   * @return bool 是否为空
   */
  bool IsEmpty() const;

 private:
  // ==================== 内部辅助方法 ====================
  bool ValidateStateTransition(CommandExecutionState from, CommandExecutionState to);

  // ==================== 私有构造函数和成员 ====================
  CommandRegistry();
  ~CommandRegistry() = default;

  // 禁止拷贝和移动
  CommandRegistry(const CommandRegistry &) = delete;
  CommandRegistry(CommandRegistry &&) = delete;
  CommandRegistry &operator=(const CommandRegistry &) = delete;
  CommandRegistry &operator=(CommandRegistry &&) = delete;

  // 注册信息结构体
  struct CommandTypeInfo {
    CommandCreator creator;
    const char *typeName;
    CommandCategory category;
    CommandPriority defaultPriority;
  };
  // 命令实例信息
  struct CommandInstance {
    CommandPtr command;
    std::type_index type;
    std::chrono::system_clock::time_point createTime;
    CommandExecutionState state;  // 命令执行状态
    bool isAcquired;              // 是否被获取（命令对象是否被移走）

    // 删除默认构造函数
    CommandInstance() = delete;

    // 必须提供 type 参数的构造函数
    CommandInstance(CommandPtr cmd,
                    std::type_index cmdType,
                    CommandExecutionState cmdState = CommandExecutionState::PENDING)
        : command(std::move(cmd)),
          type(cmdType),
          createTime(std::chrono::system_clock::now()),
          state(cmdState),
          isAcquired(false)
    {
    }
  };

  // 日志管理
  Logger m_Logger;

  // 线程安全的数据成员
  mutable std::shared_mutex m_typesMutex;
  std::unordered_map<std::type_index, CommandTypeInfo> m_commandTypes;  // 注册的命令类型
  mutable std::shared_mutex m_instancesMutex;
  std::unordered_map<CommandHandle, CommandInstance> m_commandInstances;  // 所有命令UniquePtr存储
};

// 模板方法实现
template<typename T> bool CommandRegistry::RegisterCommandType()
{
  static_assert(std::is_base_of<Command, T>::value, "T must be derived from Command");
  std::type_index typeIndex = typeid(T);
  std::unique_lock lock(m_typesMutex);

  if (m_commandTypes.find(typeIndex) != m_commandTypes.end()) {
    return false;  // 类型已存在
  }

  // 创建临时实例以获取默认信息
  T tempCommand;
  m_commandTypes[typeIndex] = {[]() -> CommandPtr { return std::make_unique<T>(); },
                               typeid(T).name(),
                               tempCommand.GetCategory(),
                               tempCommand.GetPriority()};

  return true;
}

template<typename T> bool CommandRegistry::UnregisterCommandType()
{
  std::type_index typeIndex = typeid(T);
  std::unique_lock lock(m_typesMutex);
  return m_commandTypes.erase(typeIndex) > 0;
}

template<typename T> bool CommandRegistry::IsCommandTypeRegistered() const
{
  std::type_index typeIndex = typeid(T);
  std::shared_lock lock(m_typesMutex);
  return m_commandTypes.find(typeIndex) != m_commandTypes.end();
}

template<typename T> CommandHandle CommandRegistry::CreateCommand()
{
  static_assert(std::is_base_of<Command, T>::value, "T must be derived from Command");

  std::type_index typeIndex = typeid(T);
  std::shared_lock readLock(m_typesMutex);

  auto it = m_commandTypes.find(typeIndex);
  if (it == m_commandTypes.end()) {
    return CommandHandle();  // 返回无效句柄
  }
  readLock.unlock();

  // 创建命令对象
  auto command = it->second.creator();
  if (!command) {
    return CommandHandle();
  }
  // 存储命令对象
  return StoreCommand(std::move(command));
}

template<typename T> std::string CommandRegistry::GetCommandTypeName() const
{
  std::type_index typeIndex = typeid(T);
  std::shared_lock lock(m_typesMutex);
  auto it = m_commandTypes.find(typeIndex);

  // 检查注册情况
  return it != m_commandTypes.end() ? it->second.typeName : "";
}
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY
