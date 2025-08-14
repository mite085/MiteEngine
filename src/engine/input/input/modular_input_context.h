#ifndef MITE_MODULAR_INPUT_CONTEXT
#define MITE_MODULAR_INPUT_CONTEXT

#include "input_context.h"

namespace mite {
/**
 * @brief 模块化输入上下文
 *
 * 功能：
 * 1. 处理器（Processor）管理
 * 2. 事件分发优化
 */
class ModularInputContext : public InputContext {
 public:
  explicit ModularInputContext(const std::string &name);
  ~ModularInputContext();

  // 处理器管理
  void AddProcessor(std::shared_ptr<InputProcessor> processor);
  void RemoveProcessor(const std::string &id);
  void SetProcessorEnabled(const std::string &id, bool enabled);
  std::shared_ptr<InputProcessor> GetProcessor(const std::string &id) const;

  // 调试工具
  void DebugPrintProcessors();

  // 事件处理函数（供EventBus调用）
  bool ProcessEvent(Event &e) override;

 private:
  // 排序
  void _SortProcessors();

  std::vector<std::shared_ptr<InputProcessor>> m_Processors;
  std::vector<InputProcessor *> m_SortedProcessors;  // 按优先级排序的处理器
  std::unordered_map<std::string, size_t> m_ProcessorIndexMap;
  bool m_Dirty = false;  // 需要重新排序标志

  EventBus::HandlerID m_EventHandlerID;  // 用于取消订阅EventBus
};
};  // namespace mite

#endif
