#version 430 core

in vec3 WorldPos;

layout(location = 0) out vec4 FragColour;

uniform vec3 u_cameraPos;
uniform float gGridSize = 1000.0;
uniform float gGridMinPixelsBetweenCells = 5.0;
uniform float gGridCellSize = 1.0;
uniform vec4 gGridColourThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gGridColourThick = vec4(0.0, 0.0, 0.0, 1.0);


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
    vec2 dvx = vec2(dFdx(WorldPos.x), dFdy(WorldPos.x));
    vec2 dvy = vec2(dFdx(WorldPos.y), dFdy(WorldPos.y));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    float l = length(dudv);

    float LOD = max(0.0, log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

    float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

    dudv *= 4.0;

    vec2 mod_div_dudv = mod(WorldPos.xy, GridCellSizeLod0) / dudv;
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    mod_div_dudv = mod(WorldPos.xy, GridCellSizeLod1) / dudv;
    float Lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );
    
    mod_div_dudv = mod(WorldPos.xy, GridCellSizeLod2) / dudv;
    float Lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    float LOD_fade = fract(LOD);
    vec4 Colour;

    if (Lod2a > 0.0) {
        Colour = gGridColourThick;
        Colour.a *= Lod2a;
    } else {
        if (Lod1a > 0.0) {
            Colour = mix(gGridColourThick, gGridColourThin, LOD_fade);
	        Colour.a *= Lod1a;
        } else {
            Colour = gGridColourThin;
	        Colour.a *= (Lod0a * (1.0 - LOD_fade));
        }
    }
    
    float OpacityFalloff = (1.0 - satf(length(WorldPos.xy - u_cameraPos.xy) / gGridSize));

    Colour.a *= OpacityFalloff;

    FragColour = Colour;
}
