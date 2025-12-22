#ifndef MITE_CORE_EVENT
#define MITE_CORE_EVENT

#include "event_types.h"
#include "headers/headers.h"

namespace mite {
/**
 * @brief 事件处理结果枚举
 */
enum class EventResult : uint8_t {
  None = 0,           // 未处理，继续传播
  Handled = 1 << 0,   // 已处理，但继续传播（用于中间处理）
  Consumed = 1 << 1,  // 已消费，停止传播（最终处理）
  Failed = 1 << 2,    // 处理失败，但继续传播（错误处理）
  Blocked = 1 << 3,   // 明确阻止传播（权限控制）
  Deferred = 1 << 4,  // 延迟处理，稍后继续

  // 组合标志
  HandledAndStop = Handled | Consumed,  // 已处理，阻断传播
  FailedAndStop = Failed | Consumed     // 已失败，阻断传播
};
/**
 * @brief EventResult辅助函数
 */
namespace EventResultUtil {
inline bool ShouldContinue(EventResult result) {
  uint8_t flags = static_cast<uint8_t>(result);

  // 如果包含以下任一标志，则停止传播
  bool shouldStop =
      (flags & static_cast<uint8_t>(EventResult::Consumed)) != 0 ||  // 已消费
      (flags & static_cast<uint8_t>(EventResult::Blocked)) != 0;     // 被阻止

  return !shouldStop;
}

inline bool WasSuccessful(EventResult result) {
  return (static_cast<uint8_t>(result) &
          static_cast<uint8_t>(EventResult::Failed)) == 0;
}

inline bool WasHandled(EventResult result) {
  uint8_t flags = static_cast<uint8_t>(result);
  return (flags & static_cast<uint8_t>(EventResult::Handled)) != 0 ||
         (flags & static_cast<uint8_t>(EventResult::Consumed)) != 0;
}
}  // namespace EventResultUtil
/**
 * @brief 事件优先级枚举
 */
enum class EventPriority : int {
  Lowest = 0,    // 最低优先级：结果收集、统计、日志等
  Low = 100,     // 低优先级：UI更新、本地化等
  Normal = 200,  // 普通优先级：大多数业务逻辑
  High = 300,    // 高优先级：核心系统处理
  Highest = 400  // 最高优先级：系统级关键处理
};

/**
 * @brief 事件基类(抽象类)
 *
 * 所有事件都应当派生自该类
 *
 * 子类继承示例：
 * 以class WindowResizeEvent: public Event为例
 *
 * WindowResizeEvent(int width, int height)
 * : m_Width(width), m_Height(height) {}
 *
 * Event* Clone() const override {
 *    return new WindowResizeEvent(m_Width, m_Height);
 * }
 */
class Event {
 public:
  virtual ~Event() = default;

  // ====================== 派生类需要重写的方法 ========================
  /**
   * @brief 打印事件相关字符串
   * @return
   *
   * 用于协助Log等系统，负责打印事件信息
   */
  virtual std::string ToString() const;

  /**
   * @brief 克隆事件对象(用于事件队列)
   * @return Event* 新的事件对象指针
   *
   * 使用场景：
   * 若事件需要异步处理，为防止作为局部变量
   * 创建的事件临时变量，生命周期随着事件
   * 发布函数的完成而结束，将其克隆并存储，
   * 留待后续处理。
   */
  virtual Event *Clone() const = 0;

  // ===================== 以下方法派生类无需关心 =========================
  /**
   * @brief 获取事件类别Category
   * @return 事件类别
   *
   * 该纯虚函数无需在子类override，
   * 可以通过宏EVENT_CLASS_CATEGORY直接实现
   */
  virtual int GetCategoryFlags() const = 0;

  /**
   * @brief 判断事件类别Category是否符合输入类别
   * @param category 输入类别
   * @return 是否符合
   */
  bool IsInCategory(EventCategory category);

  /**
   * @brief 设置事件处理结果
   * @param result 处理结果
   */
  void SetResult(EventResult result) { m_Result = result; }
  /**
   * @brief 获取事件处理结果
   * @return 当前处理结果
   */
  EventResult GetResult() const { return m_Result; }
  /**
   * @brief 检查是否应该继续传播
   * @return 是否继续传播
   */
  bool ShouldContinue() const {
    return EventResultUtil::ShouldContinue(m_Result);
  }

 private:
  EventResult m_Result = EventResult::None;
};
}  // namespace mite

// Event派生类辅助宏，用于确定Categories
#define EVENT_CLASS_CATEGORY(category) \
  virtual int GetCategoryFlags() const override { return category; }

// EventResult运算符重载
inline mite::EventResult operator|(mite::EventResult lhs,
                                   mite::EventResult rhs) {
  return static_cast<mite::EventResult>(static_cast<uint8_t>(lhs) |
                                        static_cast<uint8_t>(rhs));
}
inline mite::EventResult operator&(mite::EventResult lhs,
                                   mite::EventResult rhs) {
  return static_cast<mite::EventResult>(static_cast<uint8_t>(lhs) &
                                        static_cast<uint8_t>(rhs));
}
inline mite::EventResult &operator|=(mite::EventResult &lhs,
                                     mite::EventResult rhs) {
  lhs = lhs | rhs;
  return lhs;
}
inline mite::EventResult &operator&=(mite::EventResult &lhs,
                                     mite::EventResult rhs) {
  lhs = lhs & rhs;
  return lhs;
}

#endif
