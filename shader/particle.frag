#version 460

in float Transparency;
in vec2 TexCoord;

uniform sampler2D ParticleTex;

layout(location=4) out vec4 FragColor;

void main(){
	FragColor = texture(ParticleTex, TexCoord);
	FragColor.a *= Transparency;
}