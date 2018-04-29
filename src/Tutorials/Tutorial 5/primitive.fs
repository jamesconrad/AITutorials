/*
 *  Simple fragment shader for the pong game
 */

#version 400 core
out vec4 frag_colour;

uniform vec3 boxColor;

void main()
{
	frag_colour.rgb = boxColor;
	frag_colour.a = 1.0;
}