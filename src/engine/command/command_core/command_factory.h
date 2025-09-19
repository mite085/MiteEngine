#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY
#define MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY

#include "command_registry.h"
#include "command_type.h"
#include <memory>
#include <vector>

namespace mite {

/**
 * @brief 命令工厂类
 *
 * 提供高级命令创建和管理功能，支持批量创建、条件创建等
 * 作为CommandRegistry的包装层，提供更友好的接口
 * 
 * 使用示例：
 * 
 * 1. 创建单个命令
 * auto command = CommandFactory::Get().Create<CreateEntityCommand>();
 * 
 * 2. 批量创建命令
 * auto commands = CommandFactory::Get().CreateBatch<MoveEntityCommand>(5);
 * 
 * 3. 创建高优先级命令
 * auto highPriorityCmd = CommandFactory::Get().CreateWithPriority<DeleteEntityCommand>(
 *     CommandPriority::HIGH
 * );
 * 
 * 4. 根据类别筛选命令
 * auto transformCommands = CommandFactory::Get().CreateByCategory(
 *     COMMAND_CATEGORY_TRANSFORM
 * );
 */
class CommandFactory {
 public:
  /**
   * @brief 获取命令工厂单例实例
   * @return CommandFactory& 工厂引用
   */
  static CommandFactory &Get();

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

  /**
   * @brief 创建命令并立即设置优先级
   * @tparam T 命令类型
   * @param priority 命令优先级
   * @return CommandPtr 命令智能指针
   */
  template<typename T> CommandPtr CreateWithPriority(CommandPriority priority);

  /**
   * @brief 创建命令并检查是否在指定上下文中可用
   * @tparam T 命令类型
   * @param contextFlags 上下文标志
   * @return CommandPtr 命令智能指针，如果上下文不匹配返回nullptr
   */
  template<typename T> CommandPtr CreateForContext(uint32_t contextFlags);

  /**
   * @brief 根据类别筛选创建命令
   * @param category 命令类别掩码
   * @return std::vector<CommandPtr> 符合条件的命令列表
   */
  std::vector<CommandPtr> CreateByCategory(CommandCategory category);

  /**
   * @brief 检查命令类型是否可用
   * @tparam T 命令类型
   * @return bool 是否可用（已注册且可以创建）
   */
  template<typename T> bool IsCommandAvailable();

  /**
   * @brief 获取所有可用的命令类型
   * @return std::vector<std::type_index> 可用命令类型列表
   */
  std::vector<std::type_index> GetAvailableCommandTypes() const;

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

 private:
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
};

// 模板方法实现
template<typename T> CommandPtr CommandFactory::Create()
{
  static_assert(std::is_base_of<Command, T>::value, "T must be derived from Command");

  return CommandRegistry::Get().CreateCommand<T>();
}

template<typename T> std::vector<CommandPtr> CommandFactory::CreateBatch(size_t count)
{
  std::vector<CommandPtr> commands;
  commands.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    if (auto command = Create<T>()) {
      commands.push_back(std::move(command));
    }
  }

  return commands;
}

template<typename T> CommandPtr CommandFactory::CreateWithPriority(CommandPriority priority)
{
  if (auto command = Create<T>()) {
    command->SetPriority(priority);
    return command;
  }
  return nullptr;
}

template<typename T> CommandPtr CommandFactory::CreateForContext(uint32_t contextFlags)
{
  // 这里可以根据具体命令类型的上下文要求进行更复杂的检查
  // 目前先简单返回可创建的命令
  return Create<T>();
}

template<typename T> bool CommandFactory::IsCommandAvailable()
{
  return CommandRegistry::Get().IsCommandTypeRegistered<T>();
}

template<typename T> void CommandFactory::PrecreatePool(size_t poolSize)
{
  std::type_index typeIndex = typeid(T);

  auto &pool = m_commandPools[typeIndex];
  if (!pool) {
    pool = std::make_unique<CommandPool<T>>();
  }

  auto *typedPool = static_cast<CommandPool<T> *>(pool.get());
  for (size_t i = 0; i < poolSize; ++i) {
    typedPool->push_back(std::make_unique<T>());
  }
}

template<typename T> CommandPtr CommandFactory::GetFromPool()
{
  std::type_index typeIndex = typeid(T);

  auto it = m_commandPools.find(typeIndex);
  if (it != m_commandPools.end() && it->second) {
    auto *typedPool = static_cast<CommandPool<T> *>(it->second.get());
    if (!typedPool->empty()) {
      CommandPtr command = std::move(typedPool->back());
      typedPool->pop_back();
      return command;
    }
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

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_FACTORY
