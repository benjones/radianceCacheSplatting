#version 130
#pragma debug(on)
#pragma optimize(off)
//uniform sampler2DShadow ShadowMap;
uniform sampler2DShadow ShadowMap;
varying vec4 ShadowCoord;
vec3 shadowCoord;
float eps = .0005;

float lookup(float x, float y)
{
  float depth = texture(ShadowMap, shadowCoord + vec3(x,y,0.0)*eps);
  return depth  != 1.0 ? 0.5 : 1.0;
  }

void main()
{

  //vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w;
  /*  ShadowCoord.z += .0005;

  float distToLight = texture2D(ShadowMap, ShadowCoord.st).z;

  float shadow = 1.0;
  if(ShadowCoord.w > 0.0)
    {
      shadow = distToLight < ShadowCoord.z ? .5 : 1.0;
    }
  */
  /*
  float shadeFactor = lookup(0.0, 0.0);
 gl_FragColor = shadeFactor*gl_Color;
  */
  shadowCoord = vec3(ShadowCoord/ShadowCoord.w);
  shadowCoord.z += .005;
  //gl_FragColor = vec4(shadowCoord.z);
  gl_FragColor = vec4(lookup(0.0,0.0)*gl_Color.rgb, gl_Color.a);
  
  //gl_FragColor = vec4(gl_Color.rgb*lookup(0.0,0.0), gl_Color.a);//*gl_Color;
  //gl_FragColor = texture2D(ShadowMap, ShadowCoord.st);
  //gl_FragColor = ShadowCoord.z;
}
