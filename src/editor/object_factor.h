// 仅编辑器需要“创建对象”
// 对Runtime而言，Entity和Component级别的抽象就已经足够了
class EditorObjectFactory {
 public:
  static Entity CreateCamera(SceneRegistry& registry) {
    Entity entity = registry.CreateEntity();
    registry.AddComponent<TransformComponent>(entity);
    registry.AddComponent<CameraComponent>(entity);
    registry.AddComponent<IDComponent>(entity);
    registry.AddComponent<TagComponent>(entity, "Camera");
    return entity;
  }

  static Entity CreateModel(SceneRegistry& registry, MeshHandle mesh) {
    Entity entity = registry.CreateEntity();
    registry.AddComponent<TransformComponent>(entity);
    registry.AddComponent<MeshComponent>(entity, mesh);
    registry.AddComponent<IDComponent>(entity);
    registry.AddComponent<TagComponent>(entity, "Model");
    return entity;
  }
};
