#include "modular_input_context.h"

namespace mite {
ModularInputContext::ModularInputContext(const std::string &name) : InputContext(name)
{
  // 订阅EventBus中的输入事件，按照EventCategory大类订阅，由ProcessEvent分发
  m_EventHandlerID = EventBus::Get().SubscribeByCategory(EventCategory::EVENT_CATEGORY_INPUT,
                                                         [this](Event &e) { ProcessEvent(e); });
}

ModularInputContext::~ModularInputContext()
{
  // 取消订阅EventBus
  EventBus::Get().Unsubscribe(m_EventHandlerID);
}

void ModularInputContext::AddProcessor(std::shared_ptr<InputProcessor> processor)
{
  const auto &id = processor->GetID();

  if (m_ProcessorIndexMap.find(id) != m_ProcessorIndexMap.end()) {
    m_Logger->warn("Input processor already exists: {}", id);
    return;
  }

  m_Processors.push_back(processor);
  m_ProcessorIndexMap[id] = m_Processors.size() - 1;
  m_Dirty = true;
}

void ModularInputContext::RemoveProcessor(const std::string &id)
{
  auto it = m_ProcessorIndexMap.find(id);
  if (it == m_ProcessorIndexMap.end())
    return;

  size_t index = it->second;
  m_Processors.erase(m_Processors.begin() + index);
  m_ProcessorIndexMap.erase(it);

  // 更新索引映射
  for (auto &pair : m_ProcessorIndexMap) {
    if (pair.second > index)
      pair.second--;
  }

  m_Dirty = true;
}

void ModularInputContext::SetProcessorEnabled(const std::string &id, bool enabled)
{
  auto it = m_ProcessorIndexMap.find(id);
  if (it != m_ProcessorIndexMap.end()) {
    m_Processors[it->second]->SetEnabled(enabled);
  }
}

std::shared_ptr<InputProcessor> ModularInputContext::GetProcessor(const std::string &id) const
{
  auto it = m_ProcessorIndexMap.find(id);
  return it != m_ProcessorIndexMap.end() ? m_Processors[it->second] : nullptr;
}

bool ModularInputContext::ProcessEvent(Event &e)
{
  // 阻塞情况下直接返回
  if (m_BlockInput)
    return true;

  // Processor改版时重新排序
  if (m_Dirty)
    _SortProcessors();

  // 按优先级从高到低，遍历Processor
  for (auto processor : m_SortedProcessors) {
    if (processor->IsEnabled() && processor->HandleEvent(e)) {  // 最高优先级的处理器执行处理操作
      e.handled = true;
      break;  // 高优先级处理器已处理，终止传播
    }
  }
  return e.handled;
}

void ModularInputContext::_SortProcessors()
{
  m_SortedProcessors.clear();
  m_SortedProcessors.reserve(m_Processors.size());

  for (auto &processor : m_Processors) {
    m_SortedProcessors.push_back(processor.get());
  }

  // 按优先级降序排序
  std::sort(m_SortedProcessors.begin(),
            m_SortedProcessors.end(),
            [](const InputProcessor *a, const InputProcessor *b) {
              return a->GetPriority() > b->GetPriority();
            });

  m_Dirty = false;
}

void ModularInputContext::DebugPrintProcessors()
{
  m_Logger->debug("=== Processors in context: {} ===", m_Name);
  for (auto processor : m_SortedProcessors) {
    m_Logger->debug("[Prio {}] {} - Enabled: {}",
                    processor->GetPriority(),
                    processor->GetID(),
                    processor->IsEnabled());
  }
}
};  // namespace mite