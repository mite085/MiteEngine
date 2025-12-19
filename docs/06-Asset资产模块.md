## Asset资产模块
Asset模块是MiteEngine的资源管理核心，负责统一加载、缓存和生命周期管理各类游戏资产。基于事件驱动架构，该模块通过异步加载机制（暂未实现，待后续使用Async事件订阅机制完善）和智能缓存策略，为渲染系统提供高效的资源访问接口，支持模型、材质、纹理等资产的按需加载和内存优化。

### Texture Loader纹理加载器
纹理加载器采用事件驱动架构和统一加载接口设计，负责将外部图像文件和嵌入式纹理数据解析为标准的纹理资产。通过STB图像库和Assimp接口实现多格式支持，与GPU资源创建完全解耦。

**设计模式--事件驱动资源创建，双路径加载策略**：纹理加载器仅负责CPU端数据解析，通过TextureLoadEvent委托Renderer创建GPU资源，实现加载逻辑与渲染API的彻底分离；支持外部文件加载和Assimp嵌入式纹理两种数据源的路径格式，通过统一的内部接口处理像素数据转换。
```mermaid
graph LR
    A[加载请求] --> B{路径类型判断}
    B -->|外部文件| C[外部纹理--STB文件加载]
    B -->|嵌入式标识| D[嵌入式纹理--Assimp数据提取]
    
    C --> G[像素数据解析+元数据构建]
    D --> G
    G --> I[发布加载事件,Renderer创建GPU资源]
````

**缓存集成机制--路径哈希标识**：使用UUID生成器基于纹理路径创建唯一标识，确保相同路径的纹理仅加载一次。缓存查询流程如下：
1. 路径→UUID转换生成搜索ID
2. 缓存查找并验证路径匹配
3. 返回已缓存资产或触发加载

### Material Loader材质加载器

材质加载器采用GLTF PBR标准解析和纹理依赖管理设计，专门处理基于物理渲染的材质导入。通过Assimp接口提取PBR参数和纹理引用，构建完整的材质资产依赖图。
```mermaid
flowchart LR
    A[GLTF场景] --> B[遍历材质]
    B --> C[提取PBR参数]
    B --> D[解析纹理引用]
    C --> E[构建材质资产]
    D --> E
    E --> F[发布加载事件]
````

**PBR参数提取**：采用分层回退策略，从GLTF专用参数回退到传统材质参数

| 参数 | 优先级 | 默认值 |
|------|--------|--------|
| 基础颜色 | GLTF PBR → 漫反射 → 灰色 | (0.8,0.8,0.8,1.0) |
| 金属度 | GLTF因子 → 0.0 | 0.0 |
| 粗糙度 | GLTF因子 → 1.0 | 1.0 |

**纹理依赖管理**：支持5种核心纹理类型

| 纹理类型 | 用途 |
|----------|------|
| 基础颜色 | 反照率 |
| 金属粗糙度 | PBR参数 |
| 法线 | 表面细节 |
| 自发光 | 发光效果 |
| 环境光遮蔽 | 阴影细节 |

### Model Loader模型加载器

模型加载器采用多格式适配和LOD链式管理设计，通过Assimp库解析3D模型并构建完整的资产依赖图，支持自动网格优化和层次细节生成。
```mermaid
flowchart TD
    A[模型文件] --> B{格式判断}
    B -->|GLTF| C[GLTF特化配置]
    B -->|OBJ| D[OBJ特化配置]
    B -->|其他| E[通用配置]
    
    C --> F[Assimp解析]
    D --> F
    E --> F
    
    F --> G[材质加载]
    F --> H[网格处理]
    G --> I[资产构建]
    H --> I
    I --> J[事件发布]
````

**格式特化配置**（主要使用GLTF作为引擎标准格式）
| 格式 | 特化配置 | 优化标志 |
|------|----------|----------|
| GLTF | 缓存优化 | `aiProcess_ImproveCacheLocality` |
| OBJ | 网格合并 | `aiProcess_OptimizeMeshes` |
| FBX | 骨骼限制 | `aiProcess_LimitBoneWeights` |

**通用处理管线**
- 三角化：`aiProcess_Triangulate`
- 法线生成：`aiProcess_GenNormals`
- 切线计算：`aiProcess_CalcTangentSpace`
- 顶点合并：`aiProcess_JoinIdenticalVertices`

**LOD链式管理**：支持多级细节自动生成

```mermaid
flowchart LR
    A[原始网格] --> B[顶点缓存优化]
    B --> C[网格简化]
    C --> D[顶点重映射]
    D --> E[LOD级别n]
    
    A --> F[基础LOD级别0]
    E --> G[LOD链构建]
    F --> G
