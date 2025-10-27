#ifndef MITE_PROPERTY_H
#define MITE_PROPERTY_H

#include "ui_core/ui_enum_combobox.h"
#include "ui_core/ui_render.h"

namespace mite {
/**
 * @brief 属性抽象类，整合公共操作
 */
class IPropertyTable {
 public:
  IPropertyTable(std::string tableTranslateLey);
  /**
   * @brief 绘制整个Table
   * @param render
   */
  void RenderTable(UIRender &render);
  /**
   * @brief
   * @param render
   */
  virtual void Render(UIRender &render) = 0;

 protected:
  /**
   * @brief 绘制属性页表格的一行
   * @param labelTranslateKey 名称显示
   * @param propsRenderFunc 属性绘制函数
   */
  void RenderLabelItemRow(UIRender &render,
                          std::string labelTranslateKey,
                          std::function<void()> propsRenderFunc);

  std::string m_TableTranslateKey = "";
  TableProps m_TableProps;
};

/**
 * @brief 属性模板基类 - 基于模板特化的自动映射
 */
template<typename T> class PropertyTable : public IPropertyTable {};
};  // namespace mite

#endif  // MITE_PROPERTY_H
