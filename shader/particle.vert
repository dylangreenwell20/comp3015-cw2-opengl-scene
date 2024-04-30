#version 460

layout (location = 0) in vec3 VertexInitVel;
layout (location = 1) in float VertexBirthTime;

out float Transparency;
out vec2 TexCoord;

uniform float Time;

uniform vec3 Gravity = vec3(0.0,-0.05,0.0);
uniform float particleLifetime;
uniform float ParticleSize = 1.0;
uniform vec3 EmitterPos;

//uniform mat4 MV;
//uniform mat4 Proj;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

const vec3 offsets[] = vec3[](vec3(-0.5, -0.5, 0), vec3(0.5, -0.5, 0), vec3(0.5, 0.5, 0), vec3(-0.5, -0.5, 0), vec3(0.5, 0.5, 0), vec3(-0.5, 0.5, 0));
const vec2 texCoords[] = vec2[](vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1));

void main()
{
    vec3 cameraPos;

    float t = Time - VertexBirthTime;

    if(t >= 0 && t < particleLifetime){
        vec3 pos = EmitterPos + VertexInitVel * t + Gravity * t * t;
        cameraPos = (ModelViewMatrix * vec4(pos,1)).xyz + (offsets[gl_VertexID] * ParticleSize);
        Transparency = mix(1, 0, t / particleLifetime);
    }
    else{
        cameraPos = vec3(0);
        Transparency = 0.0;
    }

    TexCoord = texCoords[gl_VertexID];

    gl_Position = ProjectionMatrix * vec4(cameraPos,1);
}
