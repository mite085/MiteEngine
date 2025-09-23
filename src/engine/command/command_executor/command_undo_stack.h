#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND_UNDO_STACK
#define MITE_ENGINE_COMMAND_CORE_COMMAND_UNDO_STACK

#include "command_core/command.h"

namespace mite {

/**
 * @brief 命令撤销栈
 *
 * 只负责存储可撤销的命令，提供基本的栈操作
 */
class CommandUndoStack {
 public:
  /**
   * @brief 构造函数
   * @param maxSize 最大栈大小，0表示无限制
   */
  explicit CommandUndoStack(size_t maxSize = 0);
  ~CommandUndoStack() = default;

  // 禁止拷贝
  CommandUndoStack(const CommandUndoStack &) = delete;
  CommandUndoStack &operator=(const CommandUndoStack &) = delete;

  /**
   * @brief 将命令压入撤销栈
   * @param commandHandle 要压入的命令句柄
   */
  void Push(CommandHandle commandHandle);
  /**
   * @brief 从撤销栈弹出命令
   * @return CommandHandle 弹出的命令句柄，如果栈为空返回空句柄
   */
  CommandHandle Pop();

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
   * @brief 获取最大栈大小
   * @return size_t 最大栈大小，0表示无限制
   */
  size_t GetMaxSize() const;
  /**
   * @brief 设置最大栈大小
   * @param maxSize 最大栈大小，0表示无限制
   */
  void SetMaxSize(size_t maxSize);

  /**
   * @brief 清空撤销栈
   */
  void Clear();

  /**
   * @brief 获取栈顶命令句柄（不移除）
   * @return CommandHandle 栈顶命令句柄，如果栈为空返回空句柄
   */
  CommandHandle Peek() const;

 private:
  Logger m_Logger;

  mutable std::mutex m_Mutex;
  std::stack<CommandHandle> m_Stack;
  size_t m_MaxSize;
};

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND_UNDO_STACK
