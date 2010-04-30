
uniform sampler2D worldPoss;
uniform sampler2D worldNorms;//for all points in the scene
uniform sampler2D diffuseMaterial;//for all points in the scene

uniform float hmd;
uniform float a;

uniform vec2 windSize;

varying vec3 worldCoord;
varying vec3 worldNormal;//for this sample



void main()
{
  vec2 texCoord = vec2(gl_FragCoord.x/windSize.x, gl_FragCoord.y/windSize.y);
  vec3 thisPos = vec3(texture2D(worldPoss, texCoord));
  vec3 thisNorm = vec3(texture2D(worldNorms, texCoord));
  vec3 matCol = vec3(texture2D(diffuseMaterial, texCoord));

  vec3 outColor = matCol *gl_Color.rgb;

  if(distance(thisPos, worldCoord) >= a*hmd)
    discard;
  
  float distFrac = distance(worldCoord, thisPos)/hmd;
  
  float mydot = dot(worldNormal, thisNorm);
  mydot = min(mydot, 1.0);

  float sqrtnrm = sqrt(1.0 - mydot);
  float w = 1.0/(distFrac + sqrtnrm);
  if(isnan(w))
    {
      gl_FragColor = vec4(0.0, 0.0, 1.0, 0.0);
      return;
    }  
//gl_FragColor = vec4(w/100.0);
  //return;

  if(w < 1.0/a)
    discard;//gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
  gl_FragColor = vec4(w*outColor, w);

  //  gl_FragColor = vec4(gl_FragCoord.x/1000.0, 
  //	      gl_FragCoord.y/1000.0, 0.0, 1.0);
  // gl_FragColor = gl_Color;
}
