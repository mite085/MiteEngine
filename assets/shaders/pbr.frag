#version 430 core
in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;
in mat3 v_TBN;

layout(std140, binding = 2) uniform MaterialUBO {  // 绑定点由BindingPointManager分配
    // 基础PBR参数
    vec4 u_BaseColor;
    vec4 u_MetallicRoughnessAO;  // x: metallic, y: roughness, z: AO
    vec4 u_Emission;             // rgb: color, a: intensity
    vec4 u_NormalScale;          // x: normal scale
    
    // 纹理标识和参数
    vec4 u_TextureFlags;         // x: hasBaseColorTex, y: hasNormalTex, z: hasMRTex, w: hasEmissiveTex
    vec4 u_BaseColorTexParams;   // xy: scale, zw: offset
    vec4 u_NormalTexParams;      // xy: scale, zw: offset
    vec4 u_MRTexParams;          // xy: scale, zw: offset
    vec4 u_EmissiveTexParams;    // xy: scale, zw: offset
    vec4 u_OcclusionTexParams;   // xy: scale, zw: offset
    
    // 渲染属性
    vec4 u_RenderProperties;     // x: alphaCutoff, y: doubleSided, z: alphaMode
} Material;

// PBR贴图
uniform sampler2D u_BaseColorTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetallicRoughnessTexture;
uniform sampler2D u_EmissiveTexture;
uniform sampler2D u_OcclusionTexture;

// 环境光
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

out vec4 FragColor;

const float PI = 3.14159265359;

void main() {

    FragColor = vec4(Material.u_BaseColor);
}