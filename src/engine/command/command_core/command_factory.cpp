#include "command_factory.h"

namespace mite {

CommandFactory &CommandFactory::Get()
{
  static CommandFactory instance;
  return instance;
}

CommandPtr CommandFactory::Create(std::type_index typeIndex)
{
  return CommandRegistry::Get().CreateCommand(typeIndex);
}

std::vector<CommandPtr> CommandFactory::CreateByCategory(CommandCategory category)
{
  std::vector<CommandPtr> commands;

  auto typeIndices = CommandRegistry::Get().GetRegisteredCommandTypeIndices();
  for (const auto &typeIndex : typeIndices) {
    if (auto command = Create(typeIndex)) {
      if (command->GetCategory() & category) {
        commands.push_back(std::move(command));
      }
    }
  }

  return commands;
}

std::vector<std::type_index> CommandFactory::GetAvailableCommandTypes() const
{
  return CommandRegistry::Get().GetRegisteredCommandTypeIndices();
}

}  // namespace mite
