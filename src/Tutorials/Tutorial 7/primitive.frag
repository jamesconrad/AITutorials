/*
 *  Simple fragment shader for the pong game
 */

#version 400 core
out vec4 frag_color;

uniform vec3 color;

void main()
{
	frag_color.rgb = color;
	frag_color.a = 1.0f;
}