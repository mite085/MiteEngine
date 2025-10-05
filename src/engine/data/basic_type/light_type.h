#ifndef MITE_LIGHT_TYPES_H
#define MITE_LIGHT_TYPES_H

#include "basic_data/transform.h"
#include "headers/headers.h"

namespace mite {
// ----------------- 光源类型和基础参数 -------------------
/**
 * @brief 光源类型枚举
 * @note 支持点光源、聚光灯、方向光、面光源
 */
enum class LightType {
  POINT = 0,        // 点光源
  SPOT = 1,         // 聚光灯
  DIRECTIONAL = 2,  // 方向光
  AREA_RECT = 3,    // 矩形面光源
  AREA_ELLIPSE = 4  // 椭圆面光源
};
/**
 * @brief 光源衰减模式
 */
enum class LightAttenuation {
  LINEAR = 0,     // 线性衰减
  QUADRATIC = 1,  // 二次衰减
  PHYSICAL = 2    // 物理正确衰减
};
/**
 * @brief 面光源形状
 */
enum class AreaLightShape {
  RECTANGLE = 0,  // 矩形
  ELLIPSE = 1     // 椭圆
};

// ----------------- 光源属性 -------------------
/**
 * @brief 整合后的光源属性
 */
struct LightProperties {
  // 基础属性
  glm::vec3 color = glm::vec3(1.0f);  // 光源颜色
  float intensity = 1.0f;             // 光源强度
  bool enabled = true;                // 是否启用

  // 类型特定属性 - 按需使用
  union {
    // 点光源
    struct {
      float radius = 10.0f;                                       // 影响半径
      float falloff = 1.0f;                                       // 衰减系数
      LightAttenuation attenuation = LightAttenuation::PHYSICAL;  // 衰减模式
    } point;

    // 聚光灯
    struct {
      float innerAngle = 30.0f;  // 内角（度）
      float outerAngle = 45.0f;  // 外角（度）
      float blend = 0.5f;        // 边缘柔化(0-1)
      float range = 10.0f;       // 照射范围
    } spot;

    // 方向光
    struct {
      float irradiance = 1.0f;  // 辐照度(W/m2)
    } directional;

    // 面光源
    struct {
      glm::vec2 size = glm::vec2(1.0f, 1.0f);            // 尺寸(x,y)
      AreaLightShape shape = AreaLightShape::RECTANGLE;  // 形状
      float power = 100.0f;                              // 总功率(W)
    } area;
  } specific;

  // 默认构造函数
  LightProperties() : specific{} {}
};

// ----------------- 光源数据结构 -------------------
/**
 * @brief 完整的GPU光源数据（用于SSBO）
 * @note 将Property转换为GPU可接受的Data
 */
struct alignas(16) GPULightData {
  // 基础属性 - 16字节对齐 (每组vec3+float为16字节)
  glm::vec3 color;
  float intensity;
  glm::vec3 position;  // 世界坐标（从变换组件获取WorldPosition）
  float type;          // LightType转换为float
  glm::vec3 direction;  // 方向（从变换组件获取WorldFront，面光源以WorldUp法线为方向）
  float padding1;  // 填充以确保16字节对齐

  // 类型特定属性 - 使用union节省空间
  union {
    // 点光源和聚光灯共享属性
    struct {
      float range;        // 范围/半径
      float innerAngle;   // 内角（聚光灯，度）
      float outerAngle;   // 外角（聚光灯，度）
      float blend;        // 边缘柔化（聚光灯）
      float falloff;      // 衰减系数（点光源）
      float padding2[3];  // 填充以确保union大小为16字节倍数
    } pointSpot;

    // 方向光
    struct {
      float irradiance;   // 辐照度
      float padding3[3];  // 填充以确保union大小为16字节倍数
    } directional;

    // 面光源
    struct {
      glm::vec2 size;  // 尺寸
      float power;     // 功率（W）
      float shape;     // AreaLightShape转换为float
    } area;
  } specific;

