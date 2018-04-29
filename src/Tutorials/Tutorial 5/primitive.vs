/*
 *  Simple vertex shader for the pong game
 */

#version 400 core

uniform mat4 modelViewProjMat;

in vec3 vPosition;

void main()
{
	gl_Position = modelViewProjMat * vec4(vPosition, 1.0);
}