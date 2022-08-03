uniform float width;
uniform float height;
uniform sampler2D depthTex;
uniform sampler2D decalTex;
uniform mat4 inverseModel;
uniform mat4 inverseView;
uniform mat4 inverseProjection;

out vec4 fragColor;

vec4 getWorldPosFromDepth(float depth, vec2 uv)
{
   float z = depth * 2.0 - 1.0;

   vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
   vec4 viewSpacePosition = inverseProjection * clipSpacePosition;

   // Perspective division
   viewSpacePosition /= viewSpacePosition.w;

   vec4 worldSpacePosition = inverseView * viewSpacePosition;

   return worldSpacePosition;
}

void main()
{
   vec2 uv = gl_FragCoord.xy / vec2(width, height);
   float depth = texture(depthTex, uv).r;

   vec4 worldPos = getWorldPosFromDepth(depth, uv);
   vec4 localPos = inverseModel * worldPos;

   float distX = 0.5f - abs(localPos.x);
   float distY = 0.5f - abs(localPos.y);
   float distZ = 0.5f - abs(localPos.z);

   if (distX > 0.0f && distY > 0 && distZ > 0)
   {
      vec2 uv = vec2(localPos.x, localPos.y) + 0.5f;
      fragColor = texture(decalTex, uv);
   }
   else
   {
      fragColor = vec4(1.0f, 0, 0, 1);
      //discard;
   }
}
