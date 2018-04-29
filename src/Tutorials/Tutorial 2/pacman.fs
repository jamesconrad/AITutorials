/*
 *  Simple fragment shader for the pong game
 */

#version 400 core
out vec4 frag_colour;
in vec2 uv;

uniform vec3 boxColor;

void main()
{
	if (length(uv.rg) > 1.0f) discard;
	
	frag_colour.rgb = boxColor;
	frag_colour.a = 1.0;
}