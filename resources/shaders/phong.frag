in vec3 fragPos;
in vec3 norm;

struct PointLight
{
   vec3  worldPos;
   vec3  color;
   float linearAtt;
};

uniform PointLight light;
uniform vec3 cameraPos;
uniform vec3 diffuseColor;

out vec4 fragColor;

void main()
{
   vec3 viewDir = normalize(cameraPos - fragPos);

   // Attenuation
   float distance    = length(light.worldPos - fragPos);
   float attenuation = 1.0 / (1.0 + (light.linearAtt * distance));

   // Diffuse
   vec3  lightDir    = normalize(light.worldPos - fragPos);
   vec3  diff        = max(dot(lightDir, norm), 0.0) * light.color * attenuation;
   vec3  diffuse     = (diff * diffuseColor);

   fragColor = vec4(diffuse, 1.0);
}
