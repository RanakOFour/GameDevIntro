#version 430

in vec2 v_texCoord;
uniform sampler2D u_Texture;

out vec4 o_fragColor;

void main()
{
	vec4 tex = texture(u_Texture, v_texCoord);
	o_fragColor = tex;
}