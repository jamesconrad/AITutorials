#version 400 core

out vec4 outCol;
uniform sampler2D mainTexture;

in vec2 uv;

void main()
{
	outCol.rgb = texture2D(mainTexture, uv).rgb;
	outCol.a = 1.0;
}