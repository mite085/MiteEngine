#ifndef MITE_RUNTIME_UI_EVENTS_H
#define MITE_RUNTIME_UI_EVENTS_H

#include "ui_event.h"

namespace mite {

/**
 * @brief 运行时UI初始化事件
 */
class RuntimeUIInitializedEvent : public Event {
 public:
  RuntimeUIInitializedEvent() = default;

  std::string ToString() const override
  {
    return "RuntimeUIInitializedEvent";
  }

  Event *Clone() const override
  {
    return new RuntimeUIInitializedEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)
};

/**
 * @brief 运行时UI关闭事件
 */
class RuntimeUIShutdownEvent : public Event {
 public:
  RuntimeUIShutdownEvent() = default;

  std::string ToString() const override
  {
    return "RuntimeUIShutdownEvent";
  }

  Event *Clone() const override
  {
    return new RuntimeUIShutdownEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)
};

/**
 * @brief 游戏开始事件
 */
class GameStartEvent : public Event {
 public:
  GameStartEvent() = default;

  std::string ToString() const override
  {
    return "GameStartEvent";
  }

  Event *Clone() const override
  {
    return new GameStartEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)
};

/**
 * @brief 游戏暂停事件
 */
class GamePauseEvent : public Event {
 public:
  explicit GamePauseEvent(bool paused) : m_Paused(paused) {}

  bool IsPaused() const
  {
    return m_Paused;
  }

  std::string ToString() const override
  {
    return "GamePauseEvent: " + std::string(m_Paused ? "PAUSED" : "RESUMED");
  }

  Event *Clone() const override
  {
    return new GamePauseEvent(m_Paused);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)

 private:
  bool m_Paused;
};

/**
 * @brief 游戏停止事件
 */
class GameStopEvent : public Event {
 public:
  GameStopEvent() = default;

  std::string ToString() const override
  {
    return "GameStopEvent";
  }

  Event *Clone() const override
  {
    return new GameStopEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)
};

/**
 * @brief 运行时设置修改事件
 */
class RuntimeSettingChangedEvent : public Event {
 public:
  explicit RuntimeSettingChangedEvent(const std::string &settingName,
                                      const std::string &settingValue)
      : m_SettingName(settingName), m_SettingValue(settingValue)
  {
  }

  const std::string &GetSettingName() const
  {
    return m_SettingName;
  }
  const std::string &GetSettingValue() const
  {
    return m_SettingValue;
  }

  std::string ToString() const override
  {
    return "RuntimeSettingChangedEvent: " + m_SettingName + " = " + m_SettingValue;
  }

  Event *Clone() const override
  {
    return new RuntimeSettingChangedEvent(m_SettingName, m_SettingValue);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)

 private:
  std::string m_SettingName;
  std::string m_SettingValue;
};

/**
 * @brief 运行时性能统计事件
 */
class RuntimePerformanceEvent : public Event {
 public:
  explicit RuntimePerformanceEvent(float fps,
                                   float frameTime,
                                   uint64_t memoryUsage,
                                   uint64_t drawCalls)
      : m_FPS(fps), m_FrameTime(frameTime), m_MemoryUsage(memoryUsage), m_DrawCalls(drawCalls)
  {
  }

  float GetFPS() const
  {
    return m_FPS;
  }
  float GetFrameTime() const
  {
    return m_FrameTime;
  }
  uint64_t GetMemoryUsage() const
  {
    return m_MemoryUsage;
  }
  uint64_t GetDrawCalls() const
  {
    return m_DrawCalls;
  }

  std::string ToString() const override
  {
    return "RuntimePerformanceEvent: FPS=" + std::to_string(m_FPS) +
           ", FrameTime=" + std::to_string(m_FrameTime) + "ms" +
           ", Memory=" + std::to_string(m_MemoryUsage / 1024 / 1024) + "MB" +
           ", DrawCalls=" + std::to_string(m_DrawCalls);
  }

  Event *Clone() const override
  {
    return new RuntimePerformanceEvent(m_FPS, m_FrameTime, m_MemoryUsage, m_DrawCalls);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_RUNTIME)

 private:
  float m_FPS;
  float m_FrameTime;
  uint64_t m_MemoryUsage;
  uint64_t m_DrawCalls;
};

}  // namespace mite

#endif  //
