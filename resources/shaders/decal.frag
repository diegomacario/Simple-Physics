in vec4 clipSpacePos;

uniform mat4 inverseModel;
uniform mat4 inverseView;
uniform mat4 inverseProjection;
uniform float width;
uniform float height;
uniform sampler2D depthTex;
uniform sampler2D decalTex;

out vec4 fragColor;

// Full explanation of this shader is available here: https://stackoverflow.com/questions/54201496/bounds-check-problem-using-a-deferred-screenspace-decal-system
void main()
{
   vec3 ndcPos = clipSpacePos.xyz / clipSpacePos.w;

   vec2 uv = gl_FragCoord.xy / vec2(width, height);

   float depth = texture(depthTex, uv).r;

   float ndcZ = depth * 2.0 - 1.0;

   ndcPos = vec3(ndcPos.xy, ndcZ);

   vec4 homogeneousViewSpacePos = inverseProjection * vec4(ndcPos, 1.0);
   vec3 viewSpacePos            = homogeneousViewSpacePos.xyz / homogeneousViewSpacePos.w;
   vec3 worldSpacePos           = (inverseView * vec4(viewSpacePos, 1.0)).xyz;
   vec3 objectSpacePos          = (inverseModel * vec4(worldSpacePos, 1.0)).xyz;

   if (abs(objectSpacePos.x) > 0.5 || abs(objectSpacePos.y) > 0.5 || abs(objectSpacePos.z) > 0.5)
   {
      //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
      discard;
   }
   else
   {
      // Negate Y to flip the texture vertically
      vec2 uv   = vec2(objectSpacePos.x, -objectSpacePos.y) + 0.5f;
      fragColor = texture(decalTex, uv);
   }
}
