layout(buffer_reference, std430) readonly buffer CameraData {
    mat4 invView;
    mat4 invProj;
    vec4 position;
};

layout(buffer_reference, std430) readonly buffer VoxelGridData {
    uvec4 size;
    uint data[];
};

layout(buffer_reference, std430) readonly buffer PaletteData {
    // data[7:0] -> red
    // data[15:8] -> green
    // data[23:16] -> blue
    // data[31:24] -> alpha
    uint data[];
};


layout(buffer_reference, std430) readonly buffer MaterialData {
    Material data[];
};