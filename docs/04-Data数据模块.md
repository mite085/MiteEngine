## Data数据模块

Data数据模块是MiteEngine的数据核心层，基于GLM数学库开发，负责管理所有渲染相关的数据类型、GPU资源设施。作为引擎的第二层基础模块，它为上层渲染系统提供统一的数据抽象和资源管理。

### Transform变换
Transform系统是MiteEngine的数学基础组件，提供统一的3D空间变换管理。作为纯数学工具类，为所有需要空间变换的组件提供底层支持。

**设计理念**
- 右手坐标系：遵循OpenGL标准，Y轴向上，Z轴向前
- 双旋转表示：内部四元数存储，外部欧拉角接口
- 惰性计算：矩阵缓存与脏标记优化
- 相机友好：提供专用的视图矩阵和相机控制接口

**符合OpenGL默认的坐标系规范**
```cpp
// OpenGL标准右手坐标系定义
static const glm::vec3 s_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);      // +Y 向上
static const glm::vec3 s_WorldForward = glm::vec3(0.0f, 0.0f, -1.0f); // -Z 向前
````

**变换属性管理**

| 属性 | 内部存储 | 外部接口 | 默认值 |
|------|----------|----------|--------|
| **位置** | `glm::vec3` | 直接访问 | `(0,0,0)` |
| **旋转** | 四元数 + 欧拉角 | 度制欧拉角 | 单位四元数 |
| **缩放** | `glm::vec3` | 直接访问 | `(1,1,1)` |
| **旋转顺序** | `EulerOrder` | 枚举选择 | `YXZ` |

**矩阵缓存的惰性计算机制**
```mermaid
flowchart TD
    subgraph TransformSet [设置操作 SET]
        A1[Set Position/Rotation/Scale] --> A2[更新对应属性值]
        A2 --> A3[标记脏标志<br/>m_MatrixDirty = true]
        
        B1[Set Rotation] --> B2[更新欧拉角]
        B2 --> B3[立即更新四元数<br/>UpdateRotationFromEuler]
        B3 --> A3
        
        C1[SetRotationQuat] --> C2[更新四元数]
        C2 --> C3[标记旋转脏标志<br/>m_RotationDirty = true]
        C3 --> A3
    end
    
````
```mermaid
flowchart TD
    subgraph TransformGet [获取操作 GET]
        D1[Get Matrix/Position/Scale] --> D2{检查矩阵脏标志<br/>m_MatrixDirty?}
        D2 -->|是| D3[更新局部矩阵<br/>UpdateLocalMatrix]
        D3 --> D4[清除矩阵脏标志]
        D4 --> D5[返回缓存值]
        D2 -->|否| D5
        
        E1[Get Rotation] --> E2{检查旋转脏标志<br/>m_RotationDirty?}
        E2 -->|是| E3[更新欧拉角<br/>UpdateEulerFromRotation]
        E3 --> E4[清除旋转脏标志]
        E4 --> E5[返回欧拉角]
        E2 -->|否| E5
    end
````

**双旋转表示**
```cpp
// 内部：四元数存储（计算高效，无万向节锁）
mutable glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
// 外部：欧拉角接口（用户友好）
mutable glm::vec3 m_RotationEuler = glm::vec3(0.0f);
````

**相机控制系统（防翻滚旋转）**：确保相机的Up朝向始终和WorldUp保持一致

| 向量类型 | 计算方式 | 用途 |
|----------|----------|------|
| `GetConstrainedUp()` | `s_WorldUp(0,1,0)` | 朝上方向不变 |
| `GetForward()` | 四元数旋转 `s_WorldForward(0,0,-1)` | 基础前向 |
| `GetConstrainedRight()` | `cross(forward, constrained up)` | 防翻滚右向 |
| `GetConstrainedForward()` | `cross(worldUp, constrained right)` | 约束前向 |


### Camera相机

Camera系统是MiteEngine的视图管理核心，基于GLM数学库开发，负责投影矩阵计算和相机参数管理。与Transform系统协同工作，共同完成的3D视图解算功能。

**设计理念**
- 投影与视图分离：Camera专注投影矩阵，Transform负责视图矩阵
- 双投影支持：透视与正交投影模式
- 惰性计算：投影矩阵缓存与脏标记优化

