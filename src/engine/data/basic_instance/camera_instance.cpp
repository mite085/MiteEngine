#include "camera_instance.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
CameraInstance::CameraInstance(std::shared_ptr<Camera> camera)
    : m_Camera(std::move(camera))
{
  if (!m_Camera) {
    LOG_ERROR("CameraInstance created with null camera!");
    return;
  }
  else {
    LOG_DEBUG("CameraInstance allocated");
  }
}

CameraInstance::~CameraInstance() {}

bool CameraInstance::InitializeUBO()
{
  if (m_CameraUBO->IsInitialized()) {
    LOG_WARN("CameraInstance UBO already initialized'");
    return true;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot initialize UBO: CameraInstance has null camera");
    return false;
  }

  try {
    // 创建UBO对象
    m_CameraUBO = std::make_shared<ShaderUBO>(
        sizeof(CameraUniformBuffer), ShaderBufferResourceType::CameraUBO, GL_DYNAMIC_DRAW);
    m_CameraUBO->Initialize();

    LOG_DEBUG("CameraInstance UBO initialized successfully");
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize CameraInstance UBO: {}", e.what());
    m_CameraUBO.reset();
    return false;
  }
}

void CameraInstance::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader)
{
  // 设置着色器绑定
  m_CameraUBO->SetupShaderBinding(shader, ShaderBufferResourceNames::CAMERA_UBO);
}

bool CameraInstance::UpdateUBO(const Transform cameraTransform)
{
  if (!m_CameraUBO || !m_CameraUBO->IsInitialized()) {
    LOG_ERROR("Cannot update UBO: CameraInstance UBO not initialized");
    return false;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot update UBO: CameraInstance has null camera");
    return false;
  }

  try {
    // 获取最新的UBO数据
    CameraUniformBuffer uboData = m_Camera->FillUBOData(cameraTransform.GetViewMatrix());

    // 更新UBO数据
    bool success = m_CameraUBO->UpdateData(&uboData, sizeof(CameraUniformBuffer));

    // 更新缓存
    m_CameraTransform = cameraTransform;

    if (success) {
      // LOG_TRACE("CameraInstance UBO updated successfully");
    }
    else {
      LOG_ERROR("Failed to update CameraInstance UBO data");
    }

    return success;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Exception while updating CameraInstance UBO: {}", e.what());
    return false;
  }
}

void CameraInstance::BindUBO() const
{
  if (!m_CameraUBO || !m_CameraUBO->IsInitialized()) {
    LOG_ERROR("Cannot bind UBO: CameraInstance UBO not initialized");
    return;
  }

  // 绑定UBO
  m_CameraUBO->Bind();

  // LOG_TRACE("CameraInstance UBO");
}
}  // namespace mite