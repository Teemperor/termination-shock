#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
flat in float light;
in float OS;

// Ouput data
out vec4 color;


// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

	// Output color = color of the texture at the specified UV
	color.rgb = texture(myTextureSampler, vec2(UV.x, -UV.y)).rgb * OS * light;
	color.a = 1;
}