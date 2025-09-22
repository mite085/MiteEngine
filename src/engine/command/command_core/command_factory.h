#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY
#define MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY

#include "command_registry.h"
#include "command_type.h"

namespace mite {
/**
 * @brief 命令工厂类 -- 负责命令的创建
 *
 * 提供高级命令创建和管理功能，支持批量创建、条件创建等
 * 作为CommandRegistry的包装层，提供更友好的接口
 *
 * 使用示例：
 *
 * // 1. 基础命令创建
 * auto moveCommand = CommandFactory::Get().Create<MoveEntityCommand>();
 * if (moveCommand) {
 *     auto result = moveCommand->Execute();
 * }
 *
 * // 2. 带优先级的命令创建
 * auto criticalCommand = CommandFactory::Get().CreateWithPriority<DeleteEntityCommand>(
 *     CommandPriority::CRITICAL
 * );
 *
 * // 3. 带回调的命令创建
 * auto callbackCommand = CommandFactory::Get().CreateWithCallback<CreateEntityCommand>(
 *     [](CommandResult result) {
 *         if (result.success) {
 *             spdlog::info("Entity created successfully!");
 *         }
 *     }
 * );
 *
 * // 4. 批量创建
 * auto transformCommands = CommandFactory::Get().CreateByCategory(COMMAND_CATEGORY_TRANSFORM);
 * for (auto& cmd : transformCommands) {
 *     cmd->Execute();
 * }
 * 
 * // 5. 对象池使用
 * CommandFactory::Get().PrecreatePool<MoveEntityCommand>(10);   // 预创建对象池
 * auto moveCmd = CommandFactory::Get().GetFromPool<MoveEntityCommand>();   // 从对象池获取命令
 * if (moveCmd) {
 *     moveCmd->Execute();
 *     CommandFactory::Get().ReturnToPool<MoveEntityCommand>(std::move(moveCmd)); // 使用后返还到对象池
 * }
 * 
 * // 6. 检查命令可用性
 * if (CommandFactory::Get().IsCommandAvailable<RotateEntityCommand>()) {
 *     auto rotateCmd = CommandFactory::Get().Create<RotateEntityCommand>();
 * }
 */
class CommandFactory {
 public:
  // ==================== 单例访问接口 ====================
  /**
   * @brief 获取命令工厂单例实例
   * @return CommandFactory& 工厂引用
   */
  static CommandFactory &Get();

  // ==================== 基础命令创建接口 ====================
  /**
   * @brief 创建指定类型的命令
   * @tparam T 命令类型
   * @return CommandPtr 命令智能指针
   */
  template<typename T> CommandPtr Create();
  /**
   * @brief 创建指定类型的命令（通过type_index）
   * @param typeIndex 命令类型的type_index
   * @return CommandPtr 命令智能指针，如果类型不存在返回nullptr
   */
  CommandPtr Create(std::type_index typeIndex);
  /**
   * @brief 批量创建多个相同类型的命令
   * @tparam T 命令类型
   * @param count 创建数量
   * @return std::vector<CommandPtr> 命令指针列表
   */
  template<typename T> std::vector<CommandPtr> CreateBatch(size_t count);

  // ==================== 高级命令创建接口 ====================
  /**
   * @brief 创建命令并立即设置优先级
   * @tparam T 命令类型
   * @param priority 命令优先级
   * @return CommandPtr 命令智能指针
   */
  template<typename T> CommandPtr CreateWithPriority(CommandPriority priority);
  /**
   * @brief 创建命令并设置回调函数
   * @tparam T 命令类型
   * @param callback 完成回调函数
   * @return CommandPtr 命令智能指针
   */
  template<typename T> CommandPtr CreateWithCallback(Command::CommandCallback callback);
  /**
   * @brief 创建命令并设置优先级和回调
   * @tparam T 命令类型
   * @param priority 命令优先级
   * @param callback 完成回调函数
   * @return CommandPtr 命令智能指针
   */
  template<typename T>
  CommandPtr CreateWithPriorityAndCallback(CommandPriority priority,
                                           Command::CommandCallback callback);

  // ==================== 批量筛选创建接口 ====================

  /**
   * @brief 根据类别筛选创建命令
   * @param category 命令类别掩码
   * @return std::vector<CommandPtr> 符合条件的命令列表
   */
  std::vector<CommandPtr> CreateByCategory(CommandCategory category);
  /**
   * @brief 根据优先级筛选创建命令
   * @param priority 命令优先级
   * @return std::vector<CommandPtr> 符合条件的命令列表
   */
  std::vector<CommandPtr> CreateByPriority(CommandPriority priority);
  /**
   * @brief 根据类别和优先级筛选创建命令
   * @param category 命令类别掩码
   * @param priority 命令优先级
   * @return std::vector<CommandPtr> 符合条件的命令列表
   */
  std::vector<CommandPtr> CreateByCategoryAndPriority(CommandCategory category,
                                                      CommandPriority priority);

  // ==================== 命令可用性检查接口 ====================
  /**
   * @brief 检查命令类型是否可用
   * @tparam T 命令类型
   * @return bool 是否可用（已注册且可以创建）
   */
  template<typename T> bool IsCommandAvailable();
  /**
   * @brief 检查命令类型是否可用
   * @param typeIndex 命令类型索引
   * @return bool 是否可用
   */
  bool IsCommandAvailable(std::type_index typeIndex) const;
  /**
   * @brief 检查命令类型是否属于指定类别
   * @param typeIndex 命令类型索引
   * @param category 命令类别
   * @return bool 是否属于该类别
   */
  bool IsCommandInCategory(std::type_index typeIndex, CommandCategory category) const;

