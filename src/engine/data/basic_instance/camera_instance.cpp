#include "camera_instance.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
CameraInstance::CameraInstance(std::shared_ptr<Camera> camera, const std::string &name)
    : m_Camera(std::move(camera)), m_Name(name)
{
  if (!m_Camera) {
    LOG_ERROR("CameraInstance created with null camera!");
    return;
  }
  else {
    LOG_DEBUG("CameraInstance '{}' allocated", m_Name.empty() ? "unnamed" : m_Name);
  }
}

CameraInstance::~CameraInstance() {}

bool CameraInstance::InitializeUBO()
{
  if (m_CameraUBO->IsInitialized()) {
    LOG_WARN("CameraInstance UBO already initialized for '{}'", m_Name);
    return true;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot initialize UBO: CameraInstance has null camera");
    return false;
  }

  try {
    // 创建UBO对象
    m_CameraUBO = std::make_shared<ShaderUBO>(
        sizeof(CameraUniformBuffer), ShaderBufferResourceType::CameraUBO, m_Name, GL_DYNAMIC_DRAW);
    m_CameraUBO->Initialize();

    LOG_DEBUG("CameraInstance '{}' UBO initialized successfully",
              m_Name);
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize CameraInstance UBO for '{}': {}", m_Name, e.what());
    m_CameraUBO.reset();
    return false;
  }
}

bool CameraInstance::UpdateUBO(const Transform cameraTransform)
{
  if (!m_CameraUBO || !m_CameraUBO->IsInitialized()) {
    LOG_ERROR("Cannot update UBO: CameraInstance '{}' UBO not initialized", m_Name);
    return false;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot update UBO: CameraInstance '{}' has null camera", m_Name);
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
      // LOG_TRACE("CameraInstance '{}' UBO updated successfully", m_Name);
    }
    else {
      LOG_ERROR("Failed to update CameraInstance '{}' UBO data", m_Name);
    }

    return success;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Exception while updating CameraInstance '{}' UBO: {}", m_Name, e.what());
    return false;
  }
}

void CameraInstance::BindUBO() const
{
  if (!m_CameraUBO || !m_CameraUBO->IsInitialized()) {
    LOG_ERROR("Cannot bind UBO: CameraInstance '{}' UBO not initialized", m_Name);
    return;
  }

  // 绑定UBO
  m_CameraUBO->Bind();

  // LOG_TRACE("CameraInstance '{}' UBO", m_Name);
}
}  // namespace mite