#ifndef MITE_INPUT_EVENTS
#define MITE_INPUT_EVENTS

#include "subscription_group.h"

namespace mite {
/**
 * @brief 鼠标移动事件
 */
class MouseMoveEvent : public Event {
 public:
  /**
   * @brief 鼠标移动事件构造函数
   * @param xpos 移动后的位置X
   * @param ypos 移动后的位置Y
   */
  explicit MouseMoveEvent(double xpos, double ypos) : xpos(xpos), ypos(ypos) {}
  glm::vec2 GetPosition() const
  {
    return glm::vec2(xpos, ypos);
  }
  double GetXPos() const
  {
    return xpos;
  }
  double GetYPos() const
  {
    return ypos;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new MouseMoveEvent(xpos, ypos);
  }

 private:
  double xpos, ypos;
};
/**
 * @brief 鼠标按键按下事件
 */
class MouseButtonPressedEvent : public Event {
 public:
  /**
   * @brief 鼠标按键按下事件构造函数
   * @param button 按键的标志，有GLFW_MOUSE_BUTTON_1到8
   * @param mods 按下时配合的修饰键，如GLFW_MOD_SHIFT、GLFW_MOD_CONTROL
   * @param xpos 按下时的位置X
   * @param ypos 按下时的位置Y
   */
  explicit MouseButtonPressedEvent(int button, int mods, double xpos, double ypos)
      : button(button), mods(mods), xpos(xpos), ypos(ypos)
  {
  }
  int GetButton() const
  {
    return button;
  }
  int GetMods() const
  {
    return mods;
  }
  double GetXPos() const
  {
    return xpos;
  }
  double GetYPos() const
  {
    return ypos;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new MouseButtonPressedEvent(button, mods, xpos, ypos);
  }

 private:
  int button, mods;
  double xpos, ypos;
};
/**
 * @brief 鼠标按键释放事件
 */
class MouseButtonReleasedEvent : public Event {
 public:
  /**
   * @brief 鼠标按键释放事件构造函数
   * @param button 释放按键的标志，有GLFW_MOUSE_BUTTON_1到8
   * @param xpos 释放时的位置X
   * @param ypos 释放时的位置Y
   */
  explicit MouseButtonReleasedEvent(int button, double xpos, double ypos)
      : button(button), xpos(xpos), ypos(ypos)
  {
  }
  int GetButton() const
  {
    return button;
  }
  double GetXPos() const
  {
    return xpos;
  }
  double GetYPos() const
  {
    return ypos;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new MouseButtonReleasedEvent(button, xpos, ypos);
  }

 private:
  int button;
  double xpos, ypos;
};
/**
 * @brief 鼠标滚轮滚动事件
 *
 * 当用户滚动鼠标滚轮时触发，包含水平和垂直滚动量
 */
class MouseScrollEvent : public Event {
 public:
  /**
   * @brief 鼠标滚轮滚动事件构造函数
   * @param xoffset 水平滚动量（向右为正）
   * @param yoffset 垂直滚动量（向上为正）
   */
  explicit MouseScrollEvent(double xoffset, double yoffset) : xoffset(xoffset), yoffset(yoffset) {}

  /**
   * @brief 获取水平滚动量
   * @return 水平滚动量（向右为正）
   */
  double GetXOffset() const
  {
    return xoffset;
  }

  /**
   * @brief 获取垂直滚动量
   * @return 垂直滚动量（向上为正）
   */
  double GetYOffset() const
  {
    return yoffset;
  }

  /**
   * @brief 获取滚动量向量
   * @return 包含水平和垂直滚动量的二维向量
   */
  glm::vec2 GetOffset() const
  {
    return glm::vec2(xoffset, yoffset);
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)

  Event *Clone() const override
  {
    return new MouseScrollEvent(xoffset, yoffset);
  }

 private:
  double xoffset, yoffset;
};

/**
 * @brief 键盘按键按下事件
 */
class KeyPressedEvent : public Event {
 public:
  /**
   * @brief 键盘按键按下事件构造函数
   * @param key 按键的标志，以GLFW_KEY_开头的flag
   * @param mods 按下时配合的修饰键，如GLFW_MOD_SHIFT、GLFW_MOD_CONTROL
   * @param isRepeated 是否重复按下的flag
   */
  explicit KeyPressedEvent(int key, int mods, bool isRepeated)
      : key(key), mods(mods), isRepeated(isRepeated)
  {
  }
  int GetKey() const
  {
    return key;
  }
  int GetMods() const
  {
    return mods;
  }
  bool IsRepeated() const
  {
    return isRepeated;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new KeyPressedEvent(key, mods, isRepeated);
  }

 private:
  int key, mods;
  bool isRepeated;
};
/**
 * @brief 键盘按键释放事件
 */
class KeyReleasedEvent : public Event {
 public:
  /**
   * @brief 键盘按键释放事件构造函数
   * @param key 按键的标志，以GLFW_KEY_开头的flag
   */
  explicit KeyReleasedEvent(int key) : key(key) {}

  int GetKey() const
  {
    return key;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new KeyReleasedEvent(key);
  }

 private:
  int key;
};

/**
 * @brief 字符输入事件
 *
 * 当用户输入Unicode字符时触发（考虑组合键、IME输入等情况）
 * 与KeyPressedEvent不同，此事件表示实际输入的字符
 */
class KeyTypedEvent : public Event {
 public:
  /**
   * @brief 字符输入事件构造函数
   * @param codepoint UTF-32编码的字符
   */
  explicit KeyTypedEvent(unsigned int codepoint) : m_Codepoint(codepoint) {}
  /**
   * @brief 获取字符的UTF-32编码
   */
  unsigned int GetCodepoint() const
  {
    return m_Codepoint;
  }
  /**
   * @brief 尝试转换为ASCII字符（如果是可打印ASCII）
   * @return 如果可转换返回char，否则返回0
   */
  char GetAsciiChar() const
  {
    return (m_Codepoint < 128) ? static_cast<char>(m_Codepoint) : '\0';
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_INPUT)
  Event *Clone() const override
  {
    return new KeyTypedEvent(m_Codepoint);
  }

 private:
  unsigned int m_Codepoint;  // UTF-32编码的字符
};
};  // namespace mite

#endif
