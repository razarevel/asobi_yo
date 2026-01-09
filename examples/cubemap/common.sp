#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex{
		float x,y,z;
		float nx,ny,nz;
		float u,v;
};

layout(buffer_reference, scalar) readonly buffer Vertices{
		Vertex in_Vertices[];
};

layout(buffer_reference, scalar) readonly buffer Indices{
		uint in_Indices[];
};

layout(push_constant) uniform PerFrameData{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    uint tex;
    uint texCube;
		Vertices vtx;
		Indices idx;
}pc;

struct PerVertex{
		vec2 uv;
		vec3 worldNormal;
		vec3 worldPos;
};
