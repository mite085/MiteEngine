#include "opengl_renderer.h"
#include "asset_manager.h"

namespace mite {
OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer() {}

void OpenGLRenderer::Initialize()
{
  // 初始化OpenGL默认状态
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  defaultFBO_ = 0;  // 默认帧缓冲
}

// ===================== 渲染指令 =====================
void OpenGLRenderer::BeginFrame()
{
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, viewportSize_.x, viewportSize_.y);
}

void OpenGLRenderer::EndFrame()
{
  // 确保所有命令提交
  glFlush();

  // 重置OpenGL状态机
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
  glActiveTexture(GL_TEXTURE0);

  // 注意：不包含交换缓冲区的操作，由窗口系统负责
}

//void OpenGLRenderer::DrawModel(const Model &model, const glm::mat4 &transform)
//{
//  // TODO: 绑定Shader/Uniforms (伪代码)
//  // shader_->SetMat4("u_model", transform);
//
//  // 绘制所有子网格
//  for (size_t i = 0; i < model.GetSubMeshCount(); ++i) {
//    // TODO: 绑定材质（关联的纹理等）
//    // BindMaterial(modelId, i);
//
//    // 绘制子网格
//    model.DrawSubMesh(i);
//  }
//}
void OpenGLRenderer::RenderScene(const std::vector<RenderableEntity> &renderQueue)
{  
  // 1. 遍历渲染队列
  for (const auto &entity : renderQueue) {
    // 2. TODO: 从Asset和Material模块获取网格和材质资源
    //auto mesh = AssetManager::Get().GetMesh(entity.meshID);
    //auto material = MaterialSystem::GetMaterial(entity.materialID);

    //if (!mesh || !material) {
    //  continue;  // 资源加载失败时跳过
    //}

    //// 3. 绑定材质（Shader、Uniform、Texture等）
    //material->Bind();
    //material->SetUniform("u_ModelMatrix", entity.worldTransform);

    //// 4. 绑定网格数据
    //glBindVertexArray(mesh->VAO);
    //glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, nullptr);

    //// 5. 解绑（可选，减少状态切换）
    //glBindVertexArray(0);
  }
}

}  // namespace mite