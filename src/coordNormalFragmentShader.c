varying vec3 coord;
varying vec3 normal;
varying vec4 diffuse;
void main()
{


  gl_FragData[0] = vec4(coord, 1.0);
  gl_FragData[1] = vec4(normal, 1.0);
  gl_FragData[2] = diffuse;
}
