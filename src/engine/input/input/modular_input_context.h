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

  // 事件处理: 优先使用处理器处理，然后回退到基础动作映射
  virtual bool ProcessEvent(Event &e) override;

  // 调试工具
  void DebugPrintProcessors();

 protected:
  void _SortProcessors();

  std::vector<std::shared_ptr<InputProcessor>> m_Processors;
  std::vector<InputProcessor *> m_SortedProcessors;  // 按优先级排序的处理器
  std::unordered_map<std::string, size_t> m_ProcessorIndexMap;
  bool m_Dirty = false;                      // 需要重新排序标志
  InputProcessor *m_HotProcessor = nullptr;  // 最后成功处理事件的处理器(优化用)
};
};

#endif
