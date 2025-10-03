#include "camera_instance.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
constexpr const char *CameraInstance::UBO_BLOCK_NAME;
CameraInstance::CameraInstance(std::shared_ptr<Camera> camera, const std::string &name)
    : m_Camera(std::move(camera)), m_Name(name)
{
  // 分配UBO绑定点
  m_BindingPoint = BindingPointManager::Get().GetCameraUBOBinding();

  if (!m_Camera) {
    LOG_ERROR("CameraInstance created with null camera!");
    return;
  }

  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Failed to allocate binding point for CameraInstance");
  }
  else {
    LOG_DEBUG("CameraInstance '{}' allocated binding point: {}",
              m_Name.empty() ? "unnamed" : m_Name,
              m_BindingPoint);
  }
}

CameraInstance::~CameraInstance()
{
  if (m_BindingPoint != UINT32_MAX) {
    // 注意：绑定点释放由BindingPointManager统一管理
    // 这里只需要清理UBO资源
    LOG_DEBUG("CameraInstance '{}' destroyed", m_Name);
  }
}

bool CameraInstance::InitializeUBO(std::shared_ptr<OpenGLShader> shader)
{
  if (m_UBOInitialized) {
    LOG_WARN("CameraInstance UBO already initialized for '{}'", m_Name);
    return true;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot initialize UBO: CameraInstance has null camera");
    return false;
  }

  if (!shader) {
    LOG_ERROR("Cannot initialize UBO: null shader provided");
    return false;
  }

  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Cannot initialize UBO: invalid binding point");
    return false;
  }

  try {
    // 创建UBO对象
    m_CameraUBO = std::make_shared<ShaderUBO>(sizeof(CameraUniformBuffer), GL_DYNAMIC_DRAW);
    m_CameraUBO->Initialize();

    // 设置着色器绑定
    m_CameraUBO->SetupShaderBinding(shader, UBO_BLOCK_NAME, m_BindingPoint);

    m_UBOInitialized = true;

    LOG_DEBUG("CameraInstance '{}' UBO initialized successfully with binding point: {}",
              m_Name,
              m_BindingPoint);
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize CameraInstance UBO for '{}': {}", m_Name, e.what());
    m_CameraUBO.reset();
    return false;
  }
}

bool CameraInstance::UpdateUBO(const glm::mat4 &viewMatrix)
{
  if (!m_UBOInitialized || !m_CameraUBO) {
    LOG_ERROR("Cannot update UBO: CameraInstance '{}' UBO not initialized", m_Name);
    return false;
  }

  if (!m_Camera) {
    LOG_ERROR("Cannot update UBO: CameraInstance '{}' has null camera", m_Name);
    return false;
  }

  try {
    // 获取最新的UBO数据
    CameraUniformBuffer uboData = m_Camera->FillUBOData(viewMatrix);

    // 更新UBO数据
    bool success = m_CameraUBO->UpdateData(&uboData, sizeof(CameraUniformBuffer));

    if (success) {
      LOG_TRACE("CameraInstance '{}' UBO updated successfully", m_Name);
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
  if (!m_UBOInitialized || !m_CameraUBO) {
    LOG_ERROR("Cannot bind UBO: CameraInstance '{}' UBO not initialized", m_Name);
    return;
  }

  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Cannot bind UBO: CameraInstance '{}' has invalid binding point", m_Name);
    return;
  }

  // 绑定UBO到指定的绑定点
  m_CameraUBO->Bind(m_BindingPoint);

  LOG_TRACE("CameraInstance '{}' UBO bound to point: {}", m_Name, m_BindingPoint);
}

}  // namespace mite
