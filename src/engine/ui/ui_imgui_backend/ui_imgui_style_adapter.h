#ifndef MITE_IMGUI_STYLE_ADAPTER_H
#define MITE_IMGUI_STYLE_ADAPTER_H

#include "ui_core/ui_style_manager.h"
#include "ui_event/ui_events_lifecycle.h"
#include "imgui.h"

namespace mite {

/**
 * @brief ImGui样式适配器
 * 负责将引擎的UIStyle系统与ImGui的样式系统进行桥接
 */
class ImGuiStyleAdapter {
 public:
  ImGuiStyleAdapter();
  ~ImGuiStyleAdapter() = default;

  // 初始化
  void Initialize();

  // 关闭
  void Shutdown();

  // 从UIStyle应用到ImGui
  bool ApplyUIStyle(const std::shared_ptr<UIStyle> uiStyle);

  // 从ImGui样式导出到UIStyle
  std::shared_ptr<UIStyle> ExportToUIStyle(const std::string &styleName);

  // 获取ImGui样式引用
  ImGuiStyle &GetImGuiStyle();

 private:
  // 映射UIStyle属性到ImGui样式
  void MapUIStyleToImGui(const std::shared_ptr<UIStyle> &uiStyle);

  // 映射颜色属性
  void MapColorProperties(const std::shared_ptr<UIStyle> &uiStyle);

  // 映射尺寸属性
  void MapSizeProperties(const std::shared_ptr<UIStyle> &uiStyle);

  // 映射枚举属性
  void MapEnumProperties(const std::shared_ptr<UIStyle> &uiStyle);

  // 映射边框属性
  void MapBorderProperties(const std::shared_ptr<UIStyle> &uiStyle);

  // 映射间距属性
  void MapSpacingProperties(const std::shared_ptr<UIStyle> &uiStyle);

  // 设置ImGui颜色
  void SetImGuiColor(ImGuiCol colorIndex, const glm::vec4 &color);

  // 创建默认样式映射表
  void CreateDefaultStyleMappings();

  // 样式变更事件响应
  void OnStyleChanged(StyleChangedEvent &event);

  Logger m_Logger;
  ImGuiStyle m_BackupStyle;

  // 样式映射配置
  std::unordered_map<std::string, ImGuiCol> m_ColorMappings;
  std::unordered_map<std::string, std::function<void(ImGuiDir)>> m_EnumMappings;
  std::unordered_map<std::string, std::function<void(float)>> m_SizeMappings;
  std::unordered_map<std::string, std::function<void(bool)>> m_BorderMappings;
  std::unordered_map<std::string, std::function<void(float)>> m_SpacingMappings;

  // 事件订阅
  SubscriptionGroup m_EventSubscriptions;
};

}  // namespace mite

#endif  // MITE_IMGUI_STYLE_ADAPTER_H
