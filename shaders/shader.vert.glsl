#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 LightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float _Time;

uniform vec3 offsets[100];

in vec3 lightPos;

void main()
{
    float _EffectRadius = 0.5;
    float _WaveSpeed = 10.0;
    float _WaveHeight = 0.07;
    float _WaveDensity = 2.0;
    float _Yoffset = 2.0;
    int _Threshold = 3;
    float _StrideSpeed = 5.0;
    float _StrideStrength = 0.15;
    float _MoveOffset = 0.0;

    float sinUse = sin(_Time * _WaveSpeed + _MoveOffset + aPos.z * _WaveDensity);
    float yValue = -aPos.z + _Yoffset;
    float yDirScaling = clamp(pow(yValue * _EffectRadius,_Threshold),0.0,1.0);

    vec3 Pos = aPos;
    Pos.x = aPos.x + sinUse * _WaveHeight * yDirScaling;
    Pos.x = Pos.x + sin(-_Time * _StrideSpeed + _MoveOffset) * _StrideStrength;

    vec3 offset = offsets[gl_InstanceID];
    gl_Position = projection * view * model * vec4(Pos + offset, 1.0);

    FragPos = vec3(view * model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(view * model))) * aNormal;

    LightPos = vec3(view * vec4(lightPos, 1.0));
    TexCoords = aTexCoords;
}
