varying vec4 ShadowCoord;

void main()
{

  ShadowCoord = gl_TextureMatrix[7] * gl_Vertex;
  //TODO: use actual coordinate
  gl_Position = ftransform();

  //TODO DO REAL LIGHTING
  gl_FrontColor = gl_FrontMaterial.diffuse;

}
