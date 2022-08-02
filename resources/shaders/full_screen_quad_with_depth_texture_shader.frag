uniform sampler2D depthTex;
uniform float width;
uniform float height;

out vec4 fragColor;

float linearizeDepth(float depth)
{
   float near = 0.1;
   float far = 130.0;
   return ((2.0 * near * far) / (far + near - (2.0 * depth - 1.0) * (far - near)));
}

void main()
{
   // Use the screen space position of the current fragment as UVs to sample the depth texture
   vec2 uv = gl_FragCoord.xy / vec2(width, height);

   // The depth we read from the texture ranges from 0.0 to 1.0
   float depth = texture(depthTex, uv).r;

   // The linearized depth is unbounded, since it's the depth in world space
   // We scale it by 0.075 to make it small because we want to use it as a color, and colors range from 0.0 to 1.0
   depth = linearizeDepth(depth) * 0.075;

   fragColor = vec4(vec3(depth), 1.0);
}