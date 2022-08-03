uniform sampler2D normalTex;
uniform float width;
uniform float height;

out vec4 fragColor;

void main()
{
   // Use the screen space position of the current fragment as UVs to sample the depth texture
   vec2 uv = gl_FragCoord.xy / vec2(width, height);

   // RGB values range from 0.0 to 1.0, and they must range from -1.0 to 1.0 to be used as normals, so we adjust them here
   vec3 norm = vec3(texture(normalTex, uv));
   vec3 adjustedNorm = abs((norm * 2.0) - vec3(1.0, 1.0, 1.0));
   fragColor = vec4(adjustedNorm, 1.0);
}
