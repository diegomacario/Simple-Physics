in vec3 norm;
in vec2 uv;

uniform sampler2D diffuseTex;

out vec4 fragColor;

void main()
{
   fragColor = vec4(vec3(texture(diffuseTex, uv)), 1.0);
}
