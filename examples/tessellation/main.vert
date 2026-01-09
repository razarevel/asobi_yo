#version 460 core
#include <common.sp>

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 worldPos_in;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    uint idx = pc.idx.in_Indices[uint(gl_VertexIndex)];
    Vertex v = pc.vtx.in_Vertices[idx];
    vec3 pos = vec3(v.x, v.y, v.z);
    gl_Position = pc.proj * pc.view * pc.model * vec4(pos, 1.0f);

    uv = vec2(v.u, v.v);
    worldPos_in = (pc.model * vec4(pos, 1.0f)).xyz;
}
