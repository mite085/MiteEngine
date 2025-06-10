#ifndef MITE_MODULAR_INPUT_CONTEXT
#define MITE_MODULAR_INPUT_CONTEXT

#include "input_context.h"

namespace mite {
class ModularInputContext : public InputContext {
 public:
  explicit ModularInputContext(const std::string &name);

  // ����������
  void AddProcessor(std::shared_ptr<InputProcessor> processor);
  void RemoveProcessor(const std::string &id);
  void SetProcessorEnabled(const std::string &id, bool enabled);
  std::shared_ptr<InputProcessor> GetProcessor(const std::string &id) const;

  // ���봦��
  bool ProcessEvent(Event &e) override;

  // ���Թ���
  void DebugPrintProcessors();

 private:
  void _SortProcessors();

  std::vector<std::shared_ptr<InputProcessor>> m_Processors;
  std::vector<InputProcessor *> m_SortedProcessors;  // ����������
  std::unordered_map<std::string, size_t> m_ProcessorIndexMap;
  bool m_Dirty = false;  // �Ƿ���Ҫ��������
  InputProcessor *m_HotProcessor = nullptr;
};
};

#endif
