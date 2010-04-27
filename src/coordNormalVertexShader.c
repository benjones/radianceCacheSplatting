varying vec3 eyeCoord;
varying vec3 normal;

void main()
{
  eyeCoord = vec3(gl_ModelViewMatrix * gl_Vertex);
  normal = normalize(gl_NormalMatrix *gl_Normal);
  gl_Position = ftransform();

}
