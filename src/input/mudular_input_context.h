#ifndef MITE_MODULAR_INPUT_CONTEXT
#define MITE_MODULAR_INPUT_CONTEXT

#include "input_context.h"

namespace mite {
class ModularInputContext : public InputContext {
 public:
  explicit ModularInputContext(const std::string &name);

  // 处理器管理
  void AddProcessor(std::shared_ptr<InputProcessor> processor);
  void RemoveProcessor(const std::string &id);
  void SetProcessorEnabled(const std::string &id, bool enabled);
  std::shared_ptr<InputProcessor> GetProcessor(const std::string &id) const;

  // 输入处理
  bool ProcessEvent(Event &e) override;

  // 调试工具
  void DebugPrintProcessors();

 private:
  void _SortProcessors();

  std::vector<std::shared_ptr<InputProcessor>> m_Processors;
  std::vector<InputProcessor *> m_SortedProcessors;  // 缓存排序结果
  std::unordered_map<std::string, size_t> m_ProcessorIndexMap;
  bool m_Dirty = false;  // 是否需要重新排序
  InputProcessor *m_HotProcessor = nullptr;
};
};

#endif
