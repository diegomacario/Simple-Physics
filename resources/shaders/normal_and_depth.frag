in vec3 norm;

out vec4 fragColor;

void main()
{
   // Normal values range from -1.0 to 1.0, and they must range from 0.0 to 1.0 to be saved as RGB values, so we adjust them here
   vec3 adjustedNorm = (norm + vec3(1.0, 1.0, 1.0)) * 0.5;
   fragColor = vec4(adjustedNorm, 1.0);
}
