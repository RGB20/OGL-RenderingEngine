#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 uv[];
in float Height[];

out vec3 fNormal;
out vec2 fuv;
out float fHeight;
out vec3 fFragpos;

uniform mat4 view;
uniform mat4 projection;

void GenerateLine(int index, vec3 calculatedNormal)
{
    gl_Position = projection * view * gl_in[index].gl_Position;
   
    fNormal = calculatedNormal;
    fuv = uv[index];
    fHeight = Height[index];
    fFragpos = vec3(gl_in[index].gl_Position);
    
    EmitVertex();
}

void main()
{
    vec3 edge1 = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);    
    vec3 edge2 = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);    
    vec3 calculatedNormal = normalize(cross(edge1, edge2));

    GenerateLine(0, calculatedNormal); // first vertex normal
    GenerateLine(1, calculatedNormal); // second vertex normal
    GenerateLine(2, calculatedNormal); // third vertex normal

    EndPrimitive();
}  
