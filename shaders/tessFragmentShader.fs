#version 410 core

//in float Height;
in vec2 uv;
//in vec3 normal;

uniform sampler2D heightMap;


out vec4 FragColor;

void main()
{
    //FragColor = vec4(Height, Height, Height, 1.0);
    //FragColor = vec4(uv, 0.0f, 1.0f);
    float h = texture(heightMap, uv).r;

    //FragColor = vec4(h, 0.0f, 0.0f, 1.0f);
    FragColor = vec4(vec3(1.0f), 1.0f);

    //if (h > 0.0 && h < 0.1f)
    //{
    //    FragColor = vec4(0.0f, 0.6f, 0.3f, 1.0f);
    //}
    //else if (h >= 0.1f && h < 0.2f)
    //{
    //    FragColor = vec4(0.0f, 0.9f, 0.1f, 1.0f);
    //}
    //else if (h >= 0.2f && h < 0.5f)
    //{
    //    FragColor = vec4(0.4f, 0.1f, 0.0f, 1.0f);
    //}
    //else if (h >= 0.5f && h < 0.8f)
    //{
    //    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //}
}