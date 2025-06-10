#include "modular_input_context.h"

namespace mite {
ModularInputContext::ModularInputContext(const std::string &name) : InputContext(name) {}

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

  // ��������ӳ��
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
  // 1. �������
  if (m_BlockInput)
    return true;

  // 2. ���������ȴ��������ȼ�����
  if (m_Dirty)
    _SortProcessors();

  for (auto processor : m_SortedProcessors) {
    if (!processor->IsEnabled())
      continue;

    if (processor->HandleEvent(e)) {
      // ��¼���ɹ��Ĵ��������ȵ��Ż���
      m_HotProcessor = processor;
      return true;
    }
  }

  // 3. ���˵�����Ķ���ӳ�䴦��
  return InputContext::ProcessEvent(e);
}

void ModularInputContext::_SortProcessors()
{
  m_SortedProcessors.clear();
  m_SortedProcessors.reserve(m_Processors.size());

  for (auto &processor : m_Processors) {
    m_SortedProcessors.push_back(processor.get());
  }

  // �����ȼ���������
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
};
