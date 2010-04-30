varying vec3 coord;
varying vec3 normal;
varying vec4 diffuse;

void main()
{
  coord = vec3(gl_ModelViewMatrix * gl_Vertex);
  normal = normalize(gl_NormalMatrix *gl_Normal);
  gl_Position = ftransform();
  diffuse = gl_FrontMaterial.diffuse;
}
