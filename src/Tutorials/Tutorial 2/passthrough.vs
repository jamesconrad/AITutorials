/*
 *  Simple vertex shader for the pong game
 */

#version 400 core

uniform mat4 modelViewProjMat;

in vec3 vPosition;
out vec2 uv;

void main()
{
	uv = vPosition.xy;
	gl_Position = modelViewProjMat * vec4(vPosition, 1.0);
}