**投影矩阵的惰性计算机制**(Transform惰性计算机制的简化版)
```mermaid
flowchart TD
   subgraph CameraSet [设置操作 SET]
        A[设置相机参数] --> B[标记投影脏标志]
        B --> C[参数验证与钳制]
    end
    
````
```mermaid
flowchart TD
    subgraph CameraGet [获取操作 GET]
        D[获取投影矩阵] --> E[检查脏标志]
        E -->|干净| F[返回缓存矩阵]
        E -->|脏| G{投影类型判断}
        
        G -->|透视| H[计算透视矩阵]
        G -->|正交| I[计算正交矩阵]
        
        H --> J[短边FOV自适应]
        I --> K[基于正交尺寸计算]
        
        J --> L[清除脏标志]
        K --> L
        L --> F
    end
````
**相机控制接口**：统一的缩放控制接口Zoom()，自动适配投影类型：若为透视相机，则改变视场角；若为正交相机，则调整视口大小。

**短边FOV自适应算法**：仿照Blender设计，确保视野范围始终充足的同时，适应各种屏幕比例

当宽高比 $Aspect\geq1.0$ 时（宽屏），直接使用垂直的FOV进行计算（缓存的FOV为垂直FOV）：

$$
ProjectionMatrix = \text{perspective}\left( \text{radians}(FOV),\ Aspect,\ Near,\ Far \right)
$$

当宽高比 $Aspect<1.0$ 时（竖屏），先根据缓存的垂直FOV计算水平方向的视场角，再将其作为输入参数交给 $\text{perspective}$ 函数：

$$
horizontalFOV = 2\cdot\arctan\left(\frac{\tan\left(\frac{\text{radians}(FOV)}{2}\right)}{Aspect}\right)
$$

$$
ProjectionMatrix=\text{perspective}\left(horizontalFOV,Aspect,Near,Far\right)
$$

其中GLM提供的 $\text{perspective}$ 函数使用垂直方向的视场角 $FOV$（以弧度为单位）。

### Model模型/Mesh网格体
Mesh和Model系统是MiteEngine的几何数据管理层，负责组织和管理3D模型的网格数据和材质关联。采用分层设计，支持多子网格和多LOD级别。

**设计理念**
- 资源引用模式：Mesh引用父Model的GPU资源
- LOD链式管理：统一的基础LOD + 可选细节级别
- 材质关联：网格与材质实例的自动绑定
- 包围盒层次：模型级和网格级包围盒支持

**层次结构关系**
```mermaid
graph TD
    A[Model模型] --> B[Mesh子网格1]
    A --> C[Mesh子网格2]
    
    B --> E[LOD链管理]
    C --> F[LOD链管理]
    
    E --> H[基础LOD]
    E --> I[LOD级别123…]
    
    A --> K[材质实例数组]
    B --> L[材质索引引用]
    C --> J[材质索引引用]
````

**安全访问机制**
- 边界检查：所有索引访问都进行有效性验证
- 异常处理：无效网格构造抛出标准异常
- 无效LOD请求：返回基础级别数据

### Bounding Volume包围盒
包围盒系统是MiteEngine的空间计算核心，提供统一的几何体表示和高效的相交测试算法。该系统支持多种包围体类型，为空间查询、碰撞检测和视锥剔除提供数学基础。

**设计理念**
- 统一接口：多种包围体类型的统一抽象
- 类型转换：支持包围体间的智能转换
- 精确测试：基于分离轴定理的精确相交检测
- 性能优化：近似算法与精确算法的平衡

**包围体类型体系**
```mermaid
classDiagram
    class BoundingVolume {
        +GetType() BoundingVolumeType
        +Transform(matrix) BoundingVolume
        +Intersects(other) IntersectionType
        +GetAABBApproximation() BoundingVolumeAABB
        +ConvertTo(targetType) bool
    }
    
    class BoundingVolumeAABB {
        +min: vec3
        +max: vec3
        +GetCenter() vec3
        +GetSize() vec3
    }
    
    class BoundingVolumeSphere {
        +center: vec3
        +radius: float
    }
    
    class BoundingVolumeOBB {
        +center: vec3
        +extents: vec3
        +orientation: mat3
    }
    
    class BoundingVolumePlane {
        +normal: vec3
        +distance: float
    }
    
    BoundingVolume --> BoundingVolumeAABB
    BoundingVolume --> BoundingVolumeSphere
    BoundingVolume --> BoundingVolumeOBB
    BoundingVolume --> BoundingVolumePlane
````

**OBB的分离轴定理** (SAT) ：OBB-OBB相交测试采用完整的分离轴定理，测试15个可能的分离轴：

分离轴集合：6个面法线轴（每个OBB的3个轴向）9个边叉积轴（3×3组合）

