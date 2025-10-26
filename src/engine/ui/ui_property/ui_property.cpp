#include "ui_property.h"

namespace mite {
IPropertyTable::IPropertyTable(std::string tableTranslateLey)
    : m_TableTranslateKey(tableTranslateLey)
{
}
void IPropertyTable::RenderTable(UIRender &render)
{
  // 绘制分隔符
  render.RenderLabelSprator(m_TableTranslateKey);

  // 填充表格属性
  TableProps tableProps;
  tableProps.columns = 2;          // 2列显示
  tableProps.showHeaders = false;  // 不显示表头
  tableProps.resizable = false;    // 不允许resize
  tableProps.borders = false;      // 不显示边框
  tableProps.rowBg = false;        // 无装饰
  tableProps.bordersInnerH = false;
  tableProps.bordersInnerV = false;
  tableProps.bordersOuterH = false;
  tableProps.bordersOuterV = false;

  // 绘制表格
  render.RenderTable(tableProps, [&render, this]() { Render(render); });
}

void IPropertyTable::RenderLabelItemRow(UIRender &render,
                                        std::string labelTranslateKey,
                                        std::function<void()> propsRenderFunc)
{
  // 新的一行绘制文本
  render.TableNextRow();
  render.RenderLabel(labelTranslateKey);

  // 同一行下一列绘制属性
  render.TableNextColume();
  propsRenderFunc();
}


}  // namespace mite