#ifndef MITE_UI_PANEL
#define MITE_UI_PANEL

#include "headers/headers.h"

namespace mite {

/**
 * @brief UI面板抽象基类，使用虚函数实现
 */
class UIPanel {
 public:
  explicit UIPanel(const std::string &title) : m_Title(title) {}
  virtual ~UIPanel() = default;

  // 生命周期方法
  virtual void onAttach() {}
  virtual void onDetach() {}
  virtual void onUpdate(float deltaTime) {}
  virtual void onRender() = 0;  // 纯虚函数，必须实现

  // 事件处理
  virtual bool onEvent(Event &event)
  {
    return false;
  }

  // 可见性控制
  bool isVisible() const
  {
    return m_Visible;
  }
  void setVisible(bool visible)
  {
    m_Visible = visible;
  }

  // 标题访问
  const std::string &getTitle() const
  {
    return m_Title;
  }
  void setTitle(const std::string &title)
  {
    m_Title = title;
  }

 protected:
  std::string m_Title;    // 面板标题
  bool m_Visible = true;  // 是否可见
};

};

#endif
