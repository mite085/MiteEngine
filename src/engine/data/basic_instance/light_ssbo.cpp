#include "light_ssbo.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
LightShaderStorgeBuffer::LightShaderStorgeBuffer(size_t maxLights) : m_MaxLights(maxLights), m_CurrentLightCount(0)
{
  m_SSBOSize = CalculateSSBOSize();
  LOG_INFO("LightSSBO created: maxLights={}, SSBOSize={} bytes", m_MaxLights, m_SSBOSize);
}

void LightShaderStorgeBuffer::Initialize()
{
  if (m_IsInitialized) {
    LOG_WARN("LightSSBO already initialized");
    return;
  }

  try {
    // 创建底层SSBO
    m_SSBO = std::make_unique<ShaderSSBO>(m_SSBOSize,
                                          BindingPointManager::Get().GetLightSSBOBinding(),
                                          GL_DYNAMIC_DRAW);
    m_SSBO->Initialize();

    // 初始化为空光源数据
    ClearLights();

    m_IsInitialized = true;
    LOG_INFO("LightSSBO initialized successfully");
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize LightSSBO: {}", e.what());
    throw;
  }
}

void LightShaderStorgeBuffer::Destroy()
{
  if (!m_IsInitialized) {
    return;
  }

  // 销毁SSBO
  if (m_SSBO) {
    m_SSBO->Destroy();
    m_SSBO.reset();
  }

  m_IsInitialized = false;
  m_CurrentLightCount = 0;
  LOG_INFO("LightSSBO destroyed");
}

bool LightShaderStorgeBuffer::IsInitialized() const
{
  return m_IsInitialized;
}

bool LightShaderStorgeBuffer::UpdateLights(const std::vector<GPULightData> &lights)
{
  if (!m_IsInitialized || !m_SSBO) {
    LOG_ERROR("LightSSBO not initialized");
    return false;
  }

  // 准备头部信息和光源数据
  LightSSBOHeader header;
  auto preparedLights = PrepareLightDataForSSBO(lights, header);
  m_CurrentLightCount = preparedLights.size();

  // 计算数据偏移量
  size_t headerSize = sizeof(LightSSBOHeader);
  size_t lightsSize = sizeof(GPULightData) * preparedLights.size();
  size_t totalSize = headerSize + lightsSize;

  // 验证数据大小
  if (totalSize > m_SSBOSize) {
    LOG_ERROR("Light data exceeds SSBO size: {} > {}", totalSize, m_SSBOSize);
    return false;
  }

  // 更新SSBO数据
  bool success = true;

  // 先更新头部信息
  if (success) {
    success = m_SSBO->UpdateData(&header, headerSize, 0);
  }

  // 再更新光源数据
  if (success && !preparedLights.empty()) {
    success = m_SSBO->UpdateData(preparedLights.data(), lightsSize, headerSize);
  }

  if (success) {
    LOG_TRACE("Updated {} lights to LightSSBO", m_CurrentLightCount);
  }
  else {
    LOG_ERROR("Failed to update lights to LightSSBO");
  }

  return success;
}

bool LightShaderStorgeBuffer::UpdateLight(const GPULightData &light, size_t index)
{
  if (!m_IsInitialized || !m_SSBO) {
    LOG_ERROR("LightSSBO not initialized");
    return false;
  }

  if (!ValidateLightIndex(index)) {
    LOG_ERROR("Invalid light index: {}", index);
    return false;
  }

  // 计算数据偏移（头部大小 + 索引偏移）
  size_t headerSize = sizeof(LightSSBOHeader);
  size_t offset = headerSize + (sizeof(GPULightData) * index);

  bool success = m_SSBO->UpdateData(&light, sizeof(GPULightData), offset);

  if (success) {
    LOG_TRACE("Updated light at index {}", index);

    // 如果更新了新的有效光源，可能需要更新头部
    if (index >= m_CurrentLightCount) {
      m_CurrentLightCount = index + 1;
      LightSSBOHeader header(static_cast<int>(m_CurrentLightCount));
      m_SSBO->UpdateData(&header, sizeof(LightSSBOHeader), 0);
    }
  }
  else {
    LOG_ERROR("Failed to update light at index {}", index);
  }

  return success;
}

bool LightShaderStorgeBuffer::ClearLights()
{
  if (!m_IsInitialized || !m_SSBO) {
    return false;
  }

  // 创建空的头部信息
  LightSSBOHeader header(0);
  bool success = m_SSBO->UpdateData(&header, sizeof(LightSSBOHeader), 0);

  if (success) {
    m_CurrentLightCount = 0;
    LOG_TRACE("Cleared all lights from LightSSBO");
  }
  else {
    LOG_ERROR("Failed to clear lights from LightSSBO");
  }

  return success;
}

void LightShaderStorgeBuffer::Bind() const
{
  if (m_IsInitialized && m_SSBO) {
    m_SSBO->Bind();
  }
}

void LightShaderStorgeBuffer::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader) const
{
  if (m_IsInitialized && m_SSBO && shader) {
    m_SSBO->SetupShaderBinding(shader, ShaderBufferResourceNames::LIGHT_SSBO);
  }
}
size_t LightShaderStorgeBuffer::GetMaxLights() const
{
  return m_MaxLights;
}
size_t LightShaderStorgeBuffer::GetCurrentLightCount() const
{
  return m_CurrentLightCount;
}
size_t LightShaderStorgeBuffer::GetSSBOSize() const
{
  return m_SSBOSize;
}
void LightShaderStorgeBuffer::SetMaxLights(size_t maxLights)
{
  if (m_IsInitialized) {
    LOG_WARN("Cannot change max lights after initialization");
    return;
  }

  m_MaxLights = maxLights;
  m_SSBOSize = CalculateSSBOSize();
  LOG_INFO("LightSSBO max lights set to: {}", m_MaxLights);
}
size_t LightShaderStorgeBuffer::CalculateSSBOSize() const
{
  // 计算总大小：头部 + 最大光源数据
  size_t headerSize = sizeof(LightSSBOHeader);
  size_t lightsSize = sizeof(GPULightData) * m_MaxLights;

  return headerSize + lightsSize;
}

bool LightShaderStorgeBuffer::ValidateLightIndex(size_t index) const
{
  return index < m_MaxLights;
}

std::vector<GPULightData> LightShaderStorgeBuffer::PrepareLightDataForSSBO(
    const std::vector<GPULightData> &lights, LightSSBOHeader &header) const
{
  std::vector<GPULightData> preparedLights;

  // 截断到最大数量
  size_t count = std::min(lights.size(), m_MaxLights);
  preparedLights.reserve(count);

  // 复制有效光源数据
  for (size_t i = 0; i < count; ++i) {
    preparedLights.push_back(lights[i]);
  }

  // 设置头部信息
  header.lightCount = static_cast<int>(count);

  return preparedLights;
}

std::vector<GPULightData> LightShaderStorgeBuffer::CreateEmptyLightData() const
{
  return std::vector<GPULightData>();
}
}  // namespace mite