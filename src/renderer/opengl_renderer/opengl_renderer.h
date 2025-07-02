#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "renderer.h"
#include "glad.h"
#include "glfw/glfw3.h"// 必须在GLAD加载库之后
#include "opengl_renderer/opegl_device.h"

#include "texture.h"
#include "model.h"

namespace mite {

	/**
 * OpenGL渲染器实现
 * 职责：
 * 1. 实现基类定义的渲染接口
 * 2. 管理OpenGL专属状态（如VAO、Shader Program）
 */
class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer(std::shared_ptr<AssetManager> assetManager);
  ~OpenGLRenderer() override;
  void Initialize() override;
  // ---- 资源管理 ----
  void LoadTexture(AssetID id) override;
  void UnloadTexture(AssetID id) override;
  void LoadModel(AssetID id) override;
  void UnloadModel(AssetID id) override;

  // ---- 渲染指令 ----
  void BeginFrame() override;
  void EndFrame() override;
  void DrawModel(AssetID modelId, const glm::mat4 &transform) override;

 private:
  // ---- 内部资源存储 ----
  std::unordered_map<AssetID, std::unique_ptr<Texture>> textures_;
  std::unordered_map<AssetID, std::unique_ptr<Model>> models_;

  // ---- OpenGL专属状态 ----
  GLuint defaultFBO_ = 0;
  GLuint currentShader_ = 0;
};


}  // namespace mite

#endif
