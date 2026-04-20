#version 430

in vec2 v_texCoord;

uniform sampler2D u_Texture;
uniform vec4      u_Color;     // tint / solid color
uniform int       u_UseTexture; // 0 = solid color, 1 = texture * tint

out vec4 o_fragColor;

void main()
{
    if (u_UseTexture == 1)
    {
        vec4 tex = texture(u_Texture, v_texCoord);
        o_fragColor = tex * u_Color;
    }
    else
    {
        o_fragColor = u_Color;
    }
}
