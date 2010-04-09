#version 130
#pragma debug(on)
#pragma optimize(off)
//uniform sampler2DShadow ShadowMap;
uniform sampler2DArrayShadow ShadowMap;
uniform int numLights;
varying vec4 ShadowCoord[8];

vec3 shadowCoord[8];
float eps = .0005;

float lookup(float x, float y, int i)
{
  float depth = texture(ShadowMap, 
			vec4(shadowCoord[i].x, shadowCoord[i].y,
			     i, shadowCoord[i].z) 
			+ vec4(x,y,0,0.0)*eps);

  return depth  != 1.0 ? 0.05 : .5;
  }

void main()
{
  //varying vec4 ShadowCoord[numLights];
  //vec3 shadowCoord[numLights];
  for(int i = 0; i < numLights; ++i)
    {
      shadowCoord[i] = vec3(ShadowCoord[i]/ShadowCoord[i].w);
      shadowCoord[i].z += .005;
      //gl_FragColor = vec4(shadowCoord.z);
      gl_FragColor += vec4(lookup(0.0,0.0, i)*gl_Color.rgb, gl_Color.a);
  
    }
}
