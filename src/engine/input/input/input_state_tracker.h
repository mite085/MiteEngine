#ifndef MITE_INPUT_STATE_TRACKER_H
#define MITE_INPUT_STATE_TRACKER_H

#include "timer/timer.h"
#include "headers/headers.h"

namespace mite {

/**
 * @brief 输入状态跟踪器
 *
 * 职责：
 * 1. 跟踪键盘和鼠标按键的按下/释放状态
 * 2. 记录按键按下的时间戳
 * 3. 处理Timer的自洁行为
 * 4. 提供当前激活按键的查询接口
 */
class InputStateTracker {
 public:
  InputStateTracker();
  ~InputStateTracker() = default;

  // ==================== 状态更新接口 ====================
  /**
   * @brief 处理键盘按键按下事件
   * @param key 按键代码
   */
  void OnKeyPressed(int key);
  /**
   * @brief 处理键盘按键释放事件
   * @param key 按键代码
   */
  void OnKeyReleased(int key);
  /**
   * @brief 处理鼠标按键按下事件
   * @param button 鼠标按键代码
   */
  void OnMouseButtonPressed(int button);
  /**
   * @brief 处理鼠标按键释放事件
   * @param button 鼠标按键代码
   */
  void OnMouseButtonReleased(int button);

  // ==================== 状态查询接口 ====================
  /**
   * @brief 检查按键是否处于按下状态
   * @param key 按键代码
   * @return 是否按下
   */
  bool IsKeyPressed(int key) const;
  /**
   * @brief 检查鼠标按键是否处于按下状态
   * @param button 鼠标按键代码
   * @return 是否按下
   */
  bool IsMouseButtonPressed(int button) const;
  /**
   * @brief 获取按键按下的时间戳（从Timer重置开始计算）
   * @param key 按键代码
   * @return 按下时间（秒），如果未按下返回-1
   */
  float GetKeyPressTime(int key) const;
  /**
   * @brief 获取鼠标按键按下的时间戳
   * @param button 鼠标按键代码
   * @return 按下时间（秒），如果未按下返回-1
   */
  float GetMouseButtonPressTime(int button) const;
  /**
   * @brief 获取所有当前按下的键盘按键
   * @return 按键代码集合
   */
  std::unordered_set<int> GetPressedKeys() const;
  /**
   * @brief 获取所有当前按下的鼠标按键
   * @return 鼠标按键代码集合
   */
  std::unordered_set<int> GetPressedMouseButtons() const;

  // ==================== 工具接口 ====================
  /**
   * @brief 检查是否有任何输入状态被记录
   * @return 是否有活跃的输入状态
   */
  bool HasActiveInput() const;
  /**
   * @brief 清除所有输入状态（同时重置Timer）
   */
  void ClearAllStates();

 private:
  /**
   * @brief 检查并执行Timer自洁行为
   *
   * 当有新的输入事件且此前无任何状态记录时，重置Timer
   * 避免长时间计时导致的float精度问题
   */
  void CheckAndResetTimer();

 private:
  Timer m_Timer;  // 内部计时器

  // 键盘状态存储：按键代码 -> 按下时间戳（秒）
  std::unordered_map<int, float> m_KeyStates;

  // 鼠标按键状态存储：按键代码 -> 按下时间戳（秒）
  std::unordered_map<int, float> m_MouseButtonStates;

  bool m_HasAnyState = false;  // 是否有任何状态记录（用于Timer自洁判断）
};

}  // namespace mite

#endif  // MITE_INPUT_STATE_TRACKER_H