````

**网格简化算法**：基于meshoptimizer的高效简化
1. 顶点缓存优化：`meshopt_optimizeVertexCache`
2. 粗略简化：`meshopt_simplifySloppy`
3. 顶点重映射：`meshopt_optimizeVertexFetchRemap`
4. 数据重组：`meshopt_remapVertexBuffer` + `meshopt_remapIndexBuffer`

注意： 常规方法是使用meshopt_simplify进行粗略简化，利用target_error限制误差。但实际操作过程中，发现无论如何调整target_error，均无法确保粗略简化可以精简模型顶点数，所以使用simplifySloppy以确保顶点数量减少

**数据合并算法**：将多子网格合并为单一渲染批次

```cpp
// 索引偏移修正公式
adjustedIndex = originalIndex + vertexOffset
// 顶点数据合并
mergedData = Merge(subMesh[i].vertexData)
````
合并优势：
- 减少Draw Call数量
- 提升缓存一致性
- 简化资源管理

**包围盒计算**：基于所有子网格的极值点计算模型级包围盒：

$$BBox_{min} = \min(subMesh[i].bbox_{min})$$
$$BBox_{max} = \max(subMesh[i].bbox_{max})$$

**事件驱动架构**：模型加载事件流

1. 材质依赖解析完成
2. 网格数据处理完毕
3. LOD链构建完成
4. 生成ModelSourceData
5. 发布ModelLoadEvent
6. Renderer接收并创建GPU资源

模型加载器通过格式特化配置和智能的LOD管理，为引擎提供高性能的3D模型导入能力。

### Asset Cache资产缓存

资产缓存采用引用计数和LRU淘汰策略的双重管理机制，为引擎提供线程安全的资源生命周期管理，支持模型、材质、纹理等各类资产的统一缓存。

**设计架构**：LRU淘汰策略
```mermaid
flowchart TD
    A[资产请求] --> B{缓存查找}
    B -->|命中| C[更新LRU位置]
    B -->|未命中| D[加载新资产]
    
    C --> E[返回资产指针]
    D --> F[存储到缓存]
    F --> E
    
    G[引用管理] --> H[引用计数增减]
    H --> I{计数归零}
    I -->|是| J[LRU标记待清理]
    I -->|否| K[保持活跃]
````

**引用计数管理**：基于RAII模式的智能引用跟踪
| 操作 | 引用计数变化 | 触发条件 |
|------|-------------|----------|
| `Store` | 初始为0 | 新资产入库 |
| `Get` | 无变化 | 只读访问 |
| `AddRefCount` | +1 | 显式增加引用 |
| `Release` | -1 | 显式释放引用 |

**LRU淘汰策略**：基于访问频率的智能缓存管理
- 最近访问：移动到链表头部
- 淘汰候选：链表尾部元素
- 淘汰条件：引用计数为0 + 超过缓存限制

**性能特性**
- 查找复杂度：O(1) - 哈希表
- LRU更新：O(1) - 链表操作
- 线程安全：细粒度锁保护

**调用模式**
1. 加载器调用Store()存入新资产
2. 使用者调用Get()获取资产指针
3. 场景引用调用AddRefCount()增加引用
4. 场景卸载调用Release()减少引用
5. 系统定期调用PurgeUnused()清理

### Asset Manager资产管理器
资产管理器作为Asset模块的统一对外接口，采用单例模式和统一生命周期管理设计，为引擎提供简洁高效的资源加载和引用管理服务。
```mermaid
flowchart LR
    B[资产管理器]
    B --> C{资源类型}
    C -->|纹理| D[纹理加载器]
    C -->|模型| E[模型加载器]
    C -->|材质| F[材质加载器]
    
    D --> G[纹理缓存]
    E --> H[模型缓存]
    F --> I[材质缓存]
    
    G --> J[返回资产ID]
    H --> J
    I --> J
````

**调用模式**
```cpp
// 1. 加载资源
auto textureID = AssetManager::Get().LoadTexture("texture.png");
auto modelID = AssetManager::Get().LoadGLTFModel("scene.gltf");
// 2. 使用资源  
auto texture = AssetManager::Get().GetTexture(textureID);
auto model = AssetManager::Get().GetModel(modelID);
// 3. 释放资源
AssetManager::Get().ReleaseTexture(textureID);
AssetManager::Get().ReleaseModel(modelID);
````

Asset模块通过统一的资源管理架构和智能缓存机制，为引擎提供了高效可靠的资产加载管线。其事件驱动的异步加载设计（暂未实现，待后续使用Async事件订阅机制完善）和引用计数生命周期管理，确保了大规模资产场景下的性能和内存效率。
