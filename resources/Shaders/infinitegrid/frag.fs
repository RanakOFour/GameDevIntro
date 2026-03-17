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

#version 420

in vec3 WorldPos;

layout(location = 0) out vec4 FragColor;

uniform vec3 u_cameraPos;
uniform float gGridSize = 100.0;
uniform float gGridMinPixelsBetweenCells = 2.0;
uniform float gGridCellSize = 0.025;
uniform vec4 gGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

float log10(float x)
{
    float f = log(x) / log(10.0);
    return f;
}


float satf(float x)
{
    float f = clamp(x, 0.0, 1.0);
    return f;
}


vec2 satv(vec2 x)
{
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}


float max2(vec2 v)
{
    float f = max(v.x, v.y);
    return f;
}


void main()
{
    // Simple grid: 100 unit cells with 5 unit line width
    vec2 coord = mod(WorldPos.xy, 100.0);
    vec2 dist = min(coord, 100.0 - coord);
    float line = step(5.0, min(dist.x, dist.y));
    
    vec4 Color = mix(vec4(0.2, 0.2, 0.2, 1.0), vec4(1.0), 1.0 - line);
    
    FragColor = Color;
}
