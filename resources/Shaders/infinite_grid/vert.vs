/*

        Copyright 2024 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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