  /**
   * @brief 从LightProperties构造GPULightData
   * @param props 光源属性
   * @param lightType 光源类型
   */
  GPULightData(const LightProperties &props, const Transform &worldTransform, LightType lightType)
      : color(props.color),
        intensity(props.intensity),
        type(static_cast<float>(lightType)),
        padding1(0.0f),
        specific{}  // 使用值初始化确保union被清零
  {
    // 从世界变换提取位置和方向
    ExtractTransformData(worldTransform, lightType);

    // 根据光源类型填充特定属性
    FillTypeSpecificData(props, lightType);

    // 初始化填充字段
    InitializePadding();
  }

  // 删除默认构造函数，强制从LightProperties构造
  GPULightData() = delete;

 private:
  void ExtractTransformData(const Transform &worldTransform, LightType lightType)
  {
    // 提取位置
    position = worldTransform.GetPosition();

    // 根据光源类型提取方向/法线
    switch (lightType) {
      case LightType::SPOT:
      case LightType::POINT:
        // 聚光灯/点光源方向：变换矩阵的-Z轴（点光源方向本身无意义）
        direction = worldTransform.GetForward();
        break;

      case LightType::DIRECTIONAL:
        // 方向光方向：变换矩阵的-Z轴
        direction = worldTransform.GetForward();
        break;

      case LightType::AREA_RECT:
      case LightType::AREA_ELLIPSE:
        // 面光源法线：变换矩阵的+Y轴
        direction = worldTransform.GetUp();
        break;
    }
  }
  void FillTypeSpecificData(const LightProperties &props, LightType lightType)
  {
    // 根据光源类型填充特定属性
    switch (lightType) {
      case LightType::POINT:
        specific.pointSpot.range = props.specific.point.radius;
        specific.pointSpot.falloff = props.specific.point.falloff;
        specific.pointSpot.innerAngle = 0.0f;  // 点光源无角度
        specific.pointSpot.outerAngle = 0.0f;
        specific.pointSpot.blend = 0.0f;  // 点光源无聚光灯的边缘柔滑
        break;

      case LightType::SPOT:
        specific.pointSpot.range = props.specific.spot.range;
        specific.pointSpot.falloff = 1.0f;  // 聚光灯使用角度衰减，不使用衰减系数
        specific.pointSpot.innerAngle = props.specific.spot.innerAngle;
        specific.pointSpot.outerAngle = props.specific.spot.outerAngle;
        specific.pointSpot.blend = props.specific.spot.blend;
        break;

      case LightType::DIRECTIONAL:
        specific.directional.irradiance = props.specific.directional.irradiance;
        break;

      case LightType::AREA_RECT:
      case LightType::AREA_ELLIPSE:
        specific.area.power = props.specific.area.power;
        specific.area.size = props.specific.area.size;
        specific.area.shape = static_cast<float>(props.specific.area.shape);
        break;
    }
  }
  void InitializePadding()
  {
    // 初始化所有填充字段为0
    specific.pointSpot.padding2[0] = 0.0f;
    specific.pointSpot.padding2[1] = 0.0f;
    specific.pointSpot.padding2[2] = 0.0f;
    specific.directional.padding3[0] = 0.0f;
    specific.directional.padding3[1] = 0.0f;
    specific.directional.padding3[2] = 0.0f;
  }
};
/**
 * @brief LightSSBO头部信息
 * @note 只需要光源数量，最大数量在CPU端管理即可
 */
struct alignas(16) LightSSBOHeader {
  int lightCount;    // 有效光源数量
  float padding[3];  // 填充以确保16字节对齐

  LightSSBOHeader(int count = 0) : lightCount(count), padding{0.0f, 0.0f, 0.0f} {}
};
}  // namespace mite

#endif  // MITE_LIGHT_TYPES_H
