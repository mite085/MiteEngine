#include "ui_style_manager.h"

#include "ui_event/ui_events_lifecycle.h"

static const std::string s_LightStyleName = "light";
static const std::string s_DarkStyleName = "dark";

namespace mite {
UIStyleManager::UIStyleManager() : m_CurrentStyleName(s_LightStyleName) {
  // 构造函数保持简单，初始化在Initialize()中进行
}

void UIStyleManager::Initialize() {
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI Style Manager");
  m_Logger->info("Initializing UI Style Manager");

  // 创建并注册内置样式
  CreateBuiltinStyles();

  // 设置默认样式为dark
  if (HasStyle(s_DarkStyleName)) {
    SetCurrentStyle(s_DarkStyleName);
    m_Logger->info("Dark style set as current style");
  } else {
    m_Logger->error("Failed to find light style during initialization");
  }
}

bool UIStyleManager::RegisterStyle(const std::string &name,
                                   std::shared_ptr<UIStyle> style) {
  if (name.empty()) {
    m_Logger->warn("Cannot register style with empty name");
    return false;
  }

  if (!style) {
    m_Logger->warn("Cannot register null style: {}", name);
    return false;
  }

  if (m_Styles.find(name) != m_Styles.end()) {
    m_Logger->warn("Style already registered: {}", name);
    return false;
  }

  // 设置样式名称
  style->SetName(name);

  // 注册样式
  m_Styles[name] = style;
  m_Logger->info("Style registered: {}", name);

  return true;
}

std::shared_ptr<UIStyle> UIStyleManager::GetStyle(
    const std::string &name) const {
  auto it = m_Styles.find(name);
  if (it != m_Styles.end()) {
    return it->second;
  }

  m_Logger->warn("Style not found: {}", name);
  return nullptr;
}

std::shared_ptr<UIStyle> UIStyleManager::GetCurrentStyle() const {
  return GetStyle(m_CurrentStyleName);
}

std::string UIStyleManager::GetCurrentStyleName() const {
  return m_CurrentStyleName;
}

bool UIStyleManager::SetCurrentStyle(const std::string &name) {
  if (!HasStyle(name)) {
    m_Logger->warn("Cannot set current style: style not found - {}", name);
    return false;
  }

  std::string oldStyleName = m_CurrentStyleName;
  m_CurrentStyleName = name;

  // 发布样式变更事件
  // 当后端是Imgui时，由ImGuiStyleAdapter负责消费事件，执行样式切换操作
  EventBus::Publish<StyleChangedEvent>(GetStyle(name));

  m_Logger->info("Current style changed from {} to {}", oldStyleName, name);
  return true;
}

bool UIStyleManager::HasStyle(const std::string &name) const {
  return m_Styles.find(name) != m_Styles.end();
}

std::vector<std::string> UIStyleManager::GetAllStyleNames() const {
  std::vector<std::string> names;
  names.reserve(m_Styles.size());

  for (const auto &pair : m_Styles) {
    names.push_back(pair.first);
  }

  return names;
}

size_t UIStyleManager::GetStyleCount() const { return m_Styles.size(); }

void UIStyleManager::CreateBuiltinStyles() {
  // 注册暗色主题
  auto darkTheme = CreateDarkTheme();
  if (darkTheme) {
    RegisterStyle(s_DarkStyleName, darkTheme);
  }

  // 注册亮色主题
  auto lightTheme = CreateLightTheme();
  if (lightTheme) {
    RegisterStyle(s_LightStyleName, lightTheme);
  }

  m_Logger->info("Built-in styles created and registered");
}

std::shared_ptr<UIStyle> UIStyleManager::CreateDarkTheme() {
  auto style = std::make_shared<UIStyle>(s_DarkStyleName);
  // ===== 暗色主题 - 淡紫色系 =====

  // 基础颜色
  style->SetProperty(StyleProperties::COLOR_BACKGROUND,
                     glm::vec4(0.12f, 0.12f, 0.15f, 1.0f), "dark background");
  style->SetProperty(StyleProperties::COLOR_TEXT,
                     glm::vec4(0.95f, 0.95f, 0.98f, 1.0f), "dark text");
  style->SetProperty(StyleProperties::COLOR_BORDER,
                     glm::vec4(0.35f, 0.30f, 0.45f, 1.0f), "dark border");
  style->SetProperty(StyleProperties::COLOR_HOVER,
                     glm::vec4(0.25f, 0.22f, 0.32f, 1.0f), "dark hover");
  style->SetProperty(StyleProperties::COLOR_ACTIVE,
                     glm::vec4(0.70f, 0.55f, 0.95f, 1.0f),
                     "dark active - lavender");
  style->SetProperty(StyleProperties::COLOR_DISABLED,
                     glm::vec4(0.40f, 0.40f, 0.45f, 0.60f), "dark disabled");

  // 弹出窗口背景
  style->SetProperty(StyleProperties::COLOR_POPUP_BG,
                     glm::vec4(0.15f, 0.15f, 0.18f, 0.98f),
                     "dark popup background");

  // 框架背景
  style->SetProperty(StyleProperties::COLOR_FRAME_BG,
                     glm::vec4(0.18f, 0.18f, 0.22f, 1.0f),
                     "dark frame background");
  style->SetProperty(StyleProperties::COLOR_FRAME_BG_HOVER,
                     glm::vec4(0.25f, 0.22f, 0.32f, 1.0f), "dark frame hover");
  style->SetProperty(StyleProperties::COLOR_FRAME_BG_ACTIVE,
                     glm::vec4(0.30f, 0.25f, 0.40f, 1.0f), "dark frame active");
  // 标题栏
  style->SetProperty(StyleProperties::COLOR_TITLE_BG,
                     glm::vec4(0.20f, 0.18f, 0.28f, 1.0f),
                     "dark title background");
  style->SetProperty(StyleProperties::COLOR_TITLE_BG_ACTIVE,
                     glm::vec4(0.35f, 0.28f, 0.48f, 1.0f), "dark title active");
  style->SetProperty(StyleProperties::COLOR_TITLE_BG_COLLAPSED,
                     glm::vec4(0.20f, 0.18f, 0.28f, 0.75f),
                     "dark title collapsed");
  // 菜单栏
  style->SetProperty(StyleProperties::COLOR_MENU_BAR_BG,
                     glm::vec4(0.22f, 0.20f, 0.30f, 1.0f), "dark menu bar");
  // 按钮
  style->SetProperty(StyleProperties::COLOR_BUTTON,
                     glm::vec4(0.35f, 0.28f, 0.48f, 1.0f), "dark button");
  style->SetProperty(StyleProperties::COLOR_BUTTON_HOVER,
                     glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), "dark button hover");
  style->SetProperty(StyleProperties::COLOR_BUTTON_ACTIVE,
                     glm::vec4(0.55f, 0.42f, 0.78f, 1.0f),
                     "dark button active");
  // 标题/折叠区域
  style->SetProperty(StyleProperties::COLOR_HEADER,
                     glm::vec4(0.30f, 0.25f, 0.40f, 1.0f), "dark header");
  style->SetProperty(StyleProperties::COLOR_HEADER_HOVER,
                     glm::vec4(0.40f, 0.32f, 0.55f, 1.0f), "dark header hover");
  style->SetProperty(StyleProperties::COLOR_HEADER_ACTIVE,
                     glm::vec4(0.50f, 0.38f, 0.70f, 1.0f),
                     "dark header active");
  // 滚动条
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_BG,
                     glm::vec4(0.15f, 0.15f, 0.18f, 1.0f),
                     "dark scrollbar background");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB,
                     glm::vec4(0.35f, 0.28f, 0.48f, 1.0f),
                     "dark scrollbar grab");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB_HOVER,
                     glm::vec4(0.45f, 0.35f, 0.65f, 1.0f),
                     "dark scrollbar hover");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB_ACTIVE,
                     glm::vec4(0.55f, 0.42f, 0.78f, 1.0f),
                     "dark scrollbar active");
  // 滑块
  style->SetProperty(StyleProperties::COLOR_SLIDER_GRAB,
                     glm::vec4(0.55f, 0.42f, 0.78f, 1.0f), "dark slider grab");
  style->SetProperty(StyleProperties::COLOR_SLIDER_GRAB_ACTIVE,
                     glm::vec4(0.65f, 0.50f, 0.90f, 1.0f),
                     "dark slider active");
  // 复选框标记
  style->SetProperty(StyleProperties::COLOR_CHECK_MARK,
                     glm::vec4(0.95f, 0.95f, 0.98f, 1.0f), "dark check mark");
  // 标签页
  style->SetProperty(StyleProperties::COLOR_TAB,
                     glm::vec4(0.25f, 0.22f, 0.32f, 1.0f), "dark tab");
  style->SetProperty(StyleProperties::COLOR_TAB_HOVER,
                     glm::vec4(0.35f, 0.28f, 0.48f, 1.0f), "dark tab hover");
  style->SetProperty(StyleProperties::COLOR_TAB_ACTIVE,
                     glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), "dark tab active");
  style->SetProperty(StyleProperties::COLOR_TAB_UNFOCUSED,
                     glm::vec4(0.20f, 0.18f, 0.28f, 1.0f),
                     "dark tab unfocused");
  style->SetProperty(StyleProperties::COLOR_TAB_UNFOCUSED_ACTIVE,
                     glm::vec4(0.30f, 0.25f, 0.40f, 1.0f),
                     "dark tab unfocused active");
  // 表格
  style->SetProperty(StyleProperties::COLOR_TABLE_HEADER_BG,
                     glm::vec4(0.20f, 0.18f, 0.28f, 1.0f), "dark table header");
  style->SetProperty(StyleProperties::COLOR_TABLE_ROW_BG,
                     glm::vec4(0.15f, 0.15f, 0.18f, 1.0f), "dark table row");
  style->SetProperty(StyleProperties::COLOR_TABLE_ROW_BG_ALT,
                     glm::vec4(0.18f, 0.18f, 0.22f, 1.0f),
                     "dark table row alt");
  // 文本选择
  style->SetProperty(StyleProperties::COLOR_TEXT_SELECTED_BG,
                     glm::vec4(0.35f, 0.28f, 0.48f, 0.70f),
                     "dark text selection");
  // 导航高亮
  style->SetProperty(StyleProperties::COLOR_NAV_HIGHLIGHT,
                     glm::vec4(0.55f, 0.42f, 0.78f, 1.0f),
                     "dark nav highlight");
  // ===== 圆角设置 =====
  style->SetProperty(StyleProperties::SIZE_WINDOW_ROUNDING, 12.0f,
                     "window rounding");
  style->SetProperty(StyleProperties::SIZE_CHILD_ROUNDING, 12.0f,
                     "child rounding");
  style->SetProperty(StyleProperties::SIZE_FRAME_ROUNDING, 10.0f,
                     "frame rounding");
  style->SetProperty(StyleProperties::SIZE_POPUP_ROUNDING, 12.0f,
                     "popup rounding");
  style->SetProperty(StyleProperties::SIZE_SCROLLBAR_ROUNDING, 10.0f,
                     "scrollbar rounding");
  style->SetProperty(StyleProperties::SIZE_GRAB_ROUNDING, 8.0f,
                     "grab rounding");
  style->SetProperty(StyleProperties::SIZE_TAB_ROUNDING, 10.0f, "tab rounding");
  // ===== 边框设置 =====
  style->SetProperty(StyleProperties::BORDER_WINDOW, true, "window border");
  style->SetProperty(StyleProperties::BORDER_CHILD, true, "child border");
  style->SetProperty(StyleProperties::BORDER_POPUP, true, "popup border");
  style->SetProperty(StyleProperties::BORDER_FRAME, true, "frame border");
  style->SetProperty(StyleProperties::BORDER_TAB, true, "tab border");
  style->SetProperty(StyleProperties::SIZE_CHILD_BORDER_SIZE, 1.0f,
                     "child border size");
  style->SetProperty(StyleProperties::SIZE_POPUP_BORDER_SIZE, 1.0f,
                     "popup border size");
  style->SetProperty(StyleProperties::SIZE_FRAME_BORDER_SIZE, 1.0f,
                     "frame border size");
  style->SetProperty(StyleProperties::SIZE_TAB_BORDER_SIZE, 1.0f,
                     "tab border size");
  // ===== 间距设置 =====
  style->SetProperty(StyleProperties::SPACING_WINDOW_PADDING_X, 8.0f,
                     "window padding x");
  style->SetProperty(StyleProperties::SPACING_WINDOW_PADDING_Y, 8.0f,
                     "window padding y");
  style->SetProperty(StyleProperties::SPACING_FRAME_PADDING_X, 6.0f,
                     "frame padding x");
  style->SetProperty(StyleProperties::SPACING_FRAME_PADDING_Y, 4.0f,
                     "frame padding y");
  style->SetProperty(StyleProperties::SPACING_ITEM_SPACING_X, 8.0f,
                     "item spacing x");
  style->SetProperty(StyleProperties::SPACING_ITEM_SPACING_Y, 4.0f,
                     "item spacing y");
  style->SetProperty(StyleProperties::SPACING_ITEM_INNER_SPACING_X, 4.0f,
                     "item inner spacing x");
  style->SetProperty(StyleProperties::SPACING_ITEM_INNER_SPACING_Y, 4.0f,
                     "item inner spacing y");
  return style;
}

