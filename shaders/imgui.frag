#version 460 core

#extension GL_EXT_nonuniform_qualifier:require
layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform PushConstants {
    uint kNonLinearColorSpace;
    vec4 LRTB;
    vec2 vb;
    uint textureId;
    uint samplerId;
} pc;

void main() {
    vec4 c = in_color * texture(nonuniformEXT(sampler2D(kTextures2D[pc.textureId], kSamplers[pc.samplerId])), in_uv);
    // Render UI in linear color space to sRGB framebuffer.
    out_color = pc.kNonLinearColorSpace == 0 ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
}
