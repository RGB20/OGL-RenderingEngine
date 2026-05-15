#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 worldDir;

uniform mat4 projection;
uniform mat4 viewNoTranslate;
uniform mat4 model;

void main()
{  
    // worldDir is the direction from the camera to the vertex.
    // Since viewNoTranslate centers us, the model-transformed position is the direction.
    worldDir = vec3(model * vec4(aPos, 1.0));

    // Transform vertex to clip space. We use aPos directly (no manual cameraPos addition).
    vec4 pos = projection * viewNoTranslate * model * vec4(aPos, 1.0);
    
    // The "xyww" trick: force the depth (z/w) to be 1.0 after perspective division.
    // This places the skybox at the absolute far plane.
    gl_Position = pos.xyww;
}