  // ==================== 信息查询接口 ====================
  /**
   * @brief 获取所有可用的命令类型
   * @return std::vector<std::type_index> 可用命令类型列表
   */
  std::vector<std::type_index> GetAvailableCommandTypes() const;
  /**
   * @brief 获取指定类别的命令类型
   * @param category 命令类别
   * @return std::vector<std::type_index> 符合条件的类型列表
   */
  std::vector<std::type_index> GetCommandTypesByCategory(CommandCategory category) const;
  /**
   * @brief 获取命令类型的友好名称
   * @param typeIndex 命令类型索引
   * @return const char* 类型名称
   */
  std::string GetCommandTypeName(std::type_index typeIndex) const;

  // ==================== 对象池管理接口 ====================
  /**
   * @brief 预创建命令对象池（用于性能优化）
   * @tparam T 命令类型
   * @param poolSize 对象池大小
   */
  template<typename T> void PrecreatePool(size_t poolSize);
  /**
   * @brief 从对象池获取命令（如果可用）
   * @tparam T 命令类型
   * @return CommandPtr 命令智能指针，如果对象池为空则新建
   */
  template<typename T> CommandPtr GetFromPool();
  /**
   * @brief 将命令返回到对象池
   * @tparam T 命令类型
   * @param command 要返还的命令
   */
  template<typename T> void ReturnToPool(CommandPtr command);
  /**
   * @brief 清空指定类型的对象池
   * @tparam T 命令类型
   */
  template<typename T> void ClearPool();

 private:
  // ==================== 私有构造函数和成员 ====================
  CommandFactory() = default;
  ~CommandFactory() = default;

  // 禁止拷贝和移动
  CommandFactory(const CommandFactory &) = delete;
  CommandFactory(CommandFactory &&) = delete;
  CommandFactory &operator=(const CommandFactory &) = delete;
  CommandFactory &operator=(CommandFactory &&) = delete;

  // 对象池类型
  template<typename T> using CommandPool = std::vector<std::unique_ptr<T>>;

  // 对象池映射
  std::unordered_map<std::type_index, std::unique_ptr<void>> m_commandPools;

  // ==================== 对象池辅助函数 ====================
  template<typename T> CommandPool<T> *GetOrCreatePool();
};

// 模板方法实现

// ==================== 基础命令创建接口 ====================
template<typename T> CommandPtr CommandFactory::Create()
{
  // 类型检查
  static_assert(std::is_base_of<Command, T>::value, "T must be derived from Command");
  return CommandRegistry::Get().CreateCommand<T>();
}
template<typename T> std::vector<CommandPtr> CommandFactory::CreateBatch(size_t count)
{
  // 提前分配空间
  std::vector<CommandPtr> commands;
  commands.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    if (auto command = Create<T>()) {
      commands.push_back(std::move(command));
    }
  }

  return commands;
}

// ==================== 高级命令创建接口 ====================
template<typename T> CommandPtr CommandFactory::CreateWithPriority(CommandPriority priority)
{
  if (auto command = Create<T>()) {
    command->SetPriority(priority);
    return command;
  }
  return nullptr;
}
template<typename T>
CommandPtr CommandFactory::CreateWithCallback(Command::CommandCallback callback)
{
  if (auto command = Create<T>()) {
    command->SetCallback(callback);
    return command;
  }
  return nullptr;
}
template<typename T>
CommandPtr CommandFactory::CreateWithPriorityAndCallback(CommandPriority priority,
                                                         Command::CommandCallback callback)
{
  if (auto command = Create<T>()) {
    command->SetPriority(priority);
    command->SetCallback(callback);
    return command;
  }
  return nullptr;
}

// ==================== 命令可用性检查接口 ====================
template<typename T> bool CommandFactory::IsCommandAvailable()
{
  return CommandRegistry::Get().IsCommandTypeRegistered<T>();
}

// ==================== 对象池管理接口 ====================
template<typename T> void CommandFactory::PrecreatePool(size_t poolSize)
{
  // 调用内部接口创建
  auto *pool = GetOrCreatePool<T>();
  for (size_t i = 0; i < poolSize; ++i) {
    pool->push_back(std::make_unique<T>());
  }
}
template<typename T> CommandPtr CommandFactory::GetFromPool()
{
  auto *pool = GetOrCreatePool<T>();
  if (!pool->empty()) {
    CommandPtr command = std::move(pool->back());
    pool->pop_back();
    return command;
  }

  return Create<T>();
}
template<typename T> void CommandFactory::ReturnToPool(CommandPtr command)
{
  if (!command)
    return;

  std::type_index typeIndex = typeid(T);
  auto it = m_commandPools.find(typeIndex);
  if (it != m_commandPools.end() && it->second) {
    auto *typedPool = static_cast<CommandPool<T> *>(it->second.get());

    // 重置命令状态
    command->SetState(CommandExecutionState::PENDING);

    // 将命令放回对象池
    auto *typedCommand = static_cast<T *>(command.release());
    typedPool->push_back(std::unique_ptr<T>(typedCommand));
  }
}
template<typename T> void CommandFactory::ClearPool()
{
  std::type_index typeIndex = typeid(T);
  m_commandPools.erase(typeIndex);
}


// ==================== 对象池辅助函数 ====================
template<typename T> CommandFactory::CommandPool<T> *CommandFactory::GetOrCreatePool()
{
  std::type_index typeIndex = typeid(T);

  // 创建前检查是否存在
  auto it = m_commandPools.find(typeIndex);
  if (it == m_commandPools.end()) {
    auto pool = std::make_unique<CommandPool<T>>();
    auto *poolPtr = pool.get();
    m_commandPools[typeIndex] = std::move(pool);
    return poolPtr;
  }

  return static_cast<CommandPool<T> *>(it->second.get());
}

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY
