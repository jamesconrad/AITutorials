#version 400 core

out vec4 outCol;
uniform sampler2D mainTexture;

in vec2 uv;

void main()
{
	vec4 texCol = texture2D(mainTexture, uv);

	if (texCol.a < 0.5f) discard;

	outCol.rgb = texCol.rgb;
	outCol.a = 1.0;
}