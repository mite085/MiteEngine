#ifndef MITE_CAMERA_TYPES_H
#define MITE_CAMERA_TYPES_H

#include <cstdint>

namespace mite {
/**
 * @brief 可见性掩码定义
 * 用于分层渲染和选择性可见性控制
 */
namespace VisibilityMask {
// 基础可见性层级
constexpr uint32_t DEFAULT = 0x00000001;      // 默认可见层
constexpr uint32_t STATIC = 0x00000002;       // 静态物体层
constexpr uint32_t DYNAMIC = 0x00000004;      // 动态物体层
constexpr uint32_t TRANSPARENT = 0x00000008;  // 透明物体层

// 特殊用途层级（待后续启用）
// constexpr uint32_t UI = 0x00000010;       // UI元素层
// constexpr uint32_t DEBUG = 0x00000020;    // 调试信息层
// constexpr uint32_t EDITOR = 0x00000040;   // 编辑器专用层
// constexpr uint32_t TERRAIN = 0x00000080;  // 地形层

// 渲染通道专用（待后续启用）
// constexpr uint32_t SHADOW_CAST = 0x00000100;  // 投射阴影
// constexpr uint32_t REFLECTION = 0x00000200;   // 反射渲染
// constexpr uint32_t REFRACTION = 0x00000400;   // 折射渲染

// 游戏逻辑层级（待后续启用）
// constexpr uint32_t PLAYER = 0x00001000;       // 玩家相关
// constexpr uint32_t ENEMY = 0x00002000;        // 敌人相关
// constexpr uint32_t NPC = 0x00004000;          // NPC相关
// constexpr uint32_t ENVIRONMENT = 0x00008000;  // 环境物体

// 组合掩码
constexpr uint32_t ALL = 0xFFFFFFFF;   // 所有层级
constexpr uint32_t NONE = 0x00000000;  // 无层级

// 常用组合（待后续启用）
// constexpr uint32_t RENDER_ALL = DEFAULT | STATIC | DYNAMIC | TRANSPARENT;
// constexpr uint32_t EDITOR_VIEW = EDITOR | DEBUG | UI;
}  // namespace VisibilityMask

/**
 * @brief 可见性
 * @param m_IsVisible 是否可见，用于加速结构构建之前的剔除
 * @param m_VisibilityMask 可见性掩码，用于加速结构查询之后，视锥体剔除
 */
struct Visibility {
  bool m_IsVisible = true;                          // 当前可见性状态
  uint32_t m_VisibilityMask = VisibilityMask::ALL;  // 可见性掩码
};
}  // namespace mite

#endif  // MITE_CAMERA_TYPES_H