std::shared_ptr<UIStyle> UIStyleManager::CreateLightTheme() {
  auto style = std::make_shared<UIStyle>(s_LightStyleName);
  // ===== 明亮主题 - 橙色系 =====

  // 基础颜色
  style->SetProperty(StyleProperties::COLOR_BACKGROUND,
                     glm::vec4(0.96f, 0.96f, 0.97f, 1.0f), "light background");
  style->SetProperty(StyleProperties::COLOR_TEXT,
                     glm::vec4(0.10f, 0.10f, 0.12f, 1.0f), "light text");
  style->SetProperty(StyleProperties::COLOR_BORDER,
                     glm::vec4(0.85f, 0.75f, 0.65f, 1.0f),
                     "light border - warm gray");
  style->SetProperty(StyleProperties::COLOR_HOVER,
                     glm::vec4(0.95f, 0.85f, 0.75f, 1.0f), "light hover");
  style->SetProperty(StyleProperties::COLOR_ACTIVE,
                     glm::vec4(1.00f, 0.65f, 0.30f, 1.0f),
                     "light active - orange");
  style->SetProperty(StyleProperties::COLOR_DISABLED,
                     glm::vec4(0.80f, 0.80f, 0.82f, 0.60f), "light disabled");

  // 弹出窗口背景
  style->SetProperty(StyleProperties::COLOR_POPUP_BG,
                     glm::vec4(1.00f, 0.98f, 0.96f, 0.98f),
                     "light popup background");

  // 框架背景
  style->SetProperty(StyleProperties::COLOR_FRAME_BG,
                     glm::vec4(0.98f, 0.98f, 0.99f, 1.0f),
                     "light frame background");
  style->SetProperty(StyleProperties::COLOR_FRAME_BG_HOVER,
                     glm::vec4(1.00f, 0.95f, 0.90f, 1.0f), "light frame hover");
  style->SetProperty(StyleProperties::COLOR_FRAME_BG_ACTIVE,
                     glm::vec4(1.00f, 0.90f, 0.80f, 1.0f),
                     "light frame active");
  // 标题栏
  style->SetProperty(StyleProperties::COLOR_TITLE_BG,
                     glm::vec4(1.00f, 0.85f, 0.65f, 1.0f),
                     "light title background");
  style->SetProperty(StyleProperties::COLOR_TITLE_BG_ACTIVE,
                     glm::vec4(1.00f, 0.75f, 0.45f, 1.0f),
                     "light title active");
  style->SetProperty(StyleProperties::COLOR_TITLE_BG_COLLAPSED,
                     glm::vec4(1.00f, 0.85f, 0.65f, 0.75f),
                     "light title collapsed");
  // 菜单栏
  style->SetProperty(StyleProperties::COLOR_MENU_BAR_BG,
                     glm::vec4(1.00f, 0.88f, 0.70f, 1.0f), "light menu bar");
  // 按钮
  style->SetProperty(StyleProperties::COLOR_BUTTON,
                     glm::vec4(1.00f, 0.75f, 0.45f, 1.0f), "light button");
  style->SetProperty(StyleProperties::COLOR_BUTTON_HOVER,
                     glm::vec4(1.00f, 0.85f, 0.55f, 1.0f),
                     "light button hover");
  style->SetProperty(StyleProperties::COLOR_BUTTON_ACTIVE,
                     glm::vec4(1.00f, 0.65f, 0.35f, 1.0f),
                     "light button active");
  // 标题/折叠区域
  style->SetProperty(StyleProperties::COLOR_HEADER,
                     glm::vec4(1.00f, 0.90f, 0.80f, 1.0f), "light header");
  style->SetProperty(StyleProperties::COLOR_HEADER_HOVER,
                     glm::vec4(1.00f, 0.95f, 0.85f, 1.0f),
                     "light header hover");
  style->SetProperty(StyleProperties::COLOR_HEADER_ACTIVE,
                     glm::vec4(1.00f, 0.85f, 0.70f, 1.0f),
                     "light header active");
  // 滚动条
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_BG,
                     glm::vec4(0.92f, 0.92f, 0.94f, 1.0f),
                     "light scrollbar background");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB,
                     glm::vec4(1.00f, 0.75f, 0.45f, 1.0f),
                     "light scrollbar grab");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB_HOVER,
                     glm::vec4(1.00f, 0.85f, 0.55f, 1.0f),
                     "light scrollbar hover");
  style->SetProperty(StyleProperties::COLOR_SCROLLBAR_GRAB_ACTIVE,
                     glm::vec4(1.00f, 0.65f, 0.35f, 1.0f),
                     "light scrollbar active");
  // 滑块
  style->SetProperty(StyleProperties::COLOR_SLIDER_GRAB,
                     glm::vec4(1.00f, 0.65f, 0.35f, 1.0f), "light slider grab");
  style->SetProperty(StyleProperties::COLOR_SLIDER_GRAB_ACTIVE,
                     glm::vec4(1.00f, 0.55f, 0.25f, 1.0f),
                     "light slider active");

  // 复选框标记
  style->SetProperty(StyleProperties::COLOR_CHECK_MARK,
                     glm::vec4(0.10f, 0.10f, 0.12f, 1.0f), "light check mark");

  // 标签页
  style->SetProperty(StyleProperties::COLOR_TAB,
                     glm::vec4(1.00f, 0.90f, 0.80f, 1.0f), "light tab");
  style->SetProperty(StyleProperties::COLOR_TAB_HOVER,
                     glm::vec4(1.00f, 0.95f, 0.85f, 1.0f), "light tab hover");
  style->SetProperty(StyleProperties::COLOR_TAB_ACTIVE,
                     glm::vec4(1.00f, 0.85f, 0.70f, 1.0f), "light tab active");
  style->SetProperty(StyleProperties::COLOR_TAB_UNFOCUSED,
                     glm::vec4(1.00f, 0.95f, 0.90f, 1.0f),
                     "light tab unfocused");
  style->SetProperty(StyleProperties::COLOR_TAB_UNFOCUSED_ACTIVE,
                     glm::vec4(1.00f, 0.88f, 0.75f, 1.0f),
                     "light tab unfocused active");

  // 表格
  style->SetProperty(StyleProperties::COLOR_TABLE_HEADER_BG,
                     glm::vec4(1.00f, 0.85f, 0.65f, 1.0f),
                     "light table header");
  style->SetProperty(StyleProperties::COLOR_TABLE_ROW_BG,
                     glm::vec4(0.98f, 0.98f, 0.99f, 1.0f), "light table row");
  style->SetProperty(StyleProperties::COLOR_TABLE_ROW_BG_ALT,
                     glm::vec4(0.96f, 0.96f, 0.97f, 1.0f),
                     "light table row alt");

  // 文本选择
  style->SetProperty(StyleProperties::COLOR_TEXT_SELECTED_BG,
                     glm::vec4(1.00f, 0.75f, 0.45f, 0.70f),
                     "light text selection");

  // 导航高亮
  style->SetProperty(StyleProperties::COLOR_NAV_HIGHLIGHT,
                     glm::vec4(1.00f, 0.65f, 0.35f, 1.0f),
                     "light nav highlight");

  // 图表颜色
  style->SetProperty(StyleProperties::COLOR_PLOT_LINES,
                     glm::vec4(1.00f, 0.65f, 0.30f, 1.0f), "light plot lines");
  style->SetProperty(StyleProperties::COLOR_PLOT_LINES_HOVER,
                     glm::vec4(1.00f, 0.55f, 0.20f, 1.0f),
                     "light plot lines hover");
  style->SetProperty(StyleProperties::COLOR_PLOT_HISTOGRAM,
                     glm::vec4(1.00f, 0.75f, 0.45f, 1.0f),
                     "light plot histogram");
  style->SetProperty(StyleProperties::COLOR_PLOT_HISTOGRAM_HOVER,
                     glm::vec4(1.00f, 0.65f, 0.35f, 1.0f),
                     "light plot histogram hover");

  // 拖拽目标
  style->SetProperty(StyleProperties::COLOR_DRAG_DROP_TARGET,
                     glm::vec4(1.00f, 0.65f, 0.30f, 0.90f),
                     "light drag drop target");

  // 模态窗口背景
  style->SetProperty(StyleProperties::COLOR_MODAL_WINDOW_DIM_BG,
                     glm::vec4(0.20f, 0.20f, 0.20f, 0.35f),
                     "light modal dim background");

  // ===== 圆角设置（与暗色主题保持一致） =====
  style->SetProperty(StyleProperties::SIZE_WINDOW_ROUNDING, 12.0f,
                     "window rounding");
  style->SetProperty(StyleProperties::SIZE_CHILD_ROUNDING, 12.0f,
                     "child rounding");
  style->SetProperty(StyleProperties::SIZE_FRAME_ROUNDING, 10.0f,
                     "frame rounding");
  style->SetProperty(StyleProperties::SIZE_POPUP_ROUNDING, 12.0f,
                     "popup rounding");
  style->SetProperty(StyleProperties::SIZE_SCROLLBAR_ROUNDING, 10.0f,
                     "scrollbar rounding");
  style->SetProperty(StyleProperties::SIZE_GRAB_ROUNDING, 8.0f,
                     "grab rounding");
  style->SetProperty(StyleProperties::SIZE_TAB_ROUNDING, 10.0f, "tab rounding");

  // ===== 边框设置 =====
  style->SetProperty(StyleProperties::BORDER_WINDOW, true, "window border");
  style->SetProperty(StyleProperties::BORDER_CHILD, true, "child border");
  style->SetProperty(StyleProperties::BORDER_POPUP, true, "popup border");
  style->SetProperty(StyleProperties::BORDER_FRAME, true, "frame border");
  style->SetProperty(StyleProperties::BORDER_TAB, true, "tab border");

  style->SetProperty(StyleProperties::SIZE_CHILD_BORDER_SIZE, 1.0f,
                     "child border size");
  style->SetProperty(StyleProperties::SIZE_POPUP_BORDER_SIZE, 1.0f,
                     "popup border size");
  style->SetProperty(StyleProperties::SIZE_FRAME_BORDER_SIZE, 1.0f,
                     "frame border size");
  style->SetProperty(StyleProperties::SIZE_TAB_BORDER_SIZE, 1.0f,
                     "tab border size");

  // ===== 间距设置 =====
  style->SetProperty(StyleProperties::SPACING_WINDOW_PADDING_X, 8.0f,
                     "window padding x");
  style->SetProperty(StyleProperties::SPACING_WINDOW_PADDING_Y, 8.0f,
                     "window padding y");
  style->SetProperty(StyleProperties::SPACING_FRAME_PADDING_X, 6.0f,
                     "frame padding x");
  style->SetProperty(StyleProperties::SPACING_FRAME_PADDING_Y, 4.0f,
                     "frame padding y");
  style->SetProperty(StyleProperties::SPACING_ITEM_SPACING_X, 8.0f,
                     "item spacing x");
  style->SetProperty(StyleProperties::SPACING_ITEM_SPACING_Y, 4.0f,
                     "item spacing y");
  style->SetProperty(StyleProperties::SPACING_ITEM_INNER_SPACING_X, 4.0f,
                     "item inner spacing x");
  style->SetProperty(StyleProperties::SPACING_ITEM_INNER_SPACING_Y, 4.0f,
                     "item inner spacing y");

  // ===== 其他尺寸设置 =====
  style->SetProperty(StyleProperties::SIZE_SCROLLBAR_SIZE, 16.0f,
                     "scrollbar size");
  style->SetProperty(StyleProperties::SIZE_GRAB_MIN_SIZE, 10.0f,
                     "grab min size");
  style->SetProperty(StyleProperties::SIZE_WINDOW_MIN_SIZE_X, 32.0f,
                     "window min size x");
  style->SetProperty(StyleProperties::SIZE_WINDOW_MIN_SIZE_Y, 32.0f,
                     "window min size y");

  return style;
}
}  // namespace mite