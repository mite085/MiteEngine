#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out mat3 v_TBN;

void main() {
    v_TexCoord = aTexCoord;
    v_FragPos = vec3(u_Model * vec4(aPos, 1.0));
    
    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    v_Normal = normalMatrix * aNormal;
    
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);
    
    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}