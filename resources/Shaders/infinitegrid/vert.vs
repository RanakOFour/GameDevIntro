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

#version 330

out vec3 WorldPos;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform float gGridSize = 10.0;
uniform vec3 u_cameraPos;
uniform vec2 u_cameraSize;

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
    vec3 vPos3 = Pos[Index];

    // Output as full screen quad in NDC space
    gl_Position = vec4(vPos3.x, vPos3.y, 0.0, 1.0);

    // Calculate world position: map NDC quad to orthographic view space
    // Hardcoded for 1920x1080
    float sizeX = 960.0;
    float sizeY = 540.0;
    
    WorldPos.x = vPos3.x * sizeX + u_cameraPos.x;
    WorldPos.y = vPos3.y * sizeY + u_cameraPos.y;
    WorldPos.z = 0.0;
}