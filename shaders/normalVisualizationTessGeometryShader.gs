#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 normal[];
in vec2 uv[];

out vec3 fNormal;
out vec2 fuv;

const float MAGNITUDE = 1.2;
  
uniform mat4 view;
uniform mat4 projection;

void GenerateLine(int index, vec3 calculatedNormal)
{
    gl_Position = projection * view * gl_in[index].gl_Position;
    EmitVertex();
    //gl_Position = projection * view * (gl_in[index].gl_Position + vec4(normal[index], 0.0) * MAGNITUDE);
    gl_Position = projection * view * (gl_in[index].gl_Position + vec4(calculatedNormal, 0.0) * MAGNITUDE);
    EmitVertex();
    
    //fNormal = vec3(normal[index]);
    fNormal = calculatedNormal;
    fuv = uv[index];

    EndPrimitive();
}

void main()
{
    vec3 edge1 = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);    
    vec3 edge2 = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);    
    vec3 calculatedNormal = normalize(cross(edge1, edge2));

    GenerateLine(0, calculatedNormal); // first vertex normal
    GenerateLine(1, calculatedNormal); // second vertex normal
    GenerateLine(2, calculatedNormal); // third vertex normal
}  
