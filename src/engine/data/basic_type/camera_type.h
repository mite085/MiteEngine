// camera_types.h
#ifndef MITE_CAMERA_TYPES_H
#define MITE_CAMERA_TYPES_H

#include <cstdint>

namespace mite {
/**
 * @brief 相机可见性掩码定义
 * 用于分层渲染和选择性可见性控制
 */
namespace CameraVisibilityMask {
// 基础可见性层级
constexpr uint32_t DEFAULT = 0x00000001;      // 默认可见层
constexpr uint32_t STATIC = 0x00000002;       // 静态物体层
constexpr uint32_t DYNAMIC = 0x00000004;      // 动态物体层
constexpr uint32_t TRANSPARENT = 0x00000008;  // 透明物体层

// 特殊用途层级
constexpr uint32_t UI = 0x00000010;       // UI元素层
constexpr uint32_t DEBUG = 0x00000020;    // 调试信息层
constexpr uint32_t EDITOR = 0x00000040;   // 编辑器专用层
constexpr uint32_t TERRAIN = 0x00000080;  // 地形层

// 渲染通道专用
constexpr uint32_t SHADOW_CAST = 0x00000100;  // 投射阴影
constexpr uint32_t REFLECTION = 0x00000200;   // 反射渲染
constexpr uint32_t REFRACTION = 0x00000400;   // 折射渲染

// 游戏逻辑层级（待后续启用）
//constexpr uint32_t PLAYER = 0x00001000;       // 玩家相关
//constexpr uint32_t ENEMY = 0x00002000;        // 敌人相关
//constexpr uint32_t NPC = 0x00004000;          // NPC相关
//constexpr uint32_t ENVIRONMENT = 0x00008000;  // 环境物体

// 组合掩码
constexpr uint32_t ALL = 0xFFFFFFFF;   // 所有层级
constexpr uint32_t NONE = 0x00000000;  // 无层级

// 常用组合
constexpr uint32_t RENDER_ALL = DEFAULT | STATIC | DYNAMIC | TRANSPARENT;
constexpr uint32_t EDITOR_VIEW = EDITOR | DEBUG | UI;
}  // namespace CameraVisibilityMask

/**
 * @brief 相机类型枚举
 */
enum class CameraType {
  PERSPECTIVE,   // 透视相机
  ORTHOGRAPHIC,  // 正交相机
};

/**
 * @brief 相机清除标志
 */
enum class CameraClearFlags {
  SKYBOX,       // 清除为天空盒
  SOLID_COLOR,  // 清除为纯色
  DEPTH_ONLY,   // 只清除深度
  DONT_CLEAR    // 不清除
};
}  // namespace mite

#endif  // MITE_CAMERA_TYPES_H
