#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY
#define MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY

#include "command.h"

namespace mite {

/**
 * @brief 命令注册表类
 *
 * 负责管理所有命令类型的注册信息，提供类型安全的查找和创建功能
 * 使用type_index确保类型正确性
 * 
 * 使用示例：
 * 1. 注册命令类型（类型安全）
 * CommandRegistry::Get().RegisterCommandType<CreateEntityCommand>();
 * 
 * 2. 创建命令实例（编译时类型检查）
 * auto command = CommandRegistry::Get().CreateCommand<CreateEntityCommand>();
 * if (command) {
 *     auto result = command->Execute();
 * }
 * 
 * 3. 创建命令实例（通过type_index创建）
 * std::type_index createEntityType = typeid(CreateEntityCommand);
 * auto command2 = CommandRegistry::Get().CreateCommand(createEntityType);
 */
class CommandRegistry {
 public:
  // 命令创建函数类型
  using CommandCreator = std::function<CommandPtr()>;

  /**
   * @brief 获取命令注册表单例实例
   * @return CommandRegistry& 注册表引用
   */
  static CommandRegistry &Get();

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
   * @brief 创建命令实例
   * @tparam T 命令类型
   * @return CommandPtr 命令智能指针
   */
  template<typename T> CommandPtr CreateCommand() const;

  /**
   * @brief 创建命令实例（通过type_index）
   * @param typeIndex 命令类型的type_index
   * @return CommandPtr 命令智能指针，如果类型不存在返回nullptr
   */
  CommandPtr CreateCommand(std::type_index typeIndex) const;

  /**
   * @brief 检查命令类型是否已注册
   * @tparam T 命令类型
   * @return bool 是否已注册
   */
  template<typename T> bool IsCommandTypeRegistered() const;

  /**
   * @brief 检查命令类型是否已注册
   * @param typeIndex 命令类型的type_index
   * @return bool 是否已注册
   */
  bool IsCommandTypeRegistered(std::type_index typeIndex) const;

  /**
   * @brief 获取命令类型的友好名称
   * @tparam T 命令类型
   * @return const char* 类型名称
   */
  template<typename T> const char *GetCommandTypeName() const;

  /**
   * @brief 获取命令类型的友好名称
   * @param typeIndex 命令类型的type_index
   * @return const char* 类型名称，如果类型不存在返回nullptr
   */
  const char *GetCommandTypeName(std::type_index typeIndex) const;

  /**
   * @brief 获取所有已注册的命令类型索引
   * @return std::vector<std::type_index> 类型索引列表
   */
  std::vector<std::type_index> GetRegisteredCommandTypeIndices() const;

  /**
   * @brief 清空所有注册的命令类型
   */
  void Clear();

 private:
  CommandRegistry() = default;
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
  };

  std::unordered_map<std::type_index, CommandTypeInfo> m_commandTypes;
};

// 模板方法实现
template<typename T> bool CommandRegistry::RegisterCommandType()
{
  static_assert(std::is_base_of<Command, T>::value, "T must be derived from Command");

  std::type_index typeIndex = typeid(T);

  if (m_commandTypes.find(typeIndex) != m_commandTypes.end()) {
    return false;  // 类型已存在
  }

  m_commandTypes[typeIndex] = {[]() -> CommandPtr { return std::make_unique<T>(); },
                               typeid(T).name()};

  return true;
}

template<typename T> bool CommandRegistry::UnregisterCommandType()
{
  std::type_index typeIndex = typeid(T);
  return m_commandTypes.erase(typeIndex) > 0;
}

template<typename T> CommandPtr CommandRegistry::CreateCommand() const
{
  std::type_index typeIndex = typeid(T);
  auto it = m_commandTypes.find(typeIndex);
  if (it != m_commandTypes.end()) {
    return it->second.creator();
  }
  return nullptr;
}

template<typename T> bool CommandRegistry::IsCommandTypeRegistered() const
{
  std::type_index typeIndex = typeid(T);
  return m_commandTypes.find(typeIndex) != m_commandTypes.end();
}

template<typename T> const char *CommandRegistry::GetCommandTypeName() const
{
  std::type_index typeIndex = typeid(T);
  auto it = m_commandTypes.find(typeIndex);
  return it != m_commandTypes.end() ? it->second.typeName : nullptr;
}

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_REGISTRY