对于每个分离轴 $\vec{a}$，计算投影半径：
$$r_A = \sum_{i=0}^{2} |\vec{a} \cdot \vec{u}A^i| \cdot e_A^i$$
$$r_B = \sum{i=0}^{2} |\vec{a} \cdot \vec{u}_B^i| \cdot e_B^i$$

其中 $\vec{u}_A^i$ 和 $\vec{u}_B^i$ 是OBB的轴向向量， $e_A^i$ 和 $e_B^i$ 是半长。

时间复杂度： $O(1)$ 常数时间，但需要测试最多15个轴

**Sphere的Welzl最小包围球算法**：采用随机化线性时间算法计算点集的最小包围球：
1. 随机打乱点集顺序
2. 递归构建支撑集（最多4个点）
3. 基于支撑集大小计算最小球：1点：零半径球; 2点：直径球; 3点：外接圆球; 4点：四面体外接球

时间复杂度： 期望 $O(n)$

**四态相交结果**：支持所有的相交检测情况描述

```cpp
enum class IntersectionType {
    Outside,    // 完全不相交
    Inside,     // 测试对象在主体内部
    Intersect,  // 部分相交
    Covered     // 测试对象完全包含主体
};
````

**测试策略矩阵**

| 主体类型 | AABB | Sphere | OBB | Plane |
|---------|------|--------|-----|-------|
| **AABB** | 轴对齐测试 | 距离平方比较 | SAT简化 | 投影范围测试 |
| **Sphere** | 对称转换 | 中心距离比较 | 局部空间测试 | 符号距离测试 |
| **OBB** | 对称转换 | 对称转换 | 完整SAT | 投影范围测试 |
| **Plane** | 对称转换 | 对称转换 | 对称转换 | 无意义 |

对称转换规则：
- AABB转换：所有类型均可通过外接AABB近似
- Sphere转换：通过外接球保持保守性
- OBB转换：保持方向信息，精度最高
- Plane转换：有限支持，主要用于特殊场景

### Frustum视锥体

视锥体系统采用平面集合表示法，通过6个裁剪平面定义可视空间，为所有几何体类型提供统一的可见性判断接口：
```cpp
BoundingVolumeIntersection::IntersectionType TestBoundingVolume(const BoundingVolume& volume);
````

**平面提取算法**

从视图投影矩阵中提取6个裁剪平面，采用行向量组合法：

对于列主序矩阵 $M$，平面系数为：
- 左平面: $\text{row}_3 + \text{row}_0$
- 右平面: $\text{row}_3 - \text{row}_0$
- 下平面: $\text{row}_3 + \text{row}_1$
- 上平面: $\text{row}_3 - \text{row}_1$
- 近平面: $\text{row}_3 + \text{row}_2$
- 远平面: $\text{row}_3 - \text{row}_2$

标准化处理确保数值稳定性：

$$\vec{n} = \frac{\vec{coefficients}_{xyz}}{|\vec{coefficients}_{xyz}|}, \quad d = \frac{coefficient_w}{|\vec{coefficients}_{xyz}|}$$

**球体测试算法**

基于距离比较的快速判断，时间复杂度 $O(1)$ ：

```cpp
for (each plane P) {
    float distance = P.DistanceToPoint(sphere.center);
    if (distance < -sphere.radius) return Outside;  // 早期退出
    if (distance < sphere.radius) intersects = true;
}
return intersects ? Intersect : Inside;
````

**AABB测试算法**

采用极值点优化，避免测试所有8个顶点：

对于平面法线 $\vec{n}$：
- 正顶点：在 $\vec{n}$ 正方向上的AABB角点
- 负顶点：在 $\vec{n}$ 负方向上的AABB角点

测试逻辑：
- 测试负顶点：若 $d_{\text{negative}} < 0$，可能完全在外
- 验证完全在外：必要时测试8个顶点确认
- 测试正顶点：若 $d_{\text{positive}} < 0$，标记为相交

```mermaid
graph LR
    A[几何体输入] --> B{类型判断}
    B -->|球体| C[快速距离测试]
    B -->|AABB| D[极值点优化]
    B -->|OBB| E[AABB近似]
    
    C --> G[早期退出]
    D --> G
    E --> G
    
    G --> H[结果输出]
````

**优化技术**
- 早期退出：发现完全在外立即返回
- 极值点计算：将AABB测试从 $O(48)$ 优化到 $O(12)$
- 缓存友好：平面数据连续存储，提高缓存命中率
- 分支预测：减少测试中的条件分支

视锥体系统通过算法设计和多层优化策略，为实时渲染提供了高效的可见性判断基础，在复杂场景中可实现显著的性能提升。

### Ray射线

