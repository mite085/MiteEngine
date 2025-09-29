#version 430 core
in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;
in mat3 v_TBN;

uniform vec3 u_Albedo;
uniform float u_Roughness;
uniform float u_Metallic;
uniform vec3 u_CameraPos;

// PBR贴图
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

// 环境光
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

out vec4 FragColor;

const float PI = 3.14159265359;

// 法线分布函数 (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// 几何函数 (Schlick GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 几何函数 (Smith)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel方程 (Schlick近似)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    // 从贴图获取材质属性
    vec3 albedo = pow(texture(u_AlbedoMap, v_TexCoord).rgb, vec3(2.2)) * u_Albedo;
    float metallic = texture(u_MetallicMap, v_TexCoord).r * u_Metallic;
    float roughness = texture(u_RoughnessMap, v_TexCoord).r * u_Roughness;
    float ao = texture(u_AOMap, v_TexCoord).r;
    
    // 从法线贴图获取法线
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(v_TBN * normal);
    
    vec3 N = normal;
    vec3 V = normalize(u_CameraPos - v_FragPos);
    vec3 R = reflect(-V, N);
    
    // 计算反射率在0度时的基础反射率
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // 反射方程
    vec3 Lo = vec3(0.0);
    
    // 光照计算（点光源、方向光等）
    // for(int i = 0; i < lightCount; ++i) {
    //     vec3 L = normalize(lightPositions[i] - v_FragPos);
    //     vec3 H = normalize(V + L);
    //     float distance = length(lightPositions[i] - v_FragPos);
    //     float attenuation = 1.0 / (distance * distance);
    //     vec3 radiance = lightColors[i] * attenuation;
    //     
    //     // Cook-Torrance BRDF
    //     float NDF = DistributionGGX(N, H, roughness);
    //     float G = GeometrySmith(N, V, L, roughness);
    //     vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    //     
    //     vec3 nominator = NDF * G * F;
    //     float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    //     vec3 specular = nominator / denominator;
    //     
    //     vec3 kS = F;
    //     vec3 kD = vec3(1.0) - kS;
    //     kD *= 1.0 - metallic;
    //     
    //     float NdotL = max(dot(N, L), 0.0);
    //     Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    // }
    
    // 环境光照 (IBL)
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;
    
    // HDR色调映射
    color = color / (color + vec3(1.0));
    // gamma校正
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}