varying vec4 ShadowCoord[8];
uniform int texUnitBase;
uniform int numLights;

void main()
{
  //vec4 ShadowCoord[numLights];
  for(int i = 0; i < numLights; ++i)
    {
      ShadowCoord[i] = gl_TextureMatrix[texUnitBase + i] * gl_Vertex;
    }  

//ShadowCoord = TexCoord/TexCoord.w;
 //TODO: use actual coordinate
  gl_Position = ftransform();

  //TODO DO REAL LIGHTING
  gl_FrontColor = gl_FrontMaterial.diffuse;

}
