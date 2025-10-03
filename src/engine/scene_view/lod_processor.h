// 将OpenGLDevice中基于屏幕空间的LOD部分提取出来
// 在这里执行LOD决策。

// 在开发实例化渲染时，RenderableItem仅存储同一LOD级别的Mesh指针
// RenderQueue按照LOD级别对同一Mesh分组，分别执行不同的DrawCall
