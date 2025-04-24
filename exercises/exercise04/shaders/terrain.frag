#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture0;

uniform vec2 ColorTextureScale;

void main()
{
	vec4 color0 = texture(ColorTexture0, TexCoord * ColorTextureScale);

	FragColor = Color * color0;
}