射线系统是MiteEngine的空间查询工具，提供从屏幕到3D空间的射线投射和几何体相交检测功能。主要用于鼠标拾取、碰撞检测(暂未实现)等交互场景。

**射线定义** $\vec{p} = \vec{o} + t\vec{d}$ 
```cpp
struct Ray {
    glm::vec3 origin;     // 射线起点
    glm::vec3 direction;  // 射线方向（单位向量）
    float tMin;           // 有效范围最小值
    float tMax;           // 有效范围最大值
};
````

**屏幕射线生成**

```mermaid
graph LR
    A[屏幕UV坐标] --> B[NDC坐标转换]
    B --> C[视图空间变换]
    C --> D[世界空间变换]
    D --> E[射线方向计算]
    E --> F[标准化射线]
````

- 屏幕UV(0,1) → NDC(1,1): $(2u-1, 1-2v, -1)$
- 逆投影变换: $\text{viewPos} = P^{-1} \times \text{ndcPos}$
- 逆视图变换: $\text{worldPos} = V^{-1} \times \text{viewPos}$

**相交检测统一接口设计**

通过提供统一的相交测试接口，支持所有包围体类型，通过类型分发自动选择最优算法，简化使用复杂度。
```cpp
bool Ray::Intersects(const BoundingVolume& volume, float& t) const;
````

**AABB相交检测**（SLAB方法）：基于分离轴定理的高效算法
1. 对每个坐标轴计算相交区间
2. 取所有区间的交集
3. 检查交集是否在有效范围内

对于每个轴 $i$：
$$t_1 = \frac{\text{aabb.min}[i] - \text{origin}[i]}{\text{direction}[i]}$$
$$t_2 = \frac{\text{aabb.max}[i] - \text{origin}[i]}{\text{direction}[i]}$$

取 $t_{\text{min}} = \max(t_1, t_2)$ 和 $t_{\text{max}} = \min(t_1, t_2)$ 的区间交集。

**球体相交检测**：基于二次方程求解

球方程： 
$$|\vec{p} - \vec{c}|^2 = r^2$$

射线方程： 
$$\vec{p} = \vec{o} + t\vec{d}$$

代入得二次方程： 
$$at^2 + bt + c = 0$$

其中： 
$$a = \vec{d} \cdot \vec{d}$$
$$b = 2\vec{d} \cdot (\vec{o} - \vec{c})$$
$$c = (\vec{o} - \vec{c}) \cdot (\vec{o} - \vec{c}) - r^2$$

**三角形相交检测**（Möller-Trumbore算法）：高效的单次相交测试

算法核心： 

$$\begin{bmatrix}t \\ u \\ v\end{bmatrix} = \frac{1}{\vec{d} \cdot (\vec{e_1} \times \vec{e_2})}\begin{bmatrix}\vec{q} \cdot \vec{e_2} \\\vec{p} \cdot \vec{t} \\\vec{q} \cdot \vec{d}\end{bmatrix}$$

其中： 
$$\vec{e_1} = \vec{v_1} - \vec{v_0}$$
$$\vec{e_2} = \vec{v_2} - \vec{v_0}$$
$$\vec{p} = \vec{d} \times \vec{e_2}$$
$$\vec{q} = \vec{t} \times \vec{e_1}$$
$$\vec{t} = \vec{o} - \vec{v_0}$$

其中： $\vec{v_0},\vec{v_1},\vec{v_2}$ 分别为三角形第一、二、三个顶点坐标 (Vertex 0,1,2)， $u,v$ 分别为相对于边 $\vec{v_0}\rightarrow\vec{v_1}$ 和 $\vec{v_0}\rightarrow\vec{v_2}$ 的重心坐标分量。 $\vec{o}, t,\vec{d}$ 为射线 $\vec{p} = \vec{o} + t\vec{d}$ 的三个分量

若$u,v$其中任意一个值为负，则射线和三角形不会相交。否则判定为相交，返回射线

### Runtime Texture运行时纹理对象

运行时纹理系统负责动态创建和管理渲染目标，包括G-Buffer、ShadowMap等非资产纹理。通过事件驱动架构与GPU资源管理解耦，支持实时调整尺寸和类型特定的参数配置。

设计特点
- 事件驱动创建：通过RuntimeTextureCreateEvent委托GPU设备创建纹理
- 类型化配置：根据纹理用途（阴影贴图、G-Buffer、光照结果）自动设置过滤模式和环绕方式
- 动态尺寸调整：支持运行时重设纹理尺寸，适应窗口变化
- 资源生命周期：RAII模式管理，自动清理GPU资源
