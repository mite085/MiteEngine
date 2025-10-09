#include "mesh_instance.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {

MeshInstance::MeshInstance(std::shared_ptr<Mesh> mesh)
    : m_Mesh(std::move(mesh))
{
  if (!m_Mesh) {
    LOG_ERROR("MeshInstance created with null mesh!");
    return;
  }

  LOG_DEBUG("MeshInstance allocated");
}

MeshInstance::~MeshInstance() {}

bool MeshInstance::InitializeUBO()
{
  if (m_ModelUBO && m_ModelUBO->IsInitialized()) {
    LOG_WARN("MeshInstance UBO already initialized");
    return true;
  }

  if (!m_Mesh) {
    LOG_ERROR("Cannot initialize UBO: MeshInstance has null mesh");
    return false;
  }

  try {
    // 创建模型UBO对象
    m_ModelUBO = std::make_shared<ShaderUBO>(
        sizeof(ModelUniformBuffer), BindingPointManager::Get().GetModelUBOBinding(), GL_DYNAMIC_DRAW);
    m_ModelUBO->Initialize();

    LOG_DEBUG("MeshInstance Model UBO initialized successfully");
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize MeshInstance UBO : {}", e.what());
    m_ModelUBO.reset();
    return false;
  }
}
void MeshInstance::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader)
{
  // 设置着色器绑定
  m_ModelUBO->SetupShaderBinding(shader, ShaderBufferResourceNames::MODEL_UBO);
}

void MeshInstance::UpdateUBO(const Transform &worldTransform)
{
  if (!m_ModelUBO || !m_ModelUBO->IsInitialized()) {
    LOG_ERROR("Cannot update UBO: MeshInstance UBO not initialized");
    return;
  }

  if (!m_Mesh) {
    LOG_ERROR("Cannot update UBO: MeshInstance has null mesh");
    return;
  }

  try {
    // 构建模型UBO数据
    ModelUniformBuffer uboData;
    uboData.model = worldTransform.GetLocalMatrix(); // world transform的local matrix即为world matrix
    uboData.normalMatrix = glm::transpose(glm::inverse(glm::mat3(uboData.model)));

    // 更新UBO数据
    bool success = m_ModelUBO->UpdateData(&uboData, sizeof(ModelUniformBuffer));

    // 更新缓存
    m_WorldTransform = worldTransform;

    if (success) {
      // LOG_TRACE("MeshInstance '{}' Model UBO updated successfully", m_Name);
    }
    else {
      LOG_ERROR("Failed to update MeshInstance Model UBO data");
    }
  }
  catch (const std::exception &e) {
    LOG_ERROR("Exception while updating MeshInstance UBO: {}", e.what());
  }
}

void MeshInstance::BindUBO() const
{
  if (!m_ModelUBO || !m_ModelUBO->IsInitialized()) {
    LOG_ERROR("Cannot bind UBO: MeshInstance UBO not initialized");
    return;
  }

  // 绑定模型UBO到绑定点2
  m_ModelUBO->Bind();
}

uint32_t MeshInstance::GetMaterialIndex() const
{
  if (!m_Mesh) {
    LOG_ERROR("Cannot get material index: MeshInstance has null mesh");
    return 0;
  }
  return m_Mesh->GetMaterialIndex();
}

std::pair<glm::vec3, glm::vec3> MeshInstance::GetWorldBoundingBox() const
{
  if (!m_Mesh) {
    return {glm::vec3(0.0f), glm::vec3(0.0f)};
  }

  auto localBBox = m_Mesh->GetBoundingBox();
  glm::vec3 worldMin = m_WorldTransform.GetLocalMatrix() * glm::vec4(localBBox.first, 1.0f);
  glm::vec3 worldMax = m_WorldTransform.GetLocalMatrix() * glm::vec4(localBBox.second, 1.0f);

  return {worldMin, worldMax};
}

}  // namespace mite
