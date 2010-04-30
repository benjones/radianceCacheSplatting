#version 130
#pragma debug(on)
#pragma optimize(off)
//uniform sampler2DShadow ShadowMap;
uniform sampler2DArrayShadow ShadowMap;
uniform int numLights;
varying vec4 ShadowCoord[8];
varying vec4 diffuseComps[4];
vec3 shadowCoord[8];
float eps = .0005;

varying float realDepth;

float lookup(vec2 xy, int i)
{
  /*  if (shadowCoord[i].x < 0.0 || shadowCoord[i].x > 1.0 || 
      shadowCoord[i].y < 0.0 || shadowCoord[i].y > 1.0)
      return 0.0;*/

  float depth = texture(ShadowMap, 
			vec4(shadowCoord[i].x, shadowCoord[i].y,
			     i, shadowCoord[i].z) 
			+ vec4(xy,0.0,0.0)*eps);

  return depth  != 1.0 ? 0.0 : 1.0;
  }

void main()
{
  //varying vec4 ShadowCoord[numLights];
  //vec3 shadowCoord[numLights];
  vec2 o = mod(floor(gl_FragCoord.xy), 2.0);
  for(int i = 0; i < numLights; ++i)
    {
      shadowCoord[i] = vec3(ShadowCoord[i]/ShadowCoord[i].w);
      shadowCoord[i].z += .00005;
      //gl_FragColor = vec4(shadowCoord.z);
      

      float sum = 0.0;
      sum += lookup(vec2(-1.5, 1.5) + o, i);
      sum += lookup(vec2(0.5, 1.5) + o, i);
      sum += lookup(vec2(-1.5, -0.5) + o, i);
      sum += lookup(vec2(0.5, -0.5) + o, i);

      //gl_FragColor += vec4(.25* sum*gl_FrontColor.rgb, glFrontColor.a);
      gl_FragColor += vec4(.25* sum*diffuseComps[i].rgb, 0);
  
    }
  gl_FragColor.a = realDepth;
}
