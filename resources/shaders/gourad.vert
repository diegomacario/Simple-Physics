in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct PointLight
{
   vec3  worldPos;
   vec3  color;
   float linearAtt;
};

uniform PointLight light;
uniform vec3 diffuseColor;

out vec2 uv;
out vec3 color;

void main()
{
   gl_Position       = projection * view * model * vec4(position, 1.0f);
   uv                = texCoord;

   // Attenuation
   vec3  fragPos     = vec3(model * vec4(position, 1.0f));
   float distance    = length(light.worldPos - fragPos);
   float attenuation = 1.0 / (1.0 + (light.linearAtt * distance));

   // Diffuse
   vec3  lightDir    = normalize(light.worldPos - fragPos);
   // TODO: To support non-uniform scaling we will need to change the way we transform the normals
   vec3  norm        = normalize(vec3(model * vec4(normal, 0.0f)));
   vec3  diff        = max(dot(lightDir, norm), 0.0) * light.color * attenuation;
   vec3  diffuse     = (diff * diffuseColor);
   color             = diffuse;
}
