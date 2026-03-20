#version 430 core

out vec3 WorldPos;

uniform mat4 u_View;
uniform mat4 u_Projection;

uniform float gGridSize = 100.0;
uniform vec3 u_cameraPos;

const vec3 Pos[4] = vec3[4](
    vec3(-1.0, -1.0, 0.0),      // bottom left
    vec3( 1.0, -1.0, 0.0),      // bottom right
    vec3( 1.0, 1.0,  0.0),      // top right
    vec3(-1.0, 1.0,  0.0)       // top left
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);


void main()
{
    int Index = Indices[gl_VertexID];
    vec3 vPos3 = Pos[Index] * gGridSize;

    vPos3.x += u_cameraPos.x;
    vPos3.y += u_cameraPos.y;

    vec4 vPos4 = vec4(vPos3, 1.0);

    gl_Position = u_Projection * u_View * vPos4;

    WorldPos = vPos3;
}