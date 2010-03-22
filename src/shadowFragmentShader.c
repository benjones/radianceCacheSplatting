uniform sampler2D ShadowMap;

varying vec4 ShadowCoord;


void main()
{

  vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w;
  shadowCoordinateWdivide.z += .0005;

  float distToLight = texture2D(ShadowMap, shadowCoordinateWdivide.st).z;

  float shadow = 1.0;
  if(ShadowCoord.w > 0.0)
    shadow = distToLight < shadowCoordinateWdivide.z ? .5 : 1.0;

  gl_FragColor = shadow*gl_Color;


}
