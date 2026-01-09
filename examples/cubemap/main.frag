#version 460 core
#include <common.sp>

#extension GL_EXT_nonuniform_qualifier:require
layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];
layout(set = 0, binding = 2) uniform textureCube kCubemaps[];

layout(location = 0) in PerVertex vtx;
layout(location = 0) out vec4 out_FragColor;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(
            sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

vec4 textureBindlessCube(uint textureid, uint samplerid, vec3 uv) {
    return texture(nonuniformEXT(
            samplerCube(kCubemaps[textureid], kSamplers[samplerid])), uv);
}

void main() {
    vec3 n = normalize(vtx.worldNormal);
    vec3 v = normalize(pc.cameraPos.xyz - vtx.worldPos);
    vec3 reflection = -normalize(reflect(v, n));

    vec4 colorRefl = textureBindlessCube(pc.texCube, 0, reflection);
    vec4 Ka = colorRefl * 0.3;

    float NdotL = clamp(dot(n, normalize(vec3(0, 0, -1))), 0.1, 1.0);
    vec4 Kd = textureBindless2D(pc.tex, 0, vtx.uv) * NdotL;

    out_FragColor = Ka + Kd;
};
