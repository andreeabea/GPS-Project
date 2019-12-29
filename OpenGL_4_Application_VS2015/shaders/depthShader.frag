#version 410 core
out vec4 fColor;

float computeShadow()
{
}
void main()
{
	shadow = computeShadow();
	fColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}