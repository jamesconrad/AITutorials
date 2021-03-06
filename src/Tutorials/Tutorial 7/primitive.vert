/*
 *  Simple vertex shader for the pong game
 */

#version 400 core

uniform mat4 mvp;

in vec3 vPosition;

void main()
{
	gl_Position = mvp * vec4(vPosition, 1.0);
}