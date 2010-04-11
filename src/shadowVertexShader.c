varying vec4 ShadowCoord[4];
varying vec4 diffuseComps[4];

uniform int texUnitBase;
uniform int numLights;

void main()
{
  vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
  vec3 ecPosition3 = vec3(ecPosition)/ecPosition.w;
  vec3 normal = normalize(gl_NormalMatrix *gl_Normal);
  
  float diffuse;
  //vec4 lightDiffuse;
  //vec4 matDiffuse;

  vec3 VP;
  for(int i = 0; i < numLights; ++i)
    {
      ShadowCoord[i] = gl_TextureMatrix[texUnitBase + i] * gl_Vertex;
      VP = vec3(gl_LightSource[i].position) - ecPosition3;
      VP = normalize(VP);

      diffuse = max(0.0, dot(normal, VP));
      //lightDiffuse = gl_LightSouce[i].diffuse;
      //matDiffuse = gl_FrontMaterial.diffuse;
      //diffuseComps[i] = gl_FrontMaterial.diffuse;
      diffuseComps[i] = diffuse*gl_LightSource[i].diffuse * 
      	gl_FrontMaterial.diffuse;

 
    }  



//ShadowCoord = TexCoord/TexCoord.w;
 //TODO: use actual coordinate
  gl_Position = ftransform();

  //TODO DO REAL LIGHTING
  gl_FrontColor = gl_FrontMaterial.diffuse;

}
