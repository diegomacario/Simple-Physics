in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 model;

out vec3 norm;
out vec2 uv;

void main()
{
   gl_Position = model * vec4(position, 1.0f);
   // TODO: The normals and the UVs aren't used by the fragment shader, so it would be good if we found a way to not pass them
   // TODO: To support non-uniform scaling we will need to change the way we transform the normals
   norm        = normalize(vec3(model * vec4(normal, 0.0f)));
   uv          = texCoord;
}
