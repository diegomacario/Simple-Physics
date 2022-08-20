in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 norm;
out vec2 uv;

void main()
{
   gl_Position = projection * view * model * vec4(position, 1.0f);

   fragPos     = vec3(model * vec4(position, 1.0f));
   // TODO: To support non-uniform scaling we will need to change the way we transform the normals
   norm        = normalize(vec3(model * vec4(normal, 0.0f)));
   uv          = texCoord;
}
