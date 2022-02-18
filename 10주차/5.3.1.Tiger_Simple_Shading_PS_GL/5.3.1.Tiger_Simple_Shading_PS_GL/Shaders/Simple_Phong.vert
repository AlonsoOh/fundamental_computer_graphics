#version 330

// Shader from OpenGL 4 Shading Language Cookbook - Third Edition by D. Wolff
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Position;
out vec3 Normal;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void main() {
    Normal = normalize(NormalMatrix * VertexNormal);
    Position = ( ModelViewMatrix * vec4(VertexPosition,1.0) ).xyz;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}

