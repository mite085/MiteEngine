#ifndef MITE_CORE_EVENT
#define MITE_CORE_EVENT

#include "event/event_types.h"
#include <string>

namespace mite {
/**
 * @brief 事件基类(抽象类)
 *
 * 所有事件都应当派生自该类
 */
class Event {
 public:
  virtual ~Event() = default;

  // 派生类需要重写的方法 ==============================================
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
  virtual Event *Clone() const = 0;

  // 以下方法派生类无需关心 ==============================================
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
   * @brief 标记事件已处理，阻断传播
   */
  void Handled() {
    handled = true;
  }

  // 标记事件是否已被处理
  bool handled = false;
};
}  // namespace mite


// Event派生类辅助宏，用于确定Categories
#define EVENT_CLASS_CATEGORY(category) \
  virtual int GetCategoryFlags() const override \
  { \
    return category; \
  }

#endif
