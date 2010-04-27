varying vec3 coord;
varying vec3 normal;

void main()
{
  coord = vec3(gl_ModelViewMatrix * gl_Vertex);
  normal = normalize(gl_NormalMatrix *gl_Normal);
  gl_Position = ftransform();

}
