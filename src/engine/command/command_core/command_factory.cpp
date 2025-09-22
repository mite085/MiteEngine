#include "command_factory.h"

namespace mite {
// ==================== 单例实现 ====================
CommandFactory &CommandFactory::Get()
{
  static CommandFactory instance;
  return instance;
}

// ==================== 基础命令创建接口实现 ====================
CommandPtr CommandFactory::Create(std::type_index typeIndex)
{
  return CommandRegistry::Get().CreateCommand(typeIndex);
}

// ==================== 批量筛选创建接口实现 ====================
std::vector<CommandPtr> CommandFactory::CreateByCategory(CommandCategory category)
{
  std::vector<CommandPtr> commands;

  auto typeIndices = CommandRegistry::Get().GetCommandTypesByCategory(category);
  for (const auto &typeIndex : typeIndices) {
    if (auto command = Create(typeIndex)) {
      commands.push_back(std::move(command));
    }
  }

  return commands;
}
std::vector<CommandPtr> CommandFactory::CreateByPriority(CommandPriority priority)
{
  std::vector<CommandPtr> commands;

  auto typeIndices = CommandRegistry::Get().GetRegisteredCommandTypeIndices();
  for (const auto &typeIndex : typeIndices) {
    if (CommandRegistry::Get().GetCommandDefaultPriority(typeIndex) == priority) {
      if (auto command = Create(typeIndex)) {
        commands.push_back(std::move(command));
      }
    }
  }

  return commands;
}
std::vector<CommandPtr> CommandFactory::CreateByCategoryAndPriority(CommandCategory category,
                                                                    CommandPriority priority)
{
  std::vector<CommandPtr> commands;

  auto typeIndices = CommandRegistry::Get().GetCommandTypesByCategory(category);
  for (const auto &typeIndex : typeIndices) {
    if (CommandRegistry::Get().GetCommandDefaultPriority(typeIndex) == priority) {
      if (auto command = Create(typeIndex)) {
        commands.push_back(std::move(command));
      }
    }
  }

  return commands;
}

// ==================== 命令可用性检查接口实现 ====================
bool CommandFactory::IsCommandAvailable(std::type_index typeIndex) const
{
  return CommandRegistry::Get().IsCommandTypeRegistered(typeIndex);
}
bool CommandFactory::IsCommandInCategory(std::type_index typeIndex, CommandCategory category) const
{
  return (CommandRegistry::Get().GetCommandCategory(typeIndex) & category) != 0;
}

// ==================== 信息查询接口实现 ====================
std::vector<std::type_index> CommandFactory::GetAvailableCommandTypes() const
{
  return CommandRegistry::Get().GetRegisteredCommandTypeIndices();
}
std::vector<std::type_index> CommandFactory::GetCommandTypesByCategory(
    CommandCategory category) const
{
  return CommandRegistry::Get().GetCommandTypesByCategory(category);
}
std::string CommandFactory::GetCommandTypeName(std::type_index typeIndex) const
{
  return CommandRegistry::Get().GetCommandTypeName(typeIndex);
}

}  // namespace mite
