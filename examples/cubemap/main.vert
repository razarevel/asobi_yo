#version 460 core
#include <common.sp>

layout(location = 0) out PerVertex vtx;

void main() {
    uint idx = pc.idx.in_Indices[uint(gl_VertexIndex)];
    Vertex v = pc.vtx.in_Vertices[idx];
    vec3 pos = vec3(v.x, v.y, v.z);
    vec3 normal = vec3(v.nx, v.ny, v.nz);
    gl_Position = pc.proj * pc.view * pc.model * vec4(pos, 1.0f);

    mat4 model = pc.model;
    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    vtx.uv = vec2(v.u, v.v);
    vtx.worldNormal = normalMatrix * normal;
    vtx.worldPos = (model * vec4(pos, 1.0f)).xyz;
}
