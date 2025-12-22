#include "ui_property.h"

namespace mite {
IPropertyTable::IPropertyTable(std::string tableTranslateLey)
    : m_TableTranslateKey(tableTranslateLey) {}
void IPropertyTable::RenderTable(UIRender &render) {
  // 绘制分隔符
  render.RenderLabelSprator(m_TableTranslateKey);

  // 填充表格属性
  m_TableProps.columns = 2;          // 2列显示
  m_TableProps.showHeaders = false;  // 不显示表头
  m_TableProps.resizable = false;    // 不允许resize
  m_TableProps.borders = false;      // 不显示边框
  m_TableProps.rowBg = false;        // 无装饰
  m_TableProps.bordersInnerH = false;
  m_TableProps.bordersInnerV = false;
  m_TableProps.bordersOuterH = false;
  m_TableProps.bordersOuterV = false;

  // 绘制表格
  render.RenderTable(m_TableProps, [&render, this]() { Render(render); });
}

void IPropertyTable::RenderLabelItemRow(UIRender &render,
                                        std::string labelTranslateKey,
                                        std::function<void()> propsRenderFunc) {
  // 新的一行绘制文本
  render.TableNextRow();
  render.RenderLabel(labelTranslateKey);

  // 同一行下一列绘制属性
  render.TableNextColume();
  propsRenderFunc();
}
}  // namespace mite