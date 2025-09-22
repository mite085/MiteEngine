#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_REDO_STACK
#define MITE_ENGINE_COMMAND_CORE_COMMAND_REDO_STACK

#include "command_core/command.h"

namespace mite {

/**
 * @brief 命令重做栈
 *
 * 只负责存储可重做的命令，提供基本的栈操作
 */
class CommandRedoStack {
 public:
  CommandRedoStack();
  ~CommandRedoStack() = default;

  // 禁止拷贝
  CommandRedoStack(const CommandRedoStack &) = delete;
  CommandRedoStack &operator=(const CommandRedoStack &) = delete;

  /**
   * @brief 将命令压入重做栈
   * @param command 要压入的命令
   */
  void Push(CommandPtr command);

  /**
   * @brief 从重做栈弹出命令
   * @return CommandPtr 弹出的命令，如果栈为空返回nullptr
   */
  CommandPtr Pop();

  /**
   * @brief 检查栈是否为空
   * @return bool 是否为空
   */
  bool IsEmpty() const;

  /**
   * @brief 获取栈大小
   * @return size_t 栈中命令数量
   */
  size_t GetSize() const;

  /**
   * @brief 清空重做栈
   */
  void Clear();

  /**
   * @brief 获取栈顶命令（不移除）
   * @return Command* 栈顶命令指针，如果栈为空返回nullptr
   */
  Command *Peek() const;

 private:
  Logger m_Logger;

  mutable std::mutex m_mutex;
  std::stack<CommandPtr> m_stack;
};

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_REDO_STACK
