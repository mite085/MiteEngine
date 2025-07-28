#ifndef MITE_UI_PANEL
#define MITE_UI_PANEL

#include "headers/headers.h"

namespace mite {
/**
 * @brief UI面板抽象基类，所有具体功能面板需继承此类
 * @note 采用CRTP模式实现静态多态，避免虚函数开销
 */
template <typename DerivedPanel>
class UIPanel {
public:
    explicit UIPanel(const std::string& name);
    virtual ~UIPanel() = default;

    //=== 核心接口 ===//
    void Draw();  // 主绘制入口
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    const std::string& GetName() const { return m_name; }

    //=== 生命周期钩子 ===//
    virtual void OnAttach() {}    // 面板首次注册时调用
    virtual void OnDetach() {}    // 面板注销时调用
    virtual void OnUpdate(float deltaTime) {}  // 每帧更新逻辑

protected:
    //=== 子类需实现的接口 ===//
    virtual void DrawContent() = 0;  // 实际面板内容绘制

    //=== 工具方法 ===//
    void BeginWindowStyle();  // 应用预设窗口样式
    void EndWindowStyle();    // 恢复样式

    std::string m_name;      // 面板唯一标识名
    bool m_visible = true;   // 是否显示面板
    bool m_firstDraw = true; // 首次绘制标记

    // 样式控制（可在子类修改）
    ImGuiWindowFlags m_windowFlags = ImGuiWindowFlags_None;
    ImVec2 m_defaultSize = ImVec2(300, 400);
};

};

#